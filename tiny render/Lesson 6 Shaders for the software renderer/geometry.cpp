#include "geometry.h"

// 这个写法是“成员模板特例化”，对应的声明在"geometry.h"中
// 4种构造函数则是对不同维度的vector和不同数据类型变量的“初始化”，使用了“初始化列表”
// x、y、z分量加0.5是为了float型转换为int型时的四舍五入

template <> template <> vec<3, int>  ::vec(const vec<3, float>& v) : 
						x(int(v.x + .5f)), y(int(v.y + .5f)), z(int(v.z + .5f)) {}
template <> template <> vec<3, float>::vec(const vec<3, int>& v) : 
						x(v.x), y(v.y), z(v.z) {}

template <> template <> vec<2, int>  ::vec(const vec<2, float>& v) : 
						x(int(v.x + .5f)), y(int(v.y + .5f)) {}
template <> template <> vec<2, float>::vec(const vec<2, int>& v) : 
						x(v.x), y(v.y) {}