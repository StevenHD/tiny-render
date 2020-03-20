#include <vector>
#include <limits>
#include <iostream>
#include <algorithm>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl.h"

Model* model = NULL;

const int width = 800;
const int height = 800;

Vec3f light_dir(1, 1, 0);
//Vec3f       eye(1, 1, 4);
Vec3f       eye(0, 1, 10);
Vec3f    center(0, 0, 0);
Vec3f        up(0, 1, 0);

// It is Phong shading
struct Shader : public IShader
{
	/*	geometry.h
	template<size_t DimRows, size_t DimCols, typename T> class mat
		vec<DimCols, T> rows[DimRows];*/

	// 通过模板类的定义，得到关于varying_uv, DimRows是2， DimCols是3
	// 所以定义的是 Vec3f rows[2] varying_uv, 它是一个2行3列的矩阵
	mat<2, 3, float> varying_uv;  // triangle uv coordinates, written by VS read by FS

	// varying_nrm同理
	mat<3, 3, float> varying_nrm; // 每个顶点被片段着色器插值
	mat<4, 3, float> varying_tri; // triangle coordinates (clip coordinates)
								  // written by VS, read by FS

	// 三角形的标准化设备坐标(Normalized Device Coordinates, NDC)
	// NDC坐标——顶点坐标在顶点着色器中处理过就是标准化设备坐标了
	// NDC是在裁剪空间的坐标，x、y和z值在-1.0到1.0的一小段空间
	// 任何落在范围外的坐标都会被丢弃/裁剪，不会显示在屏幕上
	// NDC坐标下一步会变换成 Screen-space Coordinates
	// 这个过程是通过ViewPort矩阵变换而成的
	// 接下来 Screen-space Coordinates会被变换成片段
	// 然后将片段输入到片段着色器中
	mat<3, 3, float> ndc_tri;	  // triangle in normalized device coordinates

	virtual Vec4f vertex(int iface, int nthvert)
	{
		/*
		void set_col(size_t idx, vec<DimRows, T> v) 
		{
			assert(idx < DimCols);
			for (size_t i = DimRows; i--; rows[i][idx] = v[i]);
		}
		*/

		// 好，不太理解set_col()，我们拿出来看看。iface和nthvert传进来的时候都是0
		// 所以idx是0，v是Vec2f的变量，最终返回的是
		// rows[1][0] = v[1]
		// rows[0][0] = v[0]
		// v[0]表示的是x值，v[1]表示的是y值
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		varying_nrm.set_col(nthvert, proj<3>((Projection * ModelView).invert_transpose() * 
			embed<4>(model->normal(iface, nthvert), 0.f)));

		// Projection * ModelView就是矩阵M，可以将世界空间转换为裁剪空间
		// 后面的embed则是将顶点坐标内嵌成4D的
		// 二者相乘后则是转换后的顶点坐标
		Vec4f gl_Vertex = Projection * ModelView * embed<4>(model->vert(iface, nthvert));

		// set_col的意思我猜是 “设置列”
		// 注意：rows[i][1]和rows[i][2]、rows[i][3]不一样
		// 因为gl_vertex也是根据 nthvert = 0, 1, 2 来分别求得的
		varying_tri.set_col(nthvert, gl_Vertex);

		ndc_tri.set_col(nthvert, proj<3>(gl_Vertex / gl_Vertex[3])); // 使用齐次坐标了？除以w

		return gl_Vertex;
	}

	virtual bool fragment(Vec3f bar, TGAColor& color) 
	{
		// 这里的bn和nrm，跟上节课的没区别，只是为了加以区分罢了
		Vec3f bn = (varying_nrm * bar).normalize();
		Vec2f uv = varying_uv * bar;

		// diff就是光照强度

		/*float diff = std::max(0.f, bn * light_dir);*/
		// 将diff改写了，因为diff比较重要
		mat<3, 3, float> A;
		A[0] = ndc_tri.col(1) - ndc_tri.col(0);  // (p1 - p0)对应(p0p1)向量
												 // A[0][0]/A[0][1]/A[0][2]对应x、y、z三个分量
		A[1] = ndc_tri.col(2) - ndc_tri.col(0);  // Same as above
		A[2] = bn;								 // bn表示的是原始法线向量n

		mat<3, 3, float> AI = A.invert();		 // AI是A的逆矩阵

		// (varying_uv[0][1] - varying_uv[0][0]) 表示(u1 - u0)
		// (varying_uv[0][2] - varying_uv[0][0]) 表示(u2 - u0)
		Vec3f i = AI * Vec3f(varying_uv[0][1] - varying_uv[0][0],
							 varying_uv[0][2] - varying_uv[0][0], 
							 0);

		// (varying_uv[1][1] - varying_uv[1][0]) 表示(v1 - v0)
		// (varying_uv[1][2] - varying_uv[1][0]) 表示(v2 - v0)
		Vec3f j = AI * Vec3f(varying_uv[1][1] - varying_uv[1][0],
							 varying_uv[1][2] - varying_uv[1][0], 
							 0);

		/*
		void set_col(size_t idx, vec<DimRows, T> v)
		{
			assert(idx < DimCols);
			for (size_t i = DimRows; i--; rows[i][idx] = v[i]);
		}
		*/

		// Change of basis in 3D space
		// 向量(i j bn)是Darboux坐标系的基准
		mat<3, 3, float> B;
		B.set_col(0, i.normalize()); //rows[2][0] = (u1 - u0), rows[1][0] = (u2 - u0), rows[0][0] = (0)
		B.set_col(1, j.normalize()); //rows[2][1] = (v1 - v0), rows[1][1] = (v2 - v0), rows[0][1] = (0)
		B.set_col(2, bn);  // rows[2][2] = bn.x, rows[1][2] = bn.y, rows[0][2] = bn.z

		// 新的法线向量n(Darboux框架)
		// 把其他相关向量转换到切线空间
		Vec3f n = (B * model->normal(uv)).normalize();

		float diff = std::max(0.f, n * light_dir);

		// diffuse()得到的是纹理坐标(u和v的坐标)，通过u和v的坐标，得到纹理的颜色
		// 纹理颜色和光强相乘，最后得到绘在图上的颜色color
		// 最后这个颜色color是在our_gl.cpp文件中的光栅格化器triangle()函数中应用的
		color = model->diffuse(uv) * diff; // 上面改了那么多代码，就是为了更改color

		return false;
	}
};

int main(int argc, char** argv)
{
	float* zbuffer = new float[width * height];
	for (int i = width * height; i--; zbuffer[i] = -std::numeric_limits<float>::max());

	TGAImage frame(width, height, TGAImage::RGB);

	lookat(eye, center, up);

	// 试图窗口
	viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);

	// 将透视系数传入透视矩阵
	projection(-1.f / (eye - center).norm());

	light_dir = proj<3>((Projection * ModelView * embed<4>(light_dir, 0.f))).normalize();

	// for (int m = 1; m < argc; m++)
	// {
		// model = new Model(argv[m]);
		model = new Model("obj/african_head/african_head.obj");

		// 创建一个着色器对象，从而我们可以开始使用着色器
		Shader shader;

		for (int i = 0; i < model->nfaces(); i++)
		{

			for (int j = 0; j < 3; j++) 
			{
				shader.vertex(i, j);
			}
			triangle(shader.varying_tri, shader, frame, zbuffer);
		}
		delete model;
	//}

	frame.flip_vertically(); // to place the origin in the bottom left corner of the image
	frame.write_tga_file("framebuffer-BTNM.tga");

	delete[] zbuffer;
	return 0;
}