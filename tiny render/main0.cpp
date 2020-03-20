// Lesson 0：getting started
// Lesson 1：Bresenham’s Line Drawing Algorithm (其他没变，main.cpp则增加了naive segment tracing)
// Lesson 1 相较于Lesson0则是加了“朴素线段跟踪”
// Lesson 1 后面1-5还添加了 “线段追踪，减少除法的数量”
// Lesson 1-6 添加了“线段跟踪:所有变量均为整数的Bresenham”
// Lesson 1-7 添加了“线框渲染”
  
#include <cmath>            // Lesson 1-5 add
#include "tgaimage0.h"

#include <vector>           // Lesson 1-7 add
#include "geometry0.h"		// Lesson 1-7 add
#include "model0.h"			// Lesson 1-7 add


//---white白色就全是255，red红色则是255 0 0---

const TGAColor white = TGAColor(255, 255, 255, 255);  // 等式右边***语法点1***
const TGAColor red   = TGAColor(255, 0,   0,   255);

//-----以下是“线框渲染” Lesson 1-7 中添加的内容

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
											  // 注意不要写成整数除法：(x-x0)/(x1-x0)，记得加float
		/*int y = y0 * (1. - t) + y1 * t;*/

		/*image.set(x, y, color);*/               //**set函数没太搞懂？**
	/*}*/


	//--以下是Lesson 1-4 中新加的，为了完整度，重新写一遍--
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1))
	{
		// 如果直线有斜率，我们就调换图像的位置
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}

	if (x0 > x1)
	{
		// 使其从左到右
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	/** 这里是Lesson 1-4 的部分代码，Lesson 1-5 为了优化性能，重写了这部分
	for (int x = x0; x <= x1; x++)
	{
		float t = (x - x0) / (float)(x1 - x0);
		int y = y0 * (1. - t) + y1 * t;

		if (steep)
		{
			// 如果已转置，请解除转置
			image.set(y, x, color);
		}
		else
		{
			image.set(x, y, color);
		}
	} **/
	//--Lesson 1-4 的代码在这里结束--


	//---Lesson 1-5 的性能优化部分
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
			image.set(y, x, TGAColor(255, 1));  // 这里255表示颜色值，白色，1表示的字节数，1个字节
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
	//---Lesson 1-5 的性能优化部分在这里结束---


	//----Lesson 1-6 去掉浮点数型变量----
	/*
	int dx = x1 - x0;
	int dy = y1 - y0;

	int derror2 = std::abs(dy) * 2;  // derror2的初始化，是直接通过dy生成的
	int error2 = 0;                  // error2在for循环中的累加，与dx进行运算不会收到影响

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

		if (error2 > dx)  // 比较就不再是之前的与.5比较了，而是dx比大小
		{
			y += (y1 > y0 ? 1 : -1); // 相应的，y加减1，error2和dx进行运算。

			error2 -= dx * 2;        // error2在for循环中的累加，与dx进行运算不会收到影响
									 // 因为error2的值只和dy有关
		}

	}
	//----Lesson 1-6 去掉浮点数型变量 在这里结束----
	*/

	//-----Lesson 1-7 线框渲染
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
	//-----Lesson 1-7 线框渲染
}


int main(int argc, char** argv)           //参数argc有没有的区别**语法点2**
{
	

	/*TGAImage image(100, 100, TGAImage::RGB);*/   // Lesson 1-1~1-6 一直没变，1-7变了

	

	/*image.set(52, 41, red);*/           // Lesson 1-1 delete

	/* 因为Lesson 1-5 将下面这3行加入了for循环，所以这里就注掉了
	line(13, 20, 80, 40, image, white);   // Lesson 1-2 add

	line(20, 13, 40, 80, image, red);     // Lesson 1-3 add
	line(80, 40, 13, 20, image, red);     // Lesson 1-3 add
	*/
	

	//---Lesson 1-5 加入了for循环 
	//----1-6 没变----
	/*   1-7新加了循环，所以这段删了
	for (int i = 0; i < 1000000; i++)
	{
		line(13, 20, 80, 40, image, white);   
		line(20, 13, 40, 80, image, red);     
		line(80, 40, 13, 20, image, red);     
	}
	*/

	//-----Lesson 1-7 中新加的

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

	//-----Lesson 1-7 wirdframe rendering 结束-----


	image.flip_vertically();  // 垂直翻转 
							  // 我想将原点放在图片的左下角
	image.write_tga_file("output1-7(WireframeRendering).tga");

	delete model;             // Lesson 1-7 wireFrameRendering add

	return 0;
}
