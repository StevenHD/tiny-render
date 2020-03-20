// 如名所示，main3.cpp是Lesson 3中的主函数

#include <vector>
#include <cmath>
#include <limits>
#include "tgaimage0.h"
#include "geometry0.h"

const TGAColor white = TGAColor(255, 255, 255, 255);  // 为什么alpha通道一直是满的255？

const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);

const int width = 800;
const int height = 500;


void line(Vec2i p0, Vec2i p1, TGAImage& image, TGAColor color)
{
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
		// Bresham直线算法
		float t = (x - p0.x) / (float)(p1.x - p0.x);
		int y = p0.y * (1. - t) + p1.y * t + .5;

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


// 栅格化：在通常的应用中，这个术语用来表示在计算机上显示三维形状的流行渲染算法
// 栅格化目前是生成实时三维计算机图形最流行的算法
// 总体上来说，栅格化这个术语可以用于任何将矢量图形转换成位图的过程
void rasterize(Vec2i p0, Vec2i p1, TGAImage& image, TGAColor color, int ybuffer[])
{
	// ybuffer的体现还是个数组——ybuffer[]
	if (p0.x > p1.x)
	{
		std::swap(p0, p1);
	}

	for (int x = p0.x; x <= p1.x; x++)
	{
		float t = (x - p0.x) / (float)(p1.x - p0.x);

		int y = p0.y * (1. - t) + p1.y * t + .5;


		// 不是很知道这个ybuffer能干什么?
		// 这个ybuffer到底干了什么?
		if (ybuffer[x] < y)
		{
			ybuffer[x] = y;          // 这个ybuffer是个什么啊？用x的值做下标
			image.set(x, 0, color);  // 应用了ybuffer，y的值为0
		}
	}
}


int main(int argc, char** argv)
{
	{
		// just dumping the 2d scene (yay we have enough dimensions!)
		TGAImage scene(width, height, TGAImage::RGB);

		// 场景 “2d网格”	
		line(Vec2i(20, 34), Vec2i(744, 400), scene, red);
		line(Vec2i(120, 434), Vec2i(444, 400), scene, green);
		line(Vec2i(330, 463), Vec2i(594, 200), scene, blue);

		// screen line 屏幕线
		line(Vec2i(10, 10), Vec2i(790, 10), scene, white);

		// flip_vertically可以将原点放在图片的左下角
		scene.flip_vertically();    // 不懂flip_vertically()函数实现逻辑是什么样的?
		scene.write_tga_file("scene3-1.tga");
	}

	{
		TGAImage render(width, 16, TGAImage::RGB);

		int ybuffer[width];
		for (int i = 0; i < width; i++)
		{
			ybuffer[i] = std::numeric_limits<int>::min();  // 所以这里ybuffer[i]中的每一个值都是一样的
		}

		rasterize(Vec2i(20, 34), Vec2i(744, 400), render, red, ybuffer);
		rasterize(Vec2i(120, 434), Vec2i(444, 400), render, green, ybuffer);
		rasterize(Vec2i(330, 463), Vec2i(594, 200), render, blue, ybuffer);

		// 1像素宽的图像对眼睛不好，让我们把它放大
		for (int i = 0; i < width; i++)
		{
			for (int j = 1; j < 16; j++) // 这个j为何从1开始？
			{
				render.set(i, j, render.get(i, 0)); // 抽空得好好看看set()和get()函数
			}
		}

		render.flip_vertically();
		render.write_tga_file("render3-1-blue.tga");
	}

	return 0;
}
