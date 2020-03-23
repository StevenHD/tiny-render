#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <cmath>
#include <vector>
#include <cassert>
#include <iostream>

///////////////////////矩阵类模板///////////////////////
template<size_t DimCols, size_t DimRows, typename T> class mat;


///////////////////////向量类模板///////////////////////
template <size_t DIM, typename T> struct vec 
{
	// 构造函数vec()
	vec() { for (size_t i = DIM; i--; data_[i] = T()); }

	T& operator[](const size_t i) { assert(i < DIM); return data_[i]; }
	const T& operator[](const size_t i) const { assert(i < DIM); return data_[i]; }

private:
	T data_[DIM];
};



/////////////////////2维向量类模板///////////////////////

template <typename T> struct vec<2, T> 
{
	// 构造函数和构造函数重载
	vec() : x(T()), y(T()) {}
	vec(T X, T Y) : x(X), y(Y) {}

	// 成员模板
	template <class U> vec<2, T>(const vec<2, U>& v);

	T& operator[](const size_t i) { assert(i < 2); return i <= 0 ? x : y; }
	const T& operator[](const size_t i) const { assert(i < 2); return i <= 0 ? x : y; }

	// 定义变量，构造函数中调用
	T x, y;
};


/////////////////////3维向量类模板///////////////////////

template <typename T> struct vec<3, T> 
{
	vec() : x(T()), y(T()), z(T()) {}
	vec(T X, T Y, T Z) : x(X), y(Y), z(Z) {}

	template <class U> vec<3, T>(const vec<3, U>& v);

	T& operator[](const size_t i) { assert(i < 3); return i <= 0 ? x : (1 == i ? y : z); }
	const T& operator[](const size_t i) const { assert(i < 3); return i <= 0 ? x : (1 == i ? y : z); }
	
	// 对坐标进行标准化，从而将顶点坐标变换到裁剪空间
	// 顶点坐标变换到NDC，标准化设备坐标
	float norm() { return std::sqrt(x * x + y * y + z * z); }
	vec<3, T>& normalize(T l = 1) { *this = (*this) * (l / norm()); return *this; }

	T x, y, z;
};


/////////////////////操作符重载///////////////////////

template<size_t DIM, typename T> T operator*(const vec<DIM, T>& lhs, const vec<DIM, T>& rhs) 
{
	// 注意下形参lhs和rhs的类型
	// 这里两个都是const，因为它俩只是调用，不会改变它们的数据
	// 不断改变的是ret的数值
	T ret = T();
	for (size_t i = DIM; i--; ret += lhs[i] * rhs[i]);
	return ret;
}


template<size_t DIM, typename T>vec<DIM, T> operator+(vec<DIM, T> lhs, const vec<DIM, T>& rhs) 
{
	// 因为lhs不断更新数值，所以不能是const类型
	for (size_t i = DIM; i--; lhs[i] += rhs[i]);
	return lhs;
}

template<size_t DIM, typename T>vec<DIM, T> operator-(vec<DIM, T> lhs, const vec<DIM, T>& rhs) 
{
	for (size_t i = DIM; i--; lhs[i] -= rhs[i]);
	return lhs;
}

template<size_t DIM, typename T, typename U> vec<DIM, T> operator*(vec<DIM, T> lhs, const U& rhs) 
{
	for (size_t i = DIM; i--; lhs[i] *= rhs);
	return lhs;
}

template<size_t DIM, typename T, typename U> vec<DIM, T> operator/(vec<DIM, T> lhs, const U& rhs) 
{
	for (size_t i = DIM; i--; lhs[i] /= rhs);
	return lhs;
}


/////////////////////////透视投影///////////////////////

// embed()函数将坐标的维度增加1维
// 比如3D变为4D
template<size_t LEN, size_t DIM, typename T> vec<LEN, T> embed(const vec<DIM, T>& v, T fill = 1) 
{
	vec<LEN, T> ret;
	for (size_t i = LEN; i--; ret[i] = (i < DIM ? v[i] : fill));
	return ret;
}

// proj()函数将坐标的维度减少1维
// 比如4D重新变回3D
template<size_t LEN, size_t DIM, typename T> vec<LEN, T> proj(const vec<DIM, T>& v) 
{
	vec<LEN, T> ret;
	for (size_t i = LEN; i--; ret[i] = v[i]);
	return ret;
}


/////////////////////重心法画三角形///////////////////////

// 向量叉乘，返回重心坐标(u, v, 1)
template <typename T> vec<3, T> cross(vec<3, T> v1, vec<3, T> v2)
{
	return vec<3, T>(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x);
}


// 操作符<<重载
template <size_t DIM, typename T> std::ostream& operator<<(std::ostream& out, vec<DIM, T>& v) 
{
	for (unsigned int i = 0; i < DIM; i++) 
	{
		out << v[i] << " ";
	}
	return out;
}


/////////////////////行列式///////////////////////

