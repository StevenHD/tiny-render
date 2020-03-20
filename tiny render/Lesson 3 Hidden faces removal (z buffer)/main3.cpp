// ������ʾ��main3.cpp��Lesson 3�е�������

#include <vector>
#include <cmath>
#include <limits>
#include "tgaimage0.h"
#include "geometry0.h"

const TGAColor white = TGAColor(255, 255, 255, 255);  // Ϊʲôalphaͨ��һֱ������255��

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
		// Breshamֱ���㷨
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


// դ�񻯣���ͨ����Ӧ���У��������������ʾ�ڼ��������ʾ��ά��״��������Ⱦ�㷨
// դ��Ŀǰ������ʵʱ��ά�����ͼ�������е��㷨
// ��������˵��դ�����������������κν�ʸ��ͼ��ת����λͼ�Ĺ���
void rasterize(Vec2i p0, Vec2i p1, TGAImage& image, TGAColor color, int ybuffer[])
{
	// ybuffer�����ֻ��Ǹ����顪��ybuffer[]
	if (p0.x > p1.x)
	{
		std::swap(p0, p1);
	}

	for (int x = p0.x; x <= p1.x; x++)
	{
		float t = (x - p0.x) / (float)(p1.x - p0.x);

		int y = p0.y * (1. - t) + p1.y * t + .5;


		// ���Ǻ�֪�����ybuffer�ܸ�ʲô?
		// ���ybuffer���׸���ʲô?
		if (ybuffer[x] < y)
		{
			ybuffer[x] = y;          // ���ybuffer�Ǹ�ʲô������x��ֵ���±�
			image.set(x, 0, color);  // Ӧ����ybuffer��y��ֵΪ0
		}
	}
}


int main(int argc, char** argv)
{
	{
		// just dumping the 2d scene (yay we have enough dimensions!)
		TGAImage scene(width, height, TGAImage::RGB);

		// ���� ��2d����	
		line(Vec2i(20, 34), Vec2i(744, 400), scene, red);
		line(Vec2i(120, 434), Vec2i(444, 400), scene, green);
		line(Vec2i(330, 463), Vec2i(594, 200), scene, blue);

		// screen line ��Ļ��
		line(Vec2i(10, 10), Vec2i(790, 10), scene, white);

		// flip_vertically���Խ�ԭ�����ͼƬ�����½�
		scene.flip_vertically();    // ����flip_vertically()����ʵ���߼���ʲô����?
		scene.write_tga_file("scene3-1.tga");
	}

	{
		TGAImage render(width, 16, TGAImage::RGB);

		int ybuffer[width];
		for (int i = 0; i < width; i++)
		{
			ybuffer[i] = std::numeric_limits<int>::min();  // ��������ybuffer[i]�е�ÿһ��ֵ����һ����
		}

		rasterize(Vec2i(20, 34), Vec2i(744, 400), render, red, ybuffer);
		rasterize(Vec2i(120, 434), Vec2i(444, 400), render, green, ybuffer);
		rasterize(Vec2i(330, 463), Vec2i(594, 200), render, blue, ybuffer);

		// 1���ؿ��ͼ����۾����ã������ǰ����Ŵ�
		for (int i = 0; i < width; i++)
		{
			for (int j = 1; j < 16; j++) // ���jΪ�δ�1��ʼ��
			{
				render.set(i, j, render.get(i, 0)); // ��յúúÿ���set()��get()����
			}
		}

		render.flip_vertically();
		render.write_tga_file("render3-1-blue.tga");
	}

	return 0;
}
