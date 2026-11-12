// Auto Swingshot -- experimental quality-of-life mod for OpenRAC RAC1.
// Standalone implementation. No OpenRAC source files are modified.
#include "detection.hpp"
#include "openrac/mod.hpp"
#include "openrac/games/rac1.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>

namespace {
namespace as = auto_swingshot;

constexpr std::uint32_t kCircle = 0x20;
constexpr std::uint32_t kPalPad = 0x0013CA40;
constexpr std::uint32_t kNtscPad = 0x0013C940;
// Retail's item hand and request globals, version-specific. The request word is
// consumed by the game's own item tick (as for a Quick Select selection).
constexpr std::uint32_t kPalRequest = 0x00141508;
constexpr std::uint32_t kNtscRequest = 0x00141408;
constexpr std::uint32_t kPalItemState = 0x00140504;
constexpr std::uint32_t kNtscItemState = 0x00140404;
constexpr std::uint32_t kPalOwned = 0x0013D5C8;
constexpr std::uint32_t kNtscOwned = 0x0013D4C0;
constexpr std::uint32_t kPalLast = 0x00141760;
constexpr std::uint32_t kNtscLast = 0x00141660;
// PadState. Four gameplay-facing button words: held, pressed, raw-held, raw-pressed.
constexpr std::array<std::uint32_t, 4> kPadOffsets{0x1A0, 0x1A4, 0x1B0, 0x1B4};
constexpr int kReady = 2;
constexpr int kWaitLimit = 100;
constexpr int kRestFrames = 15;

enum class Phase { idle, waiting, active };

enum class PadAction { unchanged, suppress_circle, pulse_circle };

class ScopedPadCircle {
public:
    ScopedPadCircle(openrac::Mod& mod, std::uint32_t pad, PadAction action)
      : mod_(mod), pad_(pad), action_(action) {
        if (action_ == PadAction::unchanged) return;
        auto m = mod_.memory();
        for (std::size_t i = 0; i < kPadOffsets.size(); ++i) {
            const auto addr = pad_ + kPadOffsets[i];
            const auto value = m.read<std::uint32_t>(addr);
            old_circle_[i] = value & kCircle;
            const auto changed = action_ == PadAction::suppress_circle
                ? (value & ~kCircle) : (value | kCircle);
            m.write<std::uint32_t>(addr, changed);
        }
    }

    ~ScopedPadCircle() {
        if (action_ == PadAction::unchanged) return;
        auto m = mod_.memory();
        for (std::size_t i = 0; i < kPadOffsets.size(); ++i) {
            const auto addr = pad_ + kPadOffsets[i];
            const auto now = m.read<std::uint32_t>(addr);
            m.write<std::uint32_t>(addr, (now & ~kCircle) | old_circle_[i]);
        }
    }
    ScopedPadCircle(const ScopedPadCircle&) = delete;
    ScopedPadCircle& operator=(const ScopedPadCircle&) = delete;

private:
    openrac::Mod& mod_;
    std::uint32_t pad_;
    PadAction action_;
    std::array<std::uint32_t, 4> old_circle_{};
};

struct Runtime {
    explicit Runtime(openrac::Mod& m)
      : mod(m), ntsc(m.version() == "ntsc"),
        experimental(m.setting<std::string>("activation_mode") == "auto"),
        max_distance(static_cast<float>(m.setting<int>("max_distance"))),
        vertical_limit(static_cast<float>(m.setting<int>("vertical_limit"))),
        facing_degrees(static_cast<float>(m.setting<int>("facing_degrees"))) {}

    openrac::Mod& mod;
    bool ntsc = false;
    bool experimental = false;
    float max_distance = 16;
    float vertical_limit = 10;
    float facing_degrees = 30;

    float min_facing_dot() const { return as::facing_threshold(facing_degrees); }

    std::uint32_t seen_loads = std::numeric_limits<std::uint32_t>::max();
    Phase phase = Phase::idle;
    int previous_item = 0;
    int wait = 0;
    int rest = 0;
    bool circle_was_down = false;

