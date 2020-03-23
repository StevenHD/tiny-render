#include <cmath>
#include <limits>
#include <cstdlib>

#include <algorithm>

#include "our_gl.h"

// ����ModelView����Projection����Viewport����
Matrix ModelView;
Matrix Projection;
Matrix Viewport;


IShader::~IShader() {}

// viewport���ڶ���һ������
// �β�x��y��ʾ������ڵ����λ��
// �β�w��h��ʾ������ڵĴ�С
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

// ����ͶӰ����Projection Matrix
// coeff��Ȩ��ϵ��w
void projection(float coeff) 
{
	Projection = Matrix::identity();
	Projection[3][2] = coeff;
}

// �۲캯��lookat()
// �βηֱ��ǹ۲��ߵ��۾�eye����������center���Զ����������up
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

// ��ȡ��������(u, v, 1)�ĺ���barycentric()
// A B C�������ε�3������
// PΪ��������һ��
Vec3f barycentric(Vec2f A, Vec2f B, Vec2f C, Vec2f P) 
{
	// S[0]��S[1]�ֱ��ʾ����������x�������y�����
	Vec3f s[2];
	for (int i = 2; i--; ) 
	{
		s[i][0] = C[i] - A[i];
		s[i][1] = B[i] - A[i];
		s[i][2] = A[i] - P[i];
	}

	// �Զ��߽��в�˵õ���������(u, v, 1)
	Vec3f u = cross(s[0], s[1]);
	if (std::abs(u[2]) > 1e-2) // ע��u[2]�Ǹ�����. ���u[2] = 0˵��������ABCû�гɹ�����
		return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);

	return Vec3f(-1, 1, 1); // ������ص��������и�ֵ����դ�����ͻᶪ���������
}

void triangle(Vec4f* pts, IShader& shader, TGAImage& image, float* zbuffer) 
{
	// ����߽��ĳ�ֵ
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
			
			// �õ�ÿ�����ص�zֵ
			float z = pts[0][2] * c.x + pts[1][2] * c.y + pts[2][2] * c.z;
			float w = pts[0][3] * c.x + pts[1][3] * c.y + pts[2][3] * c.z;
			
			int fragDepth = z / w;
			if (c.x < 0 || c.y < 0 || c.z<0 || 
				zbuffer[P.x + P.y * image.get_width()] > fragDepth) 
				
				continue;
			
			bool discard = shader.fragment(c, color);
			
			if (!discard) 
			{
				// ʹ����Ȼ���z-buffer���к������޳�
				zbuffer[P.x + P.y * image.get_width()] = fragDepth;
				image.set(P.x, P.y, color);
			}
		}
	}
}
