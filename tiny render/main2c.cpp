// Lesson 2 的平面着色渲染，也是给已经填充好的三角形染色
// 之前main2x.cpp文件搞得有些乱了，有空了整理一下

#include <vector>
#include <cmath>
#include "tgaimage0.h"
#include "model0.h"
#include "geometry0.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);

Model* model = NULL;  // model是个指针变量，指向了空地址

const int width = 800;
const int height = 800;

void line(Vec2i p0, Vec2i p1, TGAImage& image, TGAColor color)
{
	// p0和p1两个点都是在2D屏幕上的，两个点是由x和y组成的
	// 那么像p0和p1到底是向量还是点，还是都是？
	bool steep = false;
	if (std::abs(p0.x - p1.x) < std::abs(p0.y - p1.y))
	{
		std::swap(p0.x, p0.y);
		std::swap(p1.x, p1.y);
		steep = true;
	}

	if (p0.x > p1.x)
	{
		std::swap(p0, p1);
	}

	for (int x = p0.x; x <= p1.x; x++)
	{
		float t = (x - p0.x) / (float)(p1.x - p0.x);
		int y = p0.y * (1. - t) + p1.y * t;

		if (steep)
		{
			image.set(y, x, color);
		}

		else
		{
			image.set(x, y, color);
		}
	}
}

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage& image, TGAColor color)
{
	if (t0.y == t1.y && t0.y == t2.y) return; // i dont care about degenerate triangles

	if (t0.y > t1.y) std::swap(t0, t1);
	if (t0.y > t2.y) std::swap(t0, t2);
	if (t1.y > t2.y) std::swap(t1, t2);

	int total_height = t2.y - t0.y;

	for (int i = 0; i < total_height; i++)
	{
		bool second_half = i > t1.y - t0.y || t1.y == t0.y;
		int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;

		float alpha = (float)i / total_height;
		float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height; // be careful: with above conditions no division by zero here

		Vec2i A = t0 + (t2 - t0) * alpha;
		Vec2i B = second_half ? t1 + (t2 - t1) * beta : t0 + (t1 - t0) * beta;

		if (A.x > B.x) std::swap(A, B);
		for (int j = A.x; j <= B.x; j++)
		{
			image.set(j, t0.y + i, color); // attention, due to int casts t0.y+i != A.y
		}
	}
}


int main(int argc, char** argv)  // argc：命令行参数个数(Argument Count)
								 // argv：命令行参数向量(Argument Vector) 
{
	if (2 == argc)
	{
		model = new Model(argv[1]);
	}
	else
	{
		model = new Model("obj/african_head.obj");
	}


	TGAImage image(width, height, TGAImage::RGB);

	// light_dir是光照方向，z分量为-1说明是从背面照射过来的
	Vec3f light_dir(0, 0, -1);

	for (int i = 0; i < model->nfaces(); i++)
	{
		std::vector<int> face = model->face(i);

		// 定义屏幕坐标系，小丑图中的世界坐标系是在for循环中不断迭代的
		Vec2i screen_coords[3];

		for (int j = 0; j < 3; j++)
		{
			// 小丑图中，没有定义顶点变量v，顶点变量v由世界坐标world_coordinates替代了
			// 或者说，不是替代，得到的还是顶点值，只不过名字换了
			Vec3f world_coords = model->vert(face[j]);

			// 为什么屏幕坐标的x和y都要+1然后乘width和height，再除2？
			screen_coords[j] = Vec2i((world_coords.x + 1.) * width / 2., (world_coords.y + 1.) * height / 2.);
		}
		
		triangle(screen_coords[0], screen_coords[1], screen_coords[2], image,
				TGAColor(rand() % 255, rand() % 255, rand() % 255, 255));
	}


	image.flip_vertically();
	image.write_tga_file("Lesson2-7clown.tga");

	delete model;

	return 0;

}