#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <cmath>
#include <vector>
#include <cassert>
//C++ assert()�������������ּ�����ʽexpression�������ֵΪ��(��Ϊ0),��ô����stderr��ӡ��һ��������Ϣ��Ȼ��ͨ������abort����ֹ�������С�
#include <iostream>

template <size_t DimCols, size_t DimRows, typename T> class mat; //����Ƕ���ģ���࣬Ҳ����˵����ģ��
																 //DimCols = Dimensions of Columns
																 //size_t��ʲô����ʱ�������
template <size_t DIM, typename T> struct vec
{
	// �����ģ�嶨���������vector��vec����vector����д
	
	// structҲ���ڶ�����ģ�壬��class��Ψһ�����Ƕ����ݳ�Ա��Ĭ�Ϸ���Ȩ�޵Ĳ�ͬ
	// struct��public��class��private
	vec () 
	{
		for (size_t i = DIM; i--; data_[i] = T()); //����should�����µ�for�﷨����Ȼ��Google�˺þ�Ҳû�ѵ�������͡�
												   //�ص���û������ѭ������䣬�Ҳ²���size_t������i����СֵΪ0��һ���ݼ�������������ѭ��
												   //Ҳ����i��DIM���0
												   // data_�Ƕ����private���ݳ�Ա
												   //��ô���������ˣ�T()��ʲô��˼��
	}
	
	T& operator[] (const size_t i)				   //���������ν�Ĳ���������overload operator
												   //�������ص��ǲ�����[]
												   //��������ص�Ŀ����ʹ�ÿ����ڱ��ʽ��ʹ���û��������͵Ķ��󣬼�����+��->��[]��������Ĳ�������
	{
		assert(i < DIM);						   //���assert�еĲ���Ϊ��or��, ��ô�������ֹ����
												   //��ô�������Ҫ�� imustС��DIM
		return data_[i];
	}

	const T& operator[] (const size_t i) const     //��Ȼ�ǲ��������أ������Ǹ�����ֵ�����޶�Ϊ��const &
												   //para list���const��tell���������ں����У����ܸı䵱ǰ���� 
	{
		assert(i < DIM);
		return data_[i];
	}

private:
	T data_[DIM]; //�����Ķ��������ǹؼ���struct�����ǳ�Ա�õ�private����Ȩ�ޣ�����ΪӦ�øĳ�class
};


//////////////////////////////////////////////////////////////////////////////////


template <typename T> struct vec<2, T>				//vec<2, T>�Ƕ���2D������ģ�壬2�������DIM
{
	// <>һ�㶼���������Ͷ���template

	vec () : x(T()), y(T()) {}						//Ĭ�Ϲ��캯��
	vec (T X, T Y) : x(X), y(Y) {}					//�������캯������Ϊ���������������ͣ�����������&�����Ҳ���������Ľ�����

	template <class U> vec<2, T>(const vec<2, U>& v);  //���class U��ʲô��û̫����ֻ����ͬ�е� para list ���õ��� vec<2,U>

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
	vec() : x(T()), y(T()), z(T()) {}						//��̫����T()��ʾ����ʲô��
	vec(T X, T Y, T Z) : x(X), y(Y), z(Z) {}				//x(X)�ȱ�ʾ������ʲô��
															//���� int �� float ֮���ת��

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

	float norm()											//3D�м�����norm(), ����(x^2+y^2+z^2)^0.5
	{
		return std::sqrt(x * x + y * y + z * z);
	}

	vec<3, T> & normalize(T l = 1)							//3D����һ��normalize����
	{
		*this = (*this) * (l / norm());						//�����õ���thisָ��
		return *this;
	}

	
	T x, y, z;
};

///////////////////////////////////�������Ķ���///////////////////////////////////////////////


