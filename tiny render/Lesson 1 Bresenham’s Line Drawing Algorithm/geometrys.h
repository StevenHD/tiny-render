#ifndef _GEOMETRY_H__
#define _GEOMETRY_H__

#include <cmath> // If U wanna use abs sin log rand functions
#include <vector>
#include <cassert>
#include <iostream>

/////////////////////////////////////////////////////////////

template <class t> struct Vec2
{
	union 
	{
		struct 
		{ 
			t u;
			t v;
		}

		struct 
		{
			t x;
			t y; 
		};

		t raw[2];
	};

	Vec2() : u(0), v(0) {}
	Vec2(t _u, t _v) : u(_u), v(_v) {}

	inline Vec2<t> operator +(const Vec2<t> &V) const { return Vec2<t>(u + V.u, v + V.v); }
	inline Vec2<t> operator -(const Vec2<t>& V) const { return Vec2<t>(u - V.u, v - V.v); } //这个V.u V.v对应的是哪些？
	inline Vec2<t> operator *(float f)			const { return Vec2<t>(u * f, v * f); }
	template <class > friend std::ostream& operator<<(std::ostream& s, Vec2<t>& v); //这句话属实没看懂
};

typedef Vec2<float> Vec2f;
typedef Vec2<int> Vec2i;

template <class t> std::ostream& operator<<(std::ostream& s, Vec2<t>& v) //没看懂。。。
{//为什么ostream会报错？？？
	s << "(" << v.x << ", " << v.y << ")\n";
	return s;
}


#endif // !_GEOMETRY_H_

