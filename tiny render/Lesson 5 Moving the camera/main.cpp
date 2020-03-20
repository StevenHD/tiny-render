#include <vector>
#include <iostream>
#include <cmath>
#include <limits>

#include "tgaimage.h"
#include "geometry.h"
#include "model.h"

const int width = 800;
const int height = 800;
const int depth = 255;

Model* model = NULL;
int* zbuffer = NULL;

Vec3f light_dir = Vec3f(1, -1, 1).normalize();
Vec3f eye(1, 1, 3);
Vec3f center(0, 0, 0);

// viewportǰ2���������ƴ������½ǵ�λ�ã�Ҳ����x��y
// ��3���͵�4���������ơ���Ⱦ���ڡ��Ŀ�Ⱥ͸߶ȣ����أ�
// Viewport�ж���ġ�λ�á��͡���ߡ�����2D�����ת������OpenGL�еġ�λ�����ꡱת��Ϊ����Ļ���ꡱ

// ���磬OpenGL�е�����(-0.5, 0.5)�п��ܣ����գ���ӳ��Ϊ��Ļ�е�����(200,450)
// ע�⣬�������OpenGL���귶ΧֻΪ-1��1
// ���������ʵ�Ͻ�(-1��1)��Χ�ڵ�����ӳ�䵽(0, 800)��(0, 600)

// viewport����������ǽ����ü����ꡱ�任Ϊ����Ļ���ꡱ 
// �ӿڱ任��λ��-1.0��1.0��Χ������任����viewport()��������������귶Χ�� 
// ���任���������꽫���͵���դ��������ת��ΪƬ��
// Ƭ�ξ��ǰѶ��������������γɵ���Ԫ

Matrix viewport(int x, int y, int w, int h)
{
	Matrix m = Matrix::identity(4);

	m[0][3] = x + w / 2.f;
	m[1][3] = y + h / 2.f;
	m[2][3] = depth / 2.f;

	m[0][0] = w / 2.f;
	m[1][1] = h / 2.f;
	m[2][2] = depth / 2.f;

	return m;
}

/*  
	ʹ�þ���ĺô�֮һ�����ʹ��3���໥��ֱ��������ԣ����ᶨ����һ������ռ�
	��������3�������һ��ƽ������������һ������
	���ҿ����������������κ�����������任���Ǹ�����ռ�
*/

// ������һ��lookat����lookat������Ŀռ�С���Ӱ���ռ䡱�����ᴴ��һ������(Look at)����Ŀ��Ĺ۲����
// lookat������Կ�����3���໥��ֱ�����һ������������ռ��λ������
// LookAt������Ϊ���۲���󡱣����԰����С��������ꡱ�任�����ոն���Ĺ۲�ռ䡱

// �ó���ԭ������center��ȥ�����λ������eye�Ľ�������������ָ������z
Matrix lookat(Vec3f eye, Vec3f center, Vec3f up)
{
	// eye���������λ�ã�������ռ���һ��ָ�������λ�õ�����
	// z��ʾ���ǡ���������������������ǡ���Ӱ���ķ���, �����ָ��z�Ḻ����
	Vec3f z = (eye - center).normalize();

	// x����������ռ��x���������Ҳ�ǡ���������
	// ����������Ҳ�ǡ����ᡱ
	// һ��С���ɣ��ȶ���һ��������(Up Vector)
	Vec3f x = (up ^ z).normalize(); // ����������˵Ľ����ͬʱ��ֱ��������

	// up�ǡ�����������y��ʾ���ǡ����ᡱ
	Vec3f y = (z ^ x).normalize();

	Matrix res = Matrix::identity(4);

	for (int i = 0; i < 3; i++)
	{
		// lookat�����һ���ǡ���������
		res[0][i] = x[i];

		// lookat����ڶ����ǡ���������
		res[1][i] = y[i];

		// lookat����������ǡ�����������
		res[2][i] = z[i];

		// i = 0, 1, 2 �ֱ��Ӧ �����λ�������� x y z ����
		// ע�⣬λ���������෴�ģ���Ϊ�������԰�����ƽ�Ƶ������������ƶ��෴�ķ���
		res[i][3] = -center[i];  // ����eyeֻ���ӽǲ�һ��
	}
	return res;
}