// 求解行列式的模板类
// 采用“余因子矩阵”
template<size_t DIM, typename T> struct dt 
{
	static T det(const mat<DIM, DIM, T>& src) 
	{
		T ret = 0;
		for (size_t i = DIM; i--; ret += src[0][i] * src.cofactor(0, i));
		return ret;
	}
};


// 求解行列式的模板类，参数列表中的参数变为1个
template<typename T> struct dt<1, T> 
{
	static T det(const mat<1, 1, T>& src) 
	{
		return src[0][0];
	}
};



/////////////////////矩阵类模板的定义///////////////////////

template<size_t DimRows, size_t DimCols, typename T> class mat 
{
	vec<DimCols, T> rows[DimRows];

public:
	mat() {}

	// 操作符[]重载
	// 针对于特定的数据类型，要进行操作符重载
	vec<DimCols, T>& operator[] (const size_t idx) 
	{
		assert(idx < DimRows);
		return rows[idx];
	}

	const vec<DimCols, T>& operator[] (const size_t idx) const 
	{
		assert(idx < DimRows);
		return rows[idx];
	}

	// 针对于特定列idx，更新每一行(row)的值
	vec<DimRows, T> col(const size_t idx) const 
	{
		assert(idx < DimCols);
		vec<DimRows, T> ret;
		for (size_t i = DimRows; i--; ret[i] = rows[i][idx]);
		return ret;
	}

	// 设置一列的值
	void set_col(size_t idx, vec<DimRows, T> v) 
	{
		assert(idx < DimCols);
		for (size_t i = DimRows; i--; rows[i][idx] = v[i]);
	}

	// 单位矩阵
	static mat<DimRows, DimCols, T> identity() 
	{
		mat<DimRows, DimCols, T> ret;

		for (size_t i = DimRows; i--; )
			for (size_t j = DimCols; j--; ret[i][j] = (i == j));
		return ret;
	}

	T det() const 
	{
		return dt<DimCols, T>::det(*this);
	}

	// 利用“剪切矩阵”计算行列式
	mat<DimRows - 1, DimCols - 1, T> get_minor(size_t row, size_t col) const 
	{
		mat<DimRows - 1, DimCols - 1, T> ret;

		for (size_t i = DimRows - 1; i--; )
			for (size_t j = DimCols - 1; j--; ret[i][j] = rows[i < row ? i : i + 1][j < col ? j : j + 1]);
		return ret;
	}

	// 余因子函数
	T cofactor(size_t row, size_t col) const 
	{
		return get_minor(row, col).det() * ((row + col) % 2 ? -1 : 1);
	}

	// 共轭矩阵
	mat<DimRows, DimCols, T> adjugate() const
	{
		mat<DimRows, DimCols, T> ret;

		for (size_t i = DimRows; i--; )
			for (size_t j = DimCols; j--; ret[i][j] = cofactor(i, j));
		return ret;
	}

	// 逆矩阵＋转置矩阵
	mat<DimRows, DimCols, T> invert_transpose() 
	{
		mat<DimRows, DimCols, T> ret = adjugate();
		T tmp = ret[0] * rows[0];
		return ret / tmp;
	}

	// 逆矩阵
	mat<DimCols, DimRows, T> invert() 
	{
		return invert_transpose().transpose();
	}

	// 转置矩阵
	mat<DimCols, DimRows, T> transpose() 
	{
		mat<DimCols, DimRows, T> ret;

		for (size_t i = DimRows; i--; ret[i] = this->col(i));
		return ret;
	}
};


/////////////////////矩阵运算操作符重载///////////////////////

template<size_t DimRows, size_t DimCols, typename T> vec<DimRows, T> 
	operator*(const mat<DimRows, DimCols, T>& lhs, const vec<DimCols, T>& rhs) 
{
	vec<DimRows, T> ret;

	for (size_t i = DimRows; i--; ret[i] = lhs[i] * rhs);
	return ret;
}

template<size_t R1, size_t C1, size_t C2, typename T>mat<R1, C2, T> 
	operator*(const mat<R1, C1, T>& lhs, const mat<C1, C2, T>& rhs) 
{
	mat<R1, C2, T> result;

	for (size_t i = R1; i--; )
		for (size_t j = C2; j--; result[i][j] = lhs[i] * rhs.col(j));
	return result;
}

template<size_t DimRows, size_t DimCols, typename T>mat<DimCols, DimRows, T> 
	operator/(mat<DimRows, DimCols, T> lhs, const T& rhs) 
{
	for (size_t i = DimRows; i--; lhs[i] = lhs[i] / rhs);
	return lhs;
}

template <size_t DimRows, size_t DimCols, class T> std::ostream& 
	operator<<(std::ostream& out, mat<DimRows, DimCols, T>& m) 
{
	for (size_t i = 0; i < DimRows; i++) out << m[i] << std::endl;
	return out;
}


/////////////////////类型名简化///////////////////////

typedef vec<2, float> Vec2f;
typedef vec<2, int>   Vec2i;
typedef vec<3, float> Vec3f;
typedef vec<3, int>   Vec3i;
typedef vec<4, float> Vec4f;
typedef mat<4, 4, float> Matrix;
#endif //__GEOMETRY_H__