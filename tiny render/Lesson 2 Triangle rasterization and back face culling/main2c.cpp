// Lesson 2 ��ƽ����ɫ��Ⱦ��Ҳ�Ǹ��Ѿ����õ�������Ⱦɫ
// ֮ǰmain2x.cpp�ļ������Щ���ˣ��п�������һ��

#include <vector>
#include <cmath>
#include "tgaimage0.h"
#include "model0.h"
#include "geometry0.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);

Model* model = NULL;  // model�Ǹ�ָ�������ָ���˿յ�ַ

const int width = 800;
const int height = 800;

void line(Vec2i p0, Vec2i p1, TGAImage& image, TGAColor color)
{
	// p0��p1�����㶼����2D��Ļ�ϵģ�����������x��y��ɵ�
	// ��ô��p0��p1�������������ǵ㣬���Ƕ��ǣ�
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


int main(int argc, char** argv)  // argc�������в�������(Argument Count)
								 // argv�������в�������(Argument Vector) 
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

	// light_dir�ǹ��շ���z����Ϊ-1˵���Ǵӱ������������
	Vec3f light_dir(0, 0, -1);

	for (int i = 0; i < model->nfaces(); i++)
	{
		std::vector<int> face = model->face(i);

		// ������Ļ����ϵ��С��ͼ�е���������ϵ����forѭ���в��ϵ�����
		Vec2i screen_coords[3];

		for (int j = 0; j < 3; j++)
		{
			// С��ͼ�У�û�ж��嶥�����v���������v����������world_coordinates�����
			// ����˵������������õ��Ļ��Ƕ���ֵ��ֻ�������ֻ���
			Vec3f world_coords = model->vert(face[j]);

			// Ϊʲô��Ļ�����x��y��Ҫ+1Ȼ���width��height���ٳ�2��
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