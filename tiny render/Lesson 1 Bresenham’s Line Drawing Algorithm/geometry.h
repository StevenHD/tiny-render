#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <cmath>
#include <vector>
#include <cassert>
//C++ assert()函数的作用是现计算表达式expression，如果其值为假(即为0),那么先向stderr打印出一条出错信息，然后通过调用abort来终止程序运行。
#include <iostream>

template <size_t DimCols, size_t DimRows, typename T> class mat; //这个是定义模板类，也可以说是类模板
																 //DimCols = Dimensions of Columns
																 //size_t是什么？暂时还不清楚
template <size_t DIM, typename T> struct vec
{
	// 这个类模板定义的是向量vector，vec就是vector的缩写
	
	// struct也是在定义类模板，和class的唯一区别是对数据成员的默认访问权限的不同
	// struct是public，class是private
	vec () 
	{
		for (size_t i = DIM; i--; data_[i] = T()); //这行should是种新的for语法，虽然我Google了好久也没搜到具体解释。
												   //特点是没有跳出循环的语句，我猜测是size_t限制了i的最小值为0，一旦递减到负数，跳出循环
												   //也就是i从DIM变成0
												   // data_是定义的private数据成员
												   //那么问题又来了，T()是什么意思？
	}
	
	T& operator[] (const size_t i)				   //这个就是所谓的操作符重载overload operator
												   //这里重载的是操作符[]
												   //运算符重载的目的是使得可以在表达式中使用用户定义类型的对象，即用作+或->或[]等运算符的操作数。
	{
		assert(i < DIM);						   //如果assert中的参数为假or负, 那么程序会终止程序
												   //那么这里就是要求 imust小于DIM
		return data_[i];
	}

	const T& operator[] (const size_t i) const     //依然是操作符重载，区别是给返回值类型限定为了const &
												   //para list后加const是tell编译器，在函数中，不能改变当前对象 
	{
		assert(i < DIM);
		return data_[i];
	}

private:
	T data_[DIM]; //这个类的定义明明是关键字struct，但是成员用的private访问权限，我认为应该改成class
};


//////////////////////////////////////////////////////////////////////////////////


template <typename T> struct vec<2, T>				//vec<2, T>是定义2D向量的模板，2是上面的DIM
{
	// <>一般都用于声明和定义template

	vec () : x(T()), y(T()) {}						//默认构造函数
	vec (T X, T Y) : x(X), y(Y) {}					//拷贝构造函数，因为参数是自身类类型，但不是引用&，但找不到更合理的解释了

	template <class U> vec<2, T>(const vec<2, U>& v);  //这个class U是什么，没太懂？只有在同行的 para list 中用到了 vec<2,U>

	T& operator[] (const size_t i)
	{
		assert(i < 2);
		return i <= 0 ? x : y;
	}

	const T& operator[] (const size_t i) const
	{
		assert(i < 2);
		return i <= 0 ? x : y;
	}

	T x, y;
};


//////////////////////////////////////////////////////////////////////////////////


template <typename T> struct vec<3, T>
{
	vec() : x(T()), y(T()), z(T()) {}						//不太明白T()表示的是什么？
	vec(T X, T Y, T Z) : x(X), y(Y), z(Z) {}				//x(X)等表示的又是什么？
															//用于 int 与 float 之间的转换

	template <class U> vec<3, T>(const vec<3, U>& v);
	T& operator[] (const size_t i)
	{
		assert(i < 3);
		return i <= 0 ? x : (1 == i ? y : z);
	}
	const T& operator[] (const size_t i) const
	{
		assert(i < 3);
		return i <= 0 ? x : (1 == i ? y : z);
	}

	float norm()											//3D中加入了norm(), 返回(x^2+y^2+z^2)^0.5
	{
		return std::sqrt(x * x + y * y + z * z);
	}

	vec<3, T> & normalize(T l = 1)							//3D加了一个normalize函数
	{
		*this = (*this) * (l / norm());						//这里用到了this指针
		return *this;
	}

	
	T x, y, z;
};

///////////////////////////////////操作符的定义///////////////////////////////////////////////


template <size_t DIM, typename T> T operator*(const vec<DIM, T>& lhs, const vec<DIM, T>& rhs)		//相同DIM的向量会进行*内积
																									//因为形参是const &，且DT是<DIM,T>
																									//所以是copy ctor
{
	//lhs和rhs应该是一个左，一个右。
	//不过具体是什么，并不知道lhs和rhs的具体含义？
	T ret = T(); // T()表示的是什么意思？
				 // 而且为什么 T可以出现在 = 的左右两边
	for (size_t i = DIM; i--; ret += lhs[i] * rhs[i]); 
	return ret;
}

