#include "geometry.h"

// 3D
template <> template <> vec<3, int>::  vec(const vec<3, float> &v) : x(int(v.x + .5f)), 
																	 y(int(v.y + .5f)), 
																	 z(int(v.z + .5f)) {}

template <> template <> vec<3, float>::vec(const vec<3, int> &v)   : x(v.x), 
																	 y(v.y), 
																	 z(v.z) {}

// 分成3D和2D我可以理解，但为什么要分成int和float呢？


// 2D
template <> template <> vec<2, int>::  vec(const vec<2, float> &v) : x(int(v.x + .5f)),
																	 y(int(v.y + .5f)) {}

template <> template <> vec<2, float>::vec(const vec<2, int> &v)   : x(v.x),
																     y(v.y) {}