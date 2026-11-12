#pragma once
#include <cmath>
#include <limits>
namespace auto_swingshot {
constexpr int kSwingshot = 12;
constexpr int kTargetPull = 758;
constexpr int kTargetSwing = 803;
constexpr int kIdle = 0;
constexpr int kRun = 2;
constexpr int kWalk = 0x73;
struct Point { float x=0, y=0, z=0; };
struct Candidate {
  int slot=-1, oclass=-1;
  float horizontal=std::numeric_limits<float>::infinity();
  float vertical=std::numeric_limits<float>::infinity();
  float straight=std::numeric_limits<float>::infinity();
};
inline bool target_class(int c) { return c==kTargetPull || c==kTargetSwing; }
inline Candidate evaluate(const Point& p, const Point& t, int slot, int oclass) {
  const float dx=t.x-p.x, dy=t.y-p.y, dz=t.z-p.z;
  const float flat=std::hypot(dx,dy);
  return {slot,oclass,flat,std::abs(dz),std::hypot(flat,dz)};
}
inline bool within(const Candidate& c,float distance,float vertical_limit) {
  return target_class(c.oclass) && std::isfinite(c.straight) &&
         c.straight<=distance && c.vertical<=vertical_limit;
}
inline bool rest_state(int s) { return s==kIdle || s==kRun || s==kWalk; }
}