template <size_t DIM, typename T> vec<DIM, T> operator+(vec<DIM, T> lhs, const vec<DIM, T>& rhs)
{
	// + 和 * 的区别是：+的数据类型是vec<DIM,T>，*的数据类型是T，而且lhs的定义没有const
	for (size_t i = DIM; i--; lhs[i] += rhs[i]);
	return lhs;
	//第二个区别是少了ret，return的内容也不一样
}

template <size_t DIM, typename T> vec<DIM, T> operator-(vec<DIM, T> lhs, const vec<DIM, T>& rhs)
{
	for (size_t i = DIM; i--; lhs[i] -= rhs[i]);
	return lhs;
}

// 怎么定义了2次*乘号？
// 乘号*进行了操作符重载

/////////////////////////////////////typename为T和U的向量之间的运算////////////////////////////////////////////////////////////////

template <size_t DIM, typename T, typename U> vec<DIM, T> operator*(vec<DIM, T> lhs, const U& rhs)
{
	// 这个模板中有3个参数，多了一个U，然后rhs的类型不再是 const vec<DIM,T> 而是 const U&
	for (size_t i = DIM; i--; lhs[i] *= rhs); // 这个rhs没有[]，跟上面不一样
	return lhs;
}

template <size_t DIM, typename T, typename U> vec<DIM, T> operator/(vec<DIM, T> lhs, const U& rhs)
{
	for (size_t i = DIM; i--; lhs[i] /= rhs);
	return lhs;
}


///////////////////////////////////齐次坐标的函数//////////////////////////////////////////////////////////////////

template <size_t LEN, size_t DIM, typename T> vec<LEN, T> embed(const vec<DIM, T>& v, T fill = 1)
{
	// T fill = 1 是默认给定数值吗？
	vec <LEN, T> ret;
	for (size_t i = LEN; i--; ret[i] = (i < DIM ? v[i] : fill));  //embed内嵌的话注重的是length
																  //之前运算符号注重的是dimension
	return ret;  //embed和第一个*乘号都用到了 ret
}

template <size_t LEN, size_t DIM, typename T> vec<LEN, T> proj(const vec<DIM, T>& v)		//LEN和DIM之间DIM更大
{
	//proj的参数只有一个const vec<DIM, T> &v
	vec<LEN, T> ret;
	for (size_t i = LEN; i--; ret[i] = v[i]); //ret就对应了proj中的唯一一个参数v
	return ret;
}

////////////////////////////////////cross product 向量之间的叉积////////////////////////////////////////////////////////////////

template <typename T> vec<3, T> cross(vec<3, T> v1, vec<3, T> v2)
{
	// 对比一下 vec<3,T>和vec<LEN,T>，这个3和LEN是什么对应关系？
	// 这次没有for了，直接上return
	return vec<3, T>(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x);
}

template <size_t DIM, typename T> std::ostream& operator<<(std::ostream& out, vec<DIM, T>& v)
{
	for (unsigned int i = 0; i < DIM; i++)
	{
		out << v[i] << " ";  //这行没看懂。。。
	}
	return out;
}

/////////////////////////////求行列式/////////////////////////////////////////////////////


template <size_t DIM, typename T> struct dt
{
	static T det(const mat<DIM, DIM, T>& src)  //static关键字表示的是什么？
											   //mat是个class，里面包含了3个参数
											   //为什么DIM用了2次？
											   //<>表示的是什么意思？
											   //<>代表c++模板，mat其实是一个模板类，所以参数需要用<>
	{
		T ret = 0;
		for (size_t i = DIM; i--; ret += src[0][i] * src.cofactor(0, i)); //cofactor表示的是余因子，src.cofactor是余因子矩阵
		return ret;
	}
};

template <typename T> struct dt<1, T>
{
	static T det(const mat<1, 1, T>& src) //这里面把DIM换成了1，计算的是一维的
										  //对函数进行重载
										  //为什么要把dt这个类定义2次？
	{
		return src[0][0];
	}
};

//////////////////////////////////定义矩阵Matrix的类////////////////////////////////////////////////