    std::uint32_t addr(std::uint32_t pal, std::uint32_t us) const {
        return mod.relocate(ntsc ? us : pal);
    }
    std::uint32_t pad() const { return addr(kPalPad, kNtscPad); }
    std::uint32_t request() const { return addr(kPalRequest, kNtscRequest); }
    std::uint32_t item_state() const { return addr(kPalItemState, kNtscItemState); }
    std::uint32_t owned() const { return addr(kPalOwned, kNtscOwned); }
    std::uint32_t last() const { return addr(kPalLast, kNtscLast); }

    void reset() {
        phase = Phase::idle;
        previous_item = 0;
        wait = rest = 0;
        circle_was_down = false;
    }

    bool has_swingshot() const {
        return mod.memory().read<std::uint8_t>(owned() + as::kSwingshot) != 0;
    }

    // Return both the nearest target for diagnostics and the nearest target
    // inside the configured distance AND facing cone. A closer target behind
    // Ratchet must not hide a valid target in front of him.
    struct Targets {
        as::Candidate closest;
        as::Candidate eligible;
    };

    Targets scan_targets(const openrac_pose& hero) const {
        Targets result;
        const as::Point player{hero.position[0], hero.position[1], hero.position[2]};
        for (const auto& moby : mod.moby_list()) {
            if (!as::target_class(moby.oclass) || moby.state >= 0x80u) continue;
            const as::Point target{moby.position[0], moby.position[1], moby.position[2]};
            const auto candidate = as::evaluate(
                player, target, moby.slot, moby.oclass, hero.rotation[2]);
            if (std::isfinite(candidate.straight) &&
                candidate.straight < result.closest.straight) {
                result.closest = candidate;
            }
            if (as::usable(candidate, max_distance, vertical_limit, min_facing_dot()) &&
                candidate.straight < result.eligible.straight) {
                result.eligible = candidate;
            }
        }
        return result;
    }

    void circle_pressed(const openrac_pose& hero, const as::Candidate& c,
                        bool match, std::uint32_t existing_request) {
        std::string msg = "Circle: level=" + std::to_string(hero.level) +
            " held=" + std::to_string(hero.item) + " hero_state=" + std::to_string(hero.state) +
            " target_slot=" + std::to_string(c.slot) +
            " class=" + std::to_string(c.oclass);
        if (c.slot >= 0) {
            msg += " distance=" + std::to_string(c.straight) +
                " height_diff=" + std::to_string(c.vertical);
        }
        msg += " in_range=" + std::string(as::within(c, max_distance, vertical_limit) ? "yes" : "no") +
            " facing_dot=" + std::to_string(c.facing_dot) +
            " facing=" + (c.facing_dot >= min_facing_dot() ? "yes" : "no") +
            " eligible=" + (match ? "yes" : "no") +
            " owned=" + (has_swingshot() ? "yes" : "no") +
            " request=" + std::to_string(existing_request);
        mod.info("Auto Swingshot: " + msg);
    }

