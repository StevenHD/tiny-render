#include "geometry.h"

// ���д���ǡ���Աģ��������������Ӧ��������"geometry.h"��
// 4�ֹ��캯�����ǶԲ�ͬά�ȵ�vector�Ͳ�ͬ�������ͱ����ġ���ʼ������ʹ���ˡ���ʼ���б�
// x��y��z������0.5��Ϊ��float��ת��Ϊint��ʱ����������

template <> template <> vec<3, int>  ::vec(const vec<3, float>& v) : 
						x(int(v.x + .5f)), y(int(v.y + .5f)), z(int(v.z + .5f)) {}
template <> template <> vec<3, float>::vec(const vec<3, int>& v) : 
						x(v.x), y(v.y), z(v.z) {}

template <> template <> vec<2, int>  ::vec(const vec<2, float>& v) : 
						x(int(v.x + .5f)), y(int(v.y + .5f)) {}
template <> template <> vec<2, float>::vec(const vec<2, int>& v) : 
						x(v.x), y(v.y) {}