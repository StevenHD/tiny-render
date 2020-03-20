#ifndef __OUR_GL_H__
#define __OUR_GL_H__

#include "tgaimage.h"
#include "geometry.h"

extern Matrix ModelView; //extern的作用是光声明不定义
extern Matrix ViewPort;
extern Matrix Projection;

void viewport(int x, int y, int w, int h);
void projection(float coeff = 0.f); //coeff是系数，求出来是-1/c
void lookat(Vec3f eye, Vec3f center, Vec3f up);

struct IShader
{
	virtual ~IShader();
	virtual Vec4f vertex(int ith_face, int nth_vert) = 0;
	virtual bool fragment(Vec3f bar, TGAColor& color) = 0; //将函数定义成了纯虚函数
														   //fragment函数不知道对应哪个知识点
};


void triangle(Vec4f* pts, IShader& shader, TGAImage& image, TGAImage& zbuffer);

#endif // !__OUR_GL_H__