void triangle(Vec3i t0, Vec3i t1, Vec3i t2, float ity0, float ity1, 
							float ity2, TGAImage& image, int* zbuffer)
{
	if (t0.y == t1.y && t0.y == t2.y) return; // i dont care about degenerate triangles

	// ��y����Ĵ�С��������
	if (t0.y > t1.y) 
	{
		std::swap(t0, t1); 
		std::swap(ity0, ity1); 
	}

	if (t0.y > t2.y) 
	{
		std::swap(t0, t2); 
		std::swap(ity0, ity2); 
	}

	if (t1.y > t2.y) 
	{
		std::swap(t1, t2); 
		std::swap(ity1, ity2); 
	}

	// �������η�Ϊ2����
	// �м䶥��֮�� �� �м䶥��֮��
	// ͨ��second-half���жϻ����� ��һ����
	int total_height = t2.y - t0.y;
	for (int i = 0; i < total_height; i++)
	{
		bool second_half = i > t1.y - t0.y || t1.y == t0.y;
		int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;

		float alpha = (float)i / total_height;
		float beta = (float)(i - (second_half ? t1.y - t0.y : 0))/ segment_height; // be careful: with above conditions no division by zero here

		Vec3i A = t0 + Vec3f(t2 - t0) * alpha;
		Vec3i B = second_half ? t1 + Vec3f(t2 - t1) * beta : t0 + Vec3f(t1 - t0) * beta;

		// ͨ�������Ա任���õ��߶�A��B
		float ityA = ity0 + (ity2 - ity0) * alpha;
		float ityB = second_half ? ity1 + (ity2 - ity1) * beta : ity0 + (ity1 - ity0) * beta;

		if (A.x > B.x) 
		{
			std::swap(A, B); 
			std::swap(ityA, ityB); 
		}

		for (int j = A.x; j <= B.x; j++) 
		{
			float phi = B.x == A.x ? 1. : (float)(j - A.x) / (B.x - A.x);

			Vec3i    P = Vec3f(A) + Vec3f(B - A) * phi;

			float ityP = ityA + (ityB - ityA) * phi;

			int idx = P.x + P.y * width;

			if (P.x >= width || P.y >= height || P.x < 0 || P.y < 0) continue;

			if (zbuffer[idx] < P.z) 
			{
				zbuffer[idx] = P.z;
				image.set(P.x, P.y, TGAColor(255, 255, 255) * ityP);
			}
		}
	}
}


int main(int argc, char** argv)
{
	if (2 == argc)
	{
		model = new Model(argv[1]);
	}
	else 
	{
		model = new Model("obj/african_head.obj");
	}

	zbuffer = new int[width * height];

	for (int i = 0; i < width * height; i++)
	{
		zbuffer[i] = std::numeric_limits<int>::min();
	}

	/*
		ModelView��ģ�;��󡪡���������ÿ����ı任��������ûʲô�任
		���԰Ѷ����.obj�ļ��ж�ȡ��������ģ�;���ModelView��

		Viewport����ͼ���󡪡�ģ��������۾���λ�ã���ԭ��ƽ�Ƶ��������Ȼ��ͨ����ת����λ��
	*/

	{
		// draw the model
		Matrix ModelView = lookat(eye, center, Vec3f(0, 1, 0));

		Matrix Projection = Matrix::identity(4);

		Matrix ViewPort = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);

		Projection[3][2] = -1.f / (eye - center).norm();

		std::cerr << ModelView << std::endl;
		std::cerr << Projection << std::endl;
		std::cerr << ViewPort << std::endl;

		Matrix z = (ViewPort * Projection * ModelView);

		std::cerr << z << std::endl;

		TGAImage image(width, height, TGAImage::RGB);

		// i��ʾ���ǵ�i����Ԫface��Ҳ���ǵ�i��������
		for (int i = 0; i < model->nfaces(); i++)
		{
			std::vector<int> face = model->face(i); // face����f��һ�У������λ���������
													// ��Ϊ��3�����㣬����face��size��3
			Vec3i screen_coords[3];
			Vec3f world_coords[3];

			float intensity[3];

			for (int j = 0; j < 3; j++)
			{
				// �õ���������һ�����ض��Ķ��㡱
				Vec3f v = model->vert(face[j]);

				screen_coords[j] = Vec3f(ViewPort * Projection * ModelView * Matrix(v));
				world_coords[j] = v;

				// ͨ�����������͹��շ������ö���Ĺ���ǿ��
				// ����ÿһ������Ĺ���ǿ��ֵ���ڵ���triangle()��ʱ��ʹ�м��ֵ���Ա仯
				intensity[j] = model->norm(i, j) * light_dir;
			}
			triangle(screen_coords[0], screen_coords[1], screen_coords[2], 
							intensity[0], intensity[1], intensity[2], image, zbuffer);
		}
		image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
		image.write_tga_file("Lesson5-lookat matrix bugfix.tga");
	}

	{
		// dump z-buffer (debugging purposes only)
		TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
		for (int i = 0; i < width; i++)
		{
			for (int j = 0; j < height; j++) 
			{
				zbimage.set(i, j, TGAColor(zbuffer[i + j * width]));
			}
		}
		zbimage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
		zbimage.write_tga_file("Lesson5-lookat matrix bugfix-zbuffer.tga");
	}

	delete model;
	delete[] zbuffer;

	return 0;
}