    PadAction before_hero_update() {
        const auto status = mod.play_status();
        if (seen_loads != status.loads) {
            seen_loads = status.loads;
            reset();
        }
        if (status.phase != OPENRAC_PLAY_PLAYING) {
            reset();
            return PadAction::unchanged;
        }
        openrac_pose hero{};
        hero.size = sizeof(hero);
        if (!mod.hero_pose(hero)) {
            reset();
            return PadAction::unchanged;
        }
        const auto mem = mod.memory();
        const auto held = mem.read<std::uint32_t>(pad() + kPadOffsets[0]);
        const auto pressed = mem.read<std::uint32_t>(pad() + kPadOffsets[1]);
        const bool circle = (held & kCircle) != 0;
        const bool edge = (pressed & kCircle) != 0 && !circle_was_down;
        circle_was_down = circle;
        const auto request_value = mem.read<std::uint32_t>(request());

        if (phase == Phase::waiting) {
            if (hero.item == as::kSwingshot &&
                mem.read<std::uint32_t>(item_state()) == kReady) {
                // The player might have turned around while the item was
                // being equipped. Do not synthesize a delayed shot when the
                // target is no longer in front and in range.
                if (scan_targets(hero).eligible.slot < 0) {
                    mod.info("Auto Swingshot: lost facing/range before firing; cancelling shot");
                    if (previous_item > 0 && previous_item != as::kSwingshot &&
                        mem.read<std::uint8_t>(owned() + previous_item) != 0) {
                        mem.write<std::uint32_t>(request(),
                                                 static_cast<std::uint32_t>(previous_item));
                        mod.info("Auto Swingshot: restoring item " +
                                 std::to_string(previous_item));
                    }
                    reset();
                    return PadAction::suppress_circle;
                }
                phase = Phase::active;
                rest = 0;
                mod.info("Auto Swingshot: equipped; handing Circle to the game");
                // A one-frame press also covers players who tapped Circle
                // before the item transition finished.
                return PadAction::pulse_circle;
            }
            if (++wait > kWaitLimit) {
                mod.warn("Auto Swingshot: equipment request timed out");
                if (request_value == as::kSwingshot)
                    mem.write<std::uint32_t>(request(), 0);
                reset();
                return PadAction::unchanged;
            }
            return PadAction::suppress_circle;
        }

        if (phase == Phase::active) {
            // A player-selected weapon overrides this mod immediately.
            if (hero.item != as::kSwingshot || request_value != 0) {
                reset();
                return PadAction::unchanged;
            }
            if (!circle && as::rest_state(hero.state)) {
                ++rest;
                if (rest >= kRestFrames) {
                    if (previous_item > 0 && previous_item != as::kSwingshot &&
                        mem.read<std::uint8_t>(owned() + previous_item) != 0) {
                        mem.write<std::uint32_t>(request(), static_cast<std::uint32_t>(previous_item));
                        mod.info("Auto Swingshot: restoring item " + std::to_string(previous_item));
                    }
                    reset();
                }
            } else {
                rest = 0;
            }
            return PadAction::unchanged;
        }

        if (!edge) return PadAction::unchanged;
        const auto targets = scan_targets(hero);
        const bool match = targets.eligible.slot >= 0;
        const auto& selected = match ? targets.eligible : targets.closest;
        circle_pressed(hero, selected, match, request_value);
        if (!experimental) {
            mod.info("Auto Swingshot: activation skipped because Observe mode is selected");
            return PadAction::unchanged;
        }
        if (!match || !has_swingshot() || hero.item == as::kSwingshot ||
            request_value != 0) {
            mod.info("Auto Swingshot: activation skipped (" +
                std::string(!match ? "no in-range target within facing cone" :
                            !has_swingshot() ? "Swingshot not owned" :
                            hero.item == as::kSwingshot ? "Swingshot already equipped" :
                            "another item selection is in progress") + ")");
            return PadAction::unchanged;
        }
        // RAC1 uses 8 for its wrench. The last held weapon is stored separately.
        const int from = hero.item == 8
            ? static_cast<int>(mem.read<std::uint32_t>(last())) : hero.item;
        previous_item = (from >= 0 && from < 37) ? from : 0;
        phase = Phase::waiting;
        wait = rest = 0;
        mem.write<std::uint32_t>(request(), as::kSwingshot);
        mod.info("Auto Swingshot: requested Swingshot (previous item=" +
                 std::to_string(previous_item) + ")");
        return PadAction::suppress_circle;
    }
};

Runtime* runtime = nullptr;
void load(openrac::Mod& mod) {
    if (mod.game() != "rac1" || (mod.version() != "ntsc" && mod.version() != "pal"))
        throw openrac::Error("Auto Swingshot supports RAC1 PAL and NTSC-U only");
    if (mod.api_minor() < 12)
        throw openrac::Error("Auto Swingshot requires OpenRAC mod API 1.12+");
    static Runtime state(mod);
    runtime = &state;
    mod.hook<openrac::rac1::Hero_Update>([](auto original) {
        // Only the game update sees injected/masked gameplay buttons; the real
        // input is restored even when the item changes while Hero_Update runs.
        const auto action = runtime->before_hero_update();
        ScopedPadCircle edit(runtime->mod, runtime->pad(), action);
        original();
    });
    mod.info(std::string("Auto Swingshot: ") +
             (state.experimental ? "AUTO-EQUIP ON (experimental; not game-tested)" : "OBSERVE ONLY") +
             "; range=" + std::to_string(state.max_distance) +
             "; vertical=" + std::to_string(state.vertical_limit) +
             "; facing cone=" + std::to_string(state.facing_degrees) + " degrees");
}
} // namespace
OPENRAC_MOD_CPP("auto-swingshot", "0.1.4", load)
