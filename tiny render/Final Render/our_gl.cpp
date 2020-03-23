#include <cmath>
#include <limits>
#include <cstdlib>

#include <algorithm>

#include "our_gl.h"

// 声明ModelView矩阵、Projection矩阵、Viewport矩阵
Matrix ModelView;
Matrix Projection;
Matrix Viewport;


IShader::~IShader() {}

// viewport是在定义一个窗口
// 形参x和y表示这个窗口的起点位置
// 形参w和h表示这个窗口的大小
void viewport(int x, int y, int w, int h) 
{
	Viewport = Matrix::identity();

	Viewport[0][3] = x + w / 2.f;
	Viewport[1][3] = y + h / 2.f;
	Viewport[2][3] = depth / 2.f;

	Viewport[0][0] = w / 2.f;
	Viewport[1][1] = h / 2.f;
	Viewport[2][2] = depth / 2.f;
}

// 定义投影矩阵Projection Matrix
// coeff是权重系数w
void projection(float coeff) 
{
	Projection = Matrix::identity();
	Projection[3][2] = coeff;
}

// 观察函数lookat()
// 形参分别是观察者的眼睛eye、场景中心center、自定义的上向量up
void lookat(Vec3f eye, Vec3f center, Vec3f up) 
{
	Vec3f z = (eye - center).normalize();
	Vec3f x = cross(up, z).normalize();
	Vec3f y = cross(z, x).normalize();

	ModelView = Matrix::identity();

	for (int i = 0; i < 3; i++) 
	{
		ModelView[0][i] = x[i];
		ModelView[1][i] = y[i];
		ModelView[2][i] = z[i];

		ModelView[i][3] = -center[i];
	}
}

// 求取重心坐标(u, v, 1)的函数barycentric()
// A B C是三角形的3个顶点
// P为三角形内一点
Vec3f barycentric(Vec2f A, Vec2f B, Vec2f C, Vec2f P) 
{
	// S[0]、S[1]分别表示三个向量的x轴分量和y轴分量
	Vec3f s[2];
	for (int i = 2; i--; ) 
	{
		s[i][0] = C[i] - A[i];
		s[i][1] = B[i] - A[i];
		s[i][2] = A[i] - P[i];
	}

	// 对二者进行叉乘得到重心坐标(u, v, 1)
	Vec3f u = cross(s[0], s[1]);
	if (std::abs(u[2]) > 1e-2) // 注意u[2]是个整数. 如果u[2] = 0说明三角形ABC没有成功生成
		return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);

	return Vec3f(-1, 1, 1); // 如果返回的坐标中有负值，光栅格化器就会丢弃这个坐标
}

void triangle(Vec4f* pts, IShader& shader, TGAImage& image, float* zbuffer) 
{
	// 定义边界框的初值
	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

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
			Vec3f c = barycentric(proj<2>(pts[0] / pts[0][3]), proj<2>(pts[1] / pts[1][3]), proj<2>(pts[2] / pts[2][3]), proj<2>(P));
			
			// 得到每个像素的z值
			float z = pts[0][2] * c.x + pts[1][2] * c.y + pts[2][2] * c.z;
			float w = pts[0][3] * c.x + pts[1][3] * c.y + pts[2][3] * c.z;
			
			int fragDepth = z / w;
			if (c.x < 0 || c.y < 0 || c.z<0 || 
				zbuffer[P.x + P.y * image.get_width()] > fragDepth) 
				
				continue;
			
			bool discard = shader.fragment(c, color);
			
			if (!discard) 
			{
				// 使用深度缓冲z-buffer进行后向面剔除
				zbuffer[P.x + P.y * image.get_width()] = fragDepth;
				image.set(P.x, P.y, color);
			}
		}
	}
}
