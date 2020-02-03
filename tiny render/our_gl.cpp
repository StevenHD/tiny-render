#include <cmath>
#include <limits>
#include <cstdlib>
#include <algorithm>
#include "our_gl.h"

Matrix ModelView; //Matrix就像class一样是个类，Matrix是mat<>的类型别名，mat就是个类
Matrix ViewPort;
Matrix Projection;

IShader::~IShader() {}  //为什么要在这里定义一个析构函数？

void viewport(int x, int y, int w, int h)
{
	ViewPort = Matrix::identity(); 

	ViewPort[0][3] = x + w / 2.f;
	ViewPort[1][3] = y + h / 2.f;
	ViewPort[2][3] = 255.f / 2.f;

	ViewPort[0][0] = w / 2.f;
	ViewPort[1][1] = h / 2.f;
	ViewPort[2][2] = 255.f / 2.f;
}

void projection(float coeff)
{
	//project需要有系数
	Projection = Matrix::identity();
	Projection[3][2] = coeff;
}

void lookat(Vec3f eye, Vec3f center, Vec3f up)
{
	Vec3f z = (eye - center).normalize();
	Vec3f x = cross(up, z).normalize();
	Vec3f y = cross(z, x).normalize();

	ModelView = Matrix::identity();

	for (int i = 0; i < 3; i++)
	{
		//lookat和ModelView一直在互相对应
		ModelView[0][i] = x[i];
		ModelView[1][i] = y[i];
		ModelView[2][i] = z[i];
		ModelView[i][3] = -center[i]; //这个center的概念是什么？
	}
}

Vec3f barycentric(Vec2f A, Vec2f B, Vec2f C, Vec2f P)
{
	//重心坐标是3D的，但是A B C P这4个点为什么都是2D的？
	Vec3f s[2]; //这个s[2]是存储什么元素的？
	for (int i = 2; i--; )
	{
		s[i][0] = C[i] - A[i];//表示向量AC
		s[i][1] = B[i] - A[i];//表示向量AB
		s[i][2] = A[i] - P[i];//表示向量AP
	}
	//A B C P都是数组？
	
	Vec3f u = cross(s[0], s[1]);  //cross表示正交，还是有些疑问？
	if (std::abs(u[2]) > 1e-2)
		return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
	// u.y/u.z表示权重u, put在向量AB上
	// u.x/u.z表示权重v, put在向量AC上
	
	return Vec3f(-1, 1, 1); //z这个return -1表示三角形没有完整地生成
}

void triangle(Vec4f* pts, IShader& shader, TGAImage& image, TGAImage& zbuffer)
{
	Vec2f bboxmin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	//括号里面的2个参数一样啊，我人傻了。。。
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			bboxmin[j] = std::min(bboxmin[j], pts[i][j] / pts[i][3]);
			bboxmax[j] = std::max(bboxmax[j], pts[i][j] / pts[i][3]);
		}
	}

	Vec2i P;
	TGAColor color;

	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
	{
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
		{
			Vec3f c = barycentric(proj<2>(pts[0] / pts[0][3]), proj<2>(pts[1] / pts[1][3]),
								  proj<2>(pts[2] / pts[2][3]), proj<2>(P));
			//proj是个模板，那么proj<2>是个什么意思呢？
			//pts[0]、pts[1]、pts[2]应该是三角形的三个点t0 t1 t2
			//但是pts[0][3]、pts[0][3]、pts[0][3]表示得是什么？三个点对应的重心坐标吗？

			float z = pts[0][2] * c.x + pts[1][2] * c.y + pts[2][2] * c.z;
			float w = pts[0][3] * c.x + pts[1][3] * c.y + pts[2][3] * c.z;
			//这个z和w分别是什么？z对应的是pts[][2]，w对应的是pts[][3]

			int frag_depth = std::max(0, std::min(255, int(z / w + .5)));
			//我感觉frag_depth意味着 the depth of fragment

			if (c.x < 0 || c.y < 0 || c.z < 0 || zbuffer.get(P.x, P.y)[0] > frag_depth)
				// c表示得是center还是barycentric，还是说center就是barycnetric
				continue;

			bool discard = shader.fragment(c, color);
			if (!discard)
			{
				zbuffer.set(P.x, P.y, TGAColor(frag_depth));
				image.set(P.x, P.y, color);
			}
			//discard是哪个方面的知识？
		}
	}

}