template <size_t DIM, typename T> T operator*(const vec<DIM, T>& lhs, const vec<DIM, T>& rhs)		//��ͬDIM�����������*�ڻ�
																									//��Ϊ�β���const &����DT��<DIM,T>
																									//������copy ctor
{
	//lhs��rhsӦ����һ����һ���ҡ�
	//����������ʲô������֪��lhs��rhs�ľ��庬�壿
	T ret = T(); // T()��ʾ����ʲô��˼��
				 // ����Ϊʲô T���Գ����� = ����������
	for (size_t i = DIM; i--; ret += lhs[i] * rhs[i]); 
	return ret;
}

template <size_t DIM, typename T> vec<DIM, T> operator+(vec<DIM, T> lhs, const vec<DIM, T>& rhs)
{
	// + �� * �������ǣ�+������������vec<DIM,T>��*������������T������lhs�Ķ���û��const
	for (size_t i = DIM; i--; lhs[i] += rhs[i]);
	return lhs;
	//�ڶ�������������ret��return������Ҳ��һ��
}

template <size_t DIM, typename T> vec<DIM, T> operator-(vec<DIM, T> lhs, const vec<DIM, T>& rhs)
{
	for (size_t i = DIM; i--; lhs[i] -= rhs[i]);
	return lhs;
}

// ��ô������2��*�˺ţ�
// �˺�*�����˲���������

/////////////////////////////////////typenameΪT��U������֮�������////////////////////////////////////////////////////////////////

template <size_t DIM, typename T, typename U> vec<DIM, T> operator*(vec<DIM, T> lhs, const U& rhs)
{
	// ���ģ������3������������һ��U��Ȼ��rhs�����Ͳ����� const vec<DIM,T> ���� const U&
	for (size_t i = DIM; i--; lhs[i] *= rhs); // ���rhsû��[]�������治һ��
	return lhs;
}

template <size_t DIM, typename T, typename U> vec<DIM, T> operator/(vec<DIM, T> lhs, const U& rhs)
{
	for (size_t i = DIM; i--; lhs[i] /= rhs);
	return lhs;
}


///////////////////////////////////�������ĺ���//////////////////////////////////////////////////////////////////

template <size_t LEN, size_t DIM, typename T> vec<LEN, T> embed(const vec<DIM, T>& v, T fill = 1)
{
	// T fill = 1 ��Ĭ�ϸ�����ֵ��
	vec <LEN, T> ret;
	for (size_t i = LEN; i--; ret[i] = (i < DIM ? v[i] : fill));  //embed��Ƕ�Ļ�ע�ص���length
																  //֮ǰ�������ע�ص���dimension
	return ret;  //embed�͵�һ��*�˺Ŷ��õ��� ret
}

template <size_t LEN, size_t DIM, typename T> vec<LEN, T> proj(const vec<DIM, T>& v)		//LEN��DIM֮��DIM����
{
	//proj�Ĳ���ֻ��һ��const vec<DIM, T> &v
	vec<LEN, T> ret;
	for (size_t i = LEN; i--; ret[i] = v[i]); //ret�Ͷ�Ӧ��proj�е�Ψһһ������v
	return ret;
}

////////////////////////////////////cross product ����֮��Ĳ��////////////////////////////////////////////////////////////////

template <typename T> vec<3, T> cross(vec<3, T> v1, vec<3, T> v2)
{
	// �Ա�һ�� vec<3,T>��vec<LEN,T>�����3��LEN��ʲô��Ӧ��ϵ��
	// ���û��for�ˣ�ֱ����return
	return vec<3, T>(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x);
}

template <size_t DIM, typename T> std::ostream& operator<<(std::ostream& out, vec<DIM, T>& v)
{
	for (unsigned int i = 0; i < DIM; i++)
	{
		out << v[i] << " ";  //����û����������
	}
	return out;
}

/////////////////////////////������ʽ/////////////////////////////////////////////////////


template <size_t DIM, typename T> struct dt
{
	static T det(const mat<DIM, DIM, T>& src)  //static�ؼ��ֱ�ʾ����ʲô��
											   //mat�Ǹ�class�����������3������
											   //ΪʲôDIM����2�Σ�
											   //<>��ʾ����ʲô��˼��
											   //<>����c++ģ�壬mat��ʵ��һ��ģ���࣬���Բ�����Ҫ��<>
	{
		T ret = 0;
		for (size_t i = DIM; i--; ret += src[0][i] * src.cofactor(0, i)); //cofactor��ʾ���������ӣ�src.cofactor�������Ӿ���
		return ret;
	}
};

