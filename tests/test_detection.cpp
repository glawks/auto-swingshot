#include "detection.hpp"
#include <cassert>
#include <cmath>
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
}
