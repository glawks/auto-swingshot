#include "detection.hpp"
#include <cassert>
#include <cmath>
#include <limits>
using namespace auto_swingshot;
int main() {
 auto c=evaluate({0,0,0},{3,4,5},17,758);
 assert(c.slot==17 && c.oclass==758);
 assert(std::abs(c.horizontal-5.0f)<0.001f);
 assert(std::abs(c.straight-std::sqrt(50.0f))<0.001f);
 assert(within(c,10,6));
 assert(!within(c,7,6));
 assert(!within(c,10,4));
 auto wrong=evaluate({0,0,0},{1,1,1},10,749);
 assert(!within(wrong,10,10));
 assert(target_class(758)&&target_class(803)&&!target_class(209));
 assert(rest_state(0)&&rest_state(2)&&rest_state(0x73));
 assert(!rest_state(0x2c));
 // The 30-degree default must reject the 45-degree side target.
 const float narrow=facing_threshold(30.0f);
 const float wide=facing_threshold(60.0f);
 assert(std::abs(narrow-0.8660254f)<0.0001f);
 const float threshold=wide;
 auto front=evaluate({0,0,0},{10,0,5},1,758,0.0f);
 auto back=evaluate({0,0,0},{-10,0,5},2,803,0.0f);
 auto side=evaluate({0,0,0},{0,10,5},3,803,0.0f);
 auto fortyfive=evaluate({0,0,0},{10,10,5},4,803,0.0f);
 assert(usable(front,18,10,threshold));
 assert(usable(fortyfive,18,10,threshold));
 assert(!usable(fortyfive,18,10,narrow));
 assert(usable(front,18,10,narrow));
 assert(!usable(side,18,10,narrow));
 assert(!usable(back,18,10,threshold));
 assert(!usable(side,18,10,threshold));
 assert(!usable(front,9,10,threshold));
 // A front target inside 16 game units qualifies; one just outside does not.
 assert(usable(evaluate({0,0,0},{15.9f,0,0},8,803,0.0f),16,10,narrow));
 assert(!usable(evaluate({0,0,0},{16.1f,0,0},9,803,0.0f),16,10,narrow));
 assert(!usable(front,18,4,threshold));
 assert(usable(evaluate({0,0,0},{0,10,5},5,803,kPi/2),18,10,threshold));
 assert(!usable(evaluate({0,0,0},{0,-10,5},6,803,kPi/2),18,10,threshold));
 assert(!usable(evaluate({0,0,0},{1,0,0},7,758,std::numeric_limits<float>::quiet_NaN()),18,10,threshold));
}
