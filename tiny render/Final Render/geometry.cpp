#include "geometry.h"

////////////////////////成员模板特例化///////////////////////////


/////////////////////////2维向量////////////////////////////

					/* v是顶点坐标 */

template <> template <> vec<2, int> :: vec(const vec<2, float> &v) :
	x(int(v.x + .5f)), y(int(v.y + .5f)) {}

template <> template <> vec<2, float>::vec(const vec<2, int> &v) :
	x(v.x), y(v.y) {}


/////////////////////////3维向量////////////////////////////

template <> template <> vec<3, int> ::vec(const vec<3, float>& v) :
	x(int(v.x + .5f)), y(int(v.y + .5f)), z(int(v.z + .5f)) {}

template <> template <> vec<3, float>::vec(const vec<3, int>& v) :
	x(v.x), y(v.y), z(v.z) {}