template <typename T> struct dt<1, T>
{
	static T det(const mat<1, 1, T>& src) //�������DIM������1���������һά��
										  //�Ժ�����������
										  //ΪʲôҪ��dt����ඨ��2�Σ�
	{
		return src[0][0];
	}
};

//////////////////////////////////�������Matrix����////////////////////////////////////////////////


template <size_t DimRows, size_t DimCols, typename T> class mat
{
	//������mat���class�Ķ���
	vec<DimCols, T> rows[DimRows];

public:
	mat() {} //�����ctor��

	// �����е���Columns
	vec<DimCols, T>& operator[] (const size_t idx)//�����Ƕ�operator������
	{
		assert(idx < DimRows);
		return rows[idx];
	}

	const vec<DimCols, T>& operator[] (const size_t idx) const
	{
		//�ٴ����¶���һ��operator��������ǰ�����������const
		assert(idx < DimRows);
		return rows[idx];
	}

	// �����е���Rows
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

	//////////////////////////////��λ������identity()/////////////////////////////////////////////////////
	static mat<DimRows, DimCols, T> identity()
	{
		// static��void��Щһ����
		mat<DimRows, DimCols, T> ret;
		for (size_t i = DimRows; i--; )
			for (size_t j = DimCols; j--; ret[i][j] = (i == j)); //���ret[i][j] = (i == j)��ʲô������
		return ret;
	}

	T det() const
	{
		//dtҲ����
		return dt<DimCols, T>::det(*this); //���det�ǳ�Ա�𣿺�������Ҳ��det�G������
										   //(*this)��ʾ����ʲô��
	}

	mat<DimRows - 1, DimCols - 1, T> get_minor(size_t row, size_t col) const
	{
		//ΪʲôDimRows��DimCols����1�ˣ�
		mat <DimRows - 1, DimCols - 1, T> ret;
		for (size_t i = DimRows - 1; i--; )//���for������ʲô��˼��
										   //�����for���ɻ��ǵڶ���ѭ����������һ���߼����ʽ�����ǵݼ�i--
			for (size_t j = DimCols - 1; j--; ret[i][j] = rows[i < row ? i : i + 1][j < col ? j : j + 1]);
		return ret;
	}

	T cofactor(size_t row, size_t col) const
	{
		return get_minor(row, col).det() * ((row + col) % 2 ? -1 : 1);
	}

	mat <DimRows, DimCols, T> adjugate() const
	{
		// adjugate matrix ������󡢹������
		mat<DimRows, DimCols, T> ret;
		for (size_t i = DimRows; i--; )
			for (size_t j = DimCols; j--; ret[i][j] = cofactor(i, j));
		return ret;
	}

	////////////////////////////�����������///////////////////////////////////////////////
	mat <DimRows, DimCols, T> invert_transpose()
	{
		//�����������ɫ��shader��ʹ�õ�
		//���������õ��� ��������󷨡�
		//������� = (�����Ӿ��� * (1/�����det)).T
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
	// R1 C1 C2 ��������������ô��Ϊʲô�ᱻ���ɳ�ֵ�أ�
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
	// class Ҳ������Ϊ typename ��
	for (size_t i = 0; i < DimRows; i++)
		out << m[i] << std::endl;
	return out;
}


//////////////////////////////////////////////////////////////////////////////////

typedef vec<2, float> Vec2f;
typedef vec<2, int> Vec2i; //��������2��֪����ʾ2ά��vec<>�����Ǹ�ģ��
typedef vec<3, float> Vec3f;
typedef vec<3, int> Vec3i;
typedef vec<4, float> Vec4f;
typedef mat<4, 4, float> Matrix; //���һ�����û����������


#endif // !__GEOMETRY_H__