template <size_t DimRows, size_t DimCols, typename T> class mat
{
	//这里是mat这个class的定义
	vec<DimCols, T> rows[DimRows];

public:
	mat() {} //这个是ctor？

	// 矩阵中的列Columns
	vec<DimCols, T>& operator[] (const size_t idx)//这里是对operator的重载
	{
		assert(idx < DimRows);
		return rows[idx];
	}

	const vec<DimCols, T>& operator[] (const size_t idx) const
	{
		//再次重新定义一遍operator，区别是前后加入了两个const
		assert(idx < DimRows);
		return rows[idx];
	}

	// 矩阵中的行Rows
	vec<DimRows, T> col(const size_t idx) const
	{
		assert(idx < DimCols);
		vec<DimRows, T> ret;
		
		for (size_t i = DimRows; i--; ret[i] = rows[i][idx]);
		return ret;
	}

	void set_col(size_t idx, vec<DimRows, T> v)
	{
		assert(idx < DimCols);
		for (size_t i = DimRows; i--; rows[i][idx] = v[i]);
	}

	//////////////////////////////单位矩阵函数identity()/////////////////////////////////////////////////////
	static mat<DimRows, DimCols, T> identity()
	{
		// static跟void这些一样吗？
		mat<DimRows, DimCols, T> ret;
		for (size_t i = DimRows; i--; )
			for (size_t j = DimCols; j--; ret[i][j] = (i == j)); //这个ret[i][j] = (i == j)是什么操作？
		return ret;
	}

	T det() const
	{
		//dt也是类
		return dt<DimCols, T>::det(*this); //这个det是成员吗？函数名字也叫det欸。。。
										   //(*this)表示得是什么？
	}

	mat<DimRows - 1, DimCols - 1, T> get_minor(size_t row, size_t col) const
	{
		//为什么DimRows和DimCols都减1了？
		mat <DimRows - 1, DimCols - 1, T> ret;
		for (size_t i = DimRows - 1; i--; )//这个for到底是什么意思？
										   //对这个for的疑惑是第二个循环条件不是一个逻辑表达式，而是递减i--
			for (size_t j = DimCols - 1; j--; ret[i][j] = rows[i < row ? i : i + 1][j < col ? j : j + 1]);
		return ret;
	}

	T cofactor(size_t row, size_t col) const
	{
		return get_minor(row, col).det() * ((row + col) % 2 ? -1 : 1);
	}

	mat <DimRows, DimCols, T> adjugate() const
	{
		// adjugate matrix 伴随矩阵、共轭矩阵
		mat<DimRows, DimCols, T> ret;
		for (size_t i = DimRows; i--; )
			for (size_t j = DimCols; j--; ret[i][j] = cofactor(i, j));
		return ret;
	}

	////////////////////////////这里是逆矩阵///////////////////////////////////////////////
	mat <DimRows, DimCols, T> invert_transpose()
	{
		//逆矩阵是在着色器shader中使用的
		//求解逆矩阵用的是 “伴随矩阵法”
		//矩阵的逆 = (余因子矩阵 * (1/矩阵的det)).T
		mat <DimRows, DimCols, T> ret = adjugate();
		T tmp = ret[0] * rows[0];
		return ret / tmp;
	}
};

//////////////////////////////////////////////////////////////////////////////////

template <size_t DimRows, size_t DimCols, typename T> vec<DimRows, T> operator*(const mat<DimRows, DimCols, T>& lhs,
																				const vec<DimCols, T>& rhs)
{
	vec<DimRows, T> ret;
	for (size_t i = DimRows; i--; ret[i] = lhs[i] * rhs);
	return ret;
}

template <size_t R1, size_t C1, size_t C2, typename T> mat<R1, C2, T> operator*(const mat<R1, C1, T>& lhs,
																				const mat<C1, C2, T>& rhs)
{
	mat<R1, C2, T> result;
	for (size_t i = R1; i--; )
		for (size_t j = C2; j--; result[i][j] = lhs[i] * rhs.col(j));
	// R1 C1 C2 不都是数据类型么，为什么会被赋成初值呢？
	return result;
}

template <size_t DimRows, size_t DimCols, typename T> mat<DimCols, DimRows, T> operator/(mat<DimRows, DimCols, T> lhs,
																						 const T& rhs)
{
	for (size_t i = DimRows; i--; lhs[i] = lhs[i] / rhs);
	return lhs;
}

template <size_t DimRows, size_t DimCols, class T> std::ostream& operator<<(std::ostream& out, mat<DimRows, DimCols, T>& m)
{
	// class 也可以作为 typename 吗？
	for (size_t i = 0; i < DimRows; i++)
		out << m[i] << std::endl;
	return out;
}


//////////////////////////////////////////////////////////////////////////////////

typedef vec<2, float> Vec2f;
typedef vec<2, int> Vec2i; //这个里面的2我知道表示2维，vec<>整体是个模板
typedef vec<3, float> Vec3f;
typedef vec<3, int> Vec3i;
typedef vec<4, float> Vec4f;
typedef mat<4, 4, float> Matrix; //最后一行这个没看懂。。。


#endif // !__GEOMETRY_H__
