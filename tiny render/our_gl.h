#ifndef __OUR_GL_H__
#define __OUR_GL_H__

#include "tgaimage.h"
#include "geometry.h"

extern Matrix ModelView; //extern�������ǹ�����������
extern Matrix ViewPort;
extern Matrix Projection;

void viewport(int x, int y, int w, int h);
void projection(float coeff = 0.f); //coeff��ϵ�����������-1/c
void lookat(Vec3f eye, Vec3f center, Vec3f up);

struct IShader
{
	virtual ~IShader();
	virtual Vec4f vertex(int ith_face, int nth_vert) = 0;
	virtual bool fragment(Vec3f bar, TGAColor& color) = 0; //������������˴��麯��
														   //fragment������֪����Ӧ�ĸ�֪ʶ��
};


void triangle(Vec4f* pts, IShader& shader, TGAImage& image, TGAImage& zbuffer);

#endif // !__OUR_GL_H__
