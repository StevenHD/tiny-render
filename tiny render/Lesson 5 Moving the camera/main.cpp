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

// viewport前2个参数控制窗口左下角的位置，也就是x和y
// 第3个和第4个参数控制“渲染窗口”的宽度和高度（像素）
// Viewport中定义的“位置”和“宽高”进行2D坐标的转换，将OpenGL中的“位置坐标”转换为“屏幕坐标”

// 例如，OpenGL中的坐标(-0.5, 0.5)有可能（最终）被映射为屏幕中的坐标(200,450)
// 注意，处理过的OpenGL坐标范围只为-1到1
// 因此我们事实上将(-1到1)范围内的坐标映射到(0, 800)和(0, 600)

// viewport矩阵的作用是将“裁剪坐标”变换为“屏幕坐标” 
// 视口变换将位于-1.0到1.0范围的坐标变换到由viewport()函数所定义的坐标范围内 
// 最后变换出来的坐标将会送到光栅器，将其转化为片段
// 片段就是把顶点连接起来所形成的面元

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
	使用矩阵的好处之一是如果使用3个相互垂直（或非线性）的轴定义了一个坐标空间
	可以用这3个轴外加一个平移向量来创建一个矩阵
	并且可以用这个矩阵乘以任何向量来将其变换到那个坐标空间
*/

// 来解释一下lookat矩阵，lookat矩阵定义的空间叫“摄影机空间”，它会创建一个看着(Look at)给定目标的观察矩阵。
// lookat矩阵可以看作是3个相互垂直的轴和一个定义摄像机空间的位置坐标
// LookAt矩阵作为“观察矩阵”，可以把所有“世界坐标”变换到“刚刚定义的观察空间”

// 用场景原点向量center减去摄像机位置向量eye的结果就是摄像机的指向向量z
Matrix lookat(Vec3f eye, Vec3f center, Vec3f up)
{
	// eye是摄像机的位置，是世界空间中一个指向摄像机位置的向量
	// z表示的是“方向向量”，这个方向是“摄影机的方向”, 摄像机指向z轴负方向
	Vec3f z = (eye - center).normalize();

	// x代表摄像机空间的x轴的正方向，也是“右向量”
	// “右向量”也是“右轴”
	// 一个小技巧：先定义一个上向量(Up Vector)
	Vec3f x = (up ^ z).normalize(); // 两个向量叉乘的结果会同时垂直于两向量

	// up是“上向量”，y表示的是“上轴”
	Vec3f y = (z ^ x).normalize();

	Matrix res = Matrix::identity(4);

	for (int i = 0; i < 3; i++)
	{
		// lookat矩阵第一行是“右向量”
		res[0][i] = x[i];

		// lookat矩阵第二行是“上向量”
		res[1][i] = y[i];

		// lookat矩阵第三行是“方向向量”
		res[2][i] = z[i];

		// i = 0, 1, 2 分别对应 摄像机位置向量的 x y z 分量
		// 注意，位置向量是相反的，因为这样可以把世界平移到与我们自身移动相反的方向
		res[i][3] = -center[i];  // 换成eye只是视角不一样
	}
	return res;
}

void triangle(Vec3i t0, Vec3i t1, Vec3i t2, float ity0, float ity1, 
							float ity2, TGAImage& image, int* zbuffer)
{
	if (t0.y == t1.y && t0.y == t2.y) return; // i dont care about degenerate triangles

	// 按y坐标的大小进行排序
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

	// 把三角形分为2部分
	// 中间顶点之上 、 中间顶点之下
	// 通过second-half来判断画得是 哪一条线
	int total_height = t2.y - t0.y;
	for (int i = 0; i < total_height; i++)
	{
		bool second_half = i > t1.y - t0.y || t1.y == t0.y;
		int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;

		float alpha = (float)i / total_height;
		float beta = (float)(i - (second_half ? t1.y - t0.y : 0))/ segment_height; // be careful: with above conditions no division by zero here

		Vec3i A = t0 + Vec3f(t2 - t0) * alpha;
		Vec3i B = second_half ? t1 + Vec3f(t2 - t1) * beta : t0 + Vec3f(t1 - t0) * beta;

		// 通过“线性变换”得到线段A和B
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
		ModelView是模型矩阵——用来计算每个点的变换，但这里没什么变换
		所以把顶点从.obj文件中读取出来就是模型矩阵ModelView了

		Viewport是视图矩阵——模型相对于眼睛的位置，把原点平移到相机处，然后通过旋转调整位置
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

		// i表示的是第i个面元face，也就是第i个三角形
		for (int i = 0; i < model->nfaces(); i++)
		{
			std::vector<int> face = model->face(i); // face则是f那一行，顶点的位置索引序号
													// 因为有3个顶点，所以face的size是3
			Vec3i screen_coords[3];
			Vec3f world_coords[3];

			float intensity[3];

			for (int j = 0; j < 3; j++)
			{
				// 得到三角形中一个“特定的顶点”
				Vec3f v = model->vert(face[j]);

				screen_coords[j] = Vec3f(ViewPort * Projection * ModelView * Matrix(v));
				world_coords[j] = v;

				// 通过法线向量和光照方向计算该顶点的光照强度
				// 保留每一个顶点的光照强度值，在调用triangle()的时候，使中间的值线性变化
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
