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
constexpr float kPi = 3.14159265358979323846f;
struct Point { float x=0, y=0, z=0; };
struct Candidate {
  int slot=-1, oclass=-1;
  float horizontal=std::numeric_limits<float>::infinity();
  float vertical=std::numeric_limits<float>::infinity();
  float straight=std::numeric_limits<float>::infinity();
  float facing_dot=-1.0f;
};
inline bool target_class(int c) { return c==kTargetPull || c==kTargetSwing; }
// RAC1's world has Z up and rotation[2] as the hero's XY yaw.
// The facing dot is +1 straight ahead, 0 to the side, -1 directly behind.
inline float facing(const Point& p, const Point& t, float yaw) {
  const float dx=t.x-p.x, dy=t.y-p.y;
  const float length=std::hypot(dx,dy);
  if (!std::isfinite(yaw) || !std::isfinite(length)) return -1.0f;
  if (length < 0.001f) return 1.0f;
  return (dx*std::cos(yaw)+dy*std::sin(yaw))/length;
}
inline Candidate evaluate(const Point& p, const Point& t, int slot, int oclass, float yaw=0.0f) {
  const float dx=t.x-p.x, dy=t.y-p.y, dz=t.z-p.z;
  const float flat=std::hypot(dx,dy);
  return {slot,oclass,flat,std::abs(dz),std::hypot(flat,dz),facing(p,t,yaw)};
}
inline bool within(const Candidate& c,float distance,float vertical_limit) {
  return target_class(c.oclass) && std::isfinite(c.straight) &&
         c.straight<=distance && c.vertical<=vertical_limit;
}
inline float facing_threshold(float degrees) {
  return std::cos(degrees * (kPi / 180.0f));
}
inline bool usable(const Candidate& c,float distance,float vertical_limit,float min_facing_dot) {
  return within(c,distance,vertical_limit) && std::isfinite(c.facing_dot) &&
         c.facing_dot >= min_facing_dot;
}
inline bool rest_state(int s) { return s==kIdle || s==kRun || s==kWalk; }
}
