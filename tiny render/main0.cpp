// Lesson 0��getting started
// Lesson 1��Bresenham��s Line Drawing Algorithm (����û�䣬main.cpp��������naive segment tracing)
// Lesson 1 �����Lesson0���Ǽ��ˡ������߶θ��١�
// Lesson 1 ����1-5������� ���߶�׷�٣����ٳ�����������
// Lesson 1-6 ����ˡ��߶θ���:���б�����Ϊ������Bresenham��
// Lesson 1-7 ����ˡ��߿���Ⱦ��
  
#include <cmath>            // Lesson 1-5 add
#include "tgaimage0.h"

#include <vector>           // Lesson 1-7 add
#include "geometry0.h"		// Lesson 1-7 add
#include "model0.h"			// Lesson 1-7 add


//---white��ɫ��ȫ��255��red��ɫ����255 0 0---

const TGAColor white = TGAColor(255, 255, 255, 255);  // ��ʽ�ұ�***�﷨��1***
const TGAColor red   = TGAColor(255, 0,   0,   255);

//-----�����ǡ��߿���Ⱦ�� Lesson 1-7 ����ӵ�����

Model* model = NULL;
const int width = 800;
const int height = 800;

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color)
{

	/*for (float t = 0.; t < 1.; t += .1)*/        // Lesson 1-1 / 1-2
	/*for (int x = x0; x <= x1; x++)*/             // Lesson 1-3
	/*{*/
		/*int x = x0 * (1. - t) + x1 * t;*/        // Lesson 1-1 / 1-2
		
		/*float t = (x - x0) / (float)(x1 - x0);*/ // Lesson 1-3
											  // ע�ⲻҪд������������(x-x0)/(x1-x0)���ǵü�float
		/*int y = y0 * (1. - t) + y1 * t;*/

		/*image.set(x, y, color);*/               //**set����û̫�㶮��**
	/*}*/


	//--������Lesson 1-4 ���¼ӵģ�Ϊ�������ȣ�����дһ��--
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1))
	{
		// ���ֱ����б�ʣ����Ǿ͵���ͼ���λ��
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}

	if (x0 > x1)
	{
		// ʹ�������
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	/** ������Lesson 1-4 �Ĳ��ִ��룬Lesson 1-5 Ϊ���Ż����ܣ���д���ⲿ��
	for (int x = x0; x <= x1; x++)
	{
		float t = (x - x0) / (float)(x1 - x0);
		int y = y0 * (1. - t) + y1 * t;

		if (steep)
		{
			// �����ת�ã�����ת��
			image.set(y, x, color);
		}
		else
		{
			image.set(x, y, color);
		}
	} **/
	//--Lesson 1-4 �Ĵ������������--


	//---Lesson 1-5 �������Ż�����
	/*
	int dx = x1 - x0;
	int dy = y1 - y0;
	float derror = std::abs(dy / float(dx));
	float error = 0;

	int y = y0;
	for (int x = x0; x <= x1; x++)
	{
		if (steep)
		{
			image.set(y, x, TGAColor(255, 1));  // ����255��ʾ��ɫֵ����ɫ��1��ʾ���ֽ�����1���ֽ�
		}
		else
		{
			image.set(x, y, TGAColor(255, 1));
		}

		error += derror;


		if (error > .5)
		{
			y += (y1 > y0 ? 1 : -1);
			error -= 1.;
		}
	}
	*/
	//---Lesson 1-5 �������Ż��������������---


	//----Lesson 1-6 ȥ���������ͱ���----
	/*
	int dx = x1 - x0;
	int dy = y1 - y0;

	int derror2 = std::abs(dy) * 2;  // derror2�ĳ�ʼ������ֱ��ͨ��dy���ɵ�
	int error2 = 0;                  // error2��forѭ���е��ۼӣ���dx�������㲻���յ�Ӱ��

	int y = y0;

	for (int x = x0; x <= x1; x++)
	{
		if (steep) 
		{
			image.set(y, x, TGAColor(255, 1));
		}
		else 
		{
			image.set(x, y, TGAColor(255, 1));
		}

		error2 += derror2;

		if (error2 > dx)  // �ȽϾͲ�����֮ǰ����.5�Ƚ��ˣ�����dx�ȴ�С
		{
			y += (y1 > y0 ? 1 : -1); // ��Ӧ�ģ�y�Ӽ�1��error2��dx�������㡣

			error2 -= dx * 2;        // error2��forѭ���е��ۼӣ���dx�������㲻���յ�Ӱ��
									 // ��Ϊerror2��ֵֻ��dy�й�
		}

	}
	//----Lesson 1-6 ȥ���������ͱ��� ���������----
	*/

	//-----Lesson 1-7 �߿���Ⱦ
	for (int x = x0; x <= x1; x++)
	{
		float t = (x - x0) / (float)(x1 - x0);

		int y = y0 * (1. - t) + y1 * t;

		if (steep) 
		{
			image.set(y, x, color);
		}

		else 
		{
			image.set(x, y, color);
		}
	}
	//-----Lesson 1-7 �߿���Ⱦ
}


int main(int argc, char** argv)           //����argc��û�е�����**�﷨��2**
{
	

	/*TGAImage image(100, 100, TGAImage::RGB);*/   // Lesson 1-1~1-6 һֱû�䣬1-7����

	

	/*image.set(52, 41, red);*/           // Lesson 1-1 delete

	/* ��ΪLesson 1-5 ��������3�м�����forѭ�������������ע����
	line(13, 20, 80, 40, image, white);   // Lesson 1-2 add

	line(20, 13, 40, 80, image, red);     // Lesson 1-3 add
	line(80, 40, 13, 20, image, red);     // Lesson 1-3 add
	*/
	

	//---Lesson 1-5 ������forѭ�� 
	//----1-6 û��----
	/*   1-7�¼���ѭ�����������ɾ��
	for (int i = 0; i < 1000000; i++)
	{
		line(13, 20, 80, 40, image, white);   
		line(20, 13, 40, 80, image, red);     
		line(80, 40, 13, 20, image, red);     
	}
	*/

	//-----Lesson 1-7 ���¼ӵ�

	if (2 == argc)
	{
		model = new Model(argv[1]);
	}
	else
	{
		model = new Model("obj/african_head.obj");
	}
	
	TGAImage image(width, height, TGAImage::RGB);

	for (int i = 0; i < model->nfaces(); i++)
	{
		std::vector<int> face = model->face(i);

		for (int j = 0; j < 3; j++)
		{
			Vec3f v0 = model->vert(face[j]);
			Vec3f v1 = model->vert(face[(j + 1) % 3]);

			int x0 = (v0.x + 1.) * width / 2.;
			int y0 = (v0.y + 1.) * height / 2.;
			int x1 = (v1.x + 1.) * width / 2.;
			int y1 = (v1.y + 1.) * height / 2.;

			line(x0, y0, x1, y1, image, white);
		}
	}

	//-----Lesson 1-7 wirdframe rendering ����-----


	image.flip_vertically();  // ��ֱ��ת 
							  // ���뽫ԭ�����ͼƬ�����½�
	image.write_tga_file("output1-7(WireframeRendering).tga");

	delete model;             // Lesson 1-7 wireFrameRendering add

	return 0;
}
