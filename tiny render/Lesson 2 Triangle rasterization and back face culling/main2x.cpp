/*
	main2.cpp和main0.cpp的区别是：
	main0.cpp的内容是Lesson0 ~ Lesson1中的内容变化
	main2.cpp的内容是Lesson2中的内容
	本来想放一起的，但是感觉加上注释太乱了hhh，当然了，分开也好
	main.cpp是这个系列课程中变化最多，如果一直用一个文件，最后估计乱得看不了

	那么相比于Lesson0和Lesson1，Lesson2中一开始的main.cpp的变化是：
	增加了一个color_green。增加了triangle()函数，因为Lesson2的第一个目标就是画一个填充好的三角形
	原先line()的逻辑没变，但是写法变得“简洁”了，当然，main()内部调用方面也有改变，具体的见代码吧
*/

#include <algorithm>
#include <iostream>        // Lesson2-6 add

#include <vector>
#include <cmath>



#include "tgaimage0.h"
#include "geometry0.h"

using namespace std;


const TGAColor white = TGAColor(255, 255, 255, 255);  // 4个参数的位置分别是R G B A
const TGAColor red   = TGAColor(255, 0, 0, 255);  // 通过构造函数ctor来实例化一个对象
const TGAColor green = TGAColor(0, 255, 0, 255);

const int width = 200;
const int height = 200;


void line(Vec2i p0, Vec2i p1, TGAImage& image, TGAColor color)
{
	//  Vec2i是什么呢？Vec2是一个模板类，Vec2i是模板类中的参数定为int型后的简写
	//  也就是说，类Vec2i不再是个模板类，Vec2i定义的对象都是int型变量，不能是float。
	
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
		float t = (x - p0.x) / (float)(p1.x - p0.x);  // t是从0到1变化的
		int y = p0.y * (1. - t) + p1.y * t;           // Bresham算法，t由x得到，y由y0、y1和t得到

		if (steep)
		{
			image.set(y, x, color);      // 上面交换x和y是为了计算斜率和画直线时对称性的保证
										 // 最后根据x和y画线还是要交换回来
		}
		else
		{
			image.set(x, y, color);
		}
	}
}

/*****因为Lesson2-6中，加入了重心坐标barycentric，所以我们重写triangle()函数，这里就注掉了****

//---画一个三角形中的三条线---
void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage& image, TGAColor color)
{
	/* Lesson2-1的代码，下面写Lesson2-2的代码
	line(t0, t1, image, color);
	line(t1, t2, image, color);
	line(t2, t0, image, color);*/

	/*
	//--Lesson 2-2——将顶点t0 t1 t2从低到高排序(是冒泡排序哦！)
	// t0, t1, t2。这三个点按y坐标升序排列
	if (t0.y > t1.y)
		std::swap(t0, t1);
	if (t0.y > t2.y)
		std::swap(t0, t2);
	if (t1.y > t2.y)
		std::swap(t1, t2);

	line(t0, t1, image, green);
	line(t1, t2, image, green);
	line(t2, t0, image, red);
	*/

	/* 因为下面的Lesson2-4的代码更改了triangle()中的一些内容，所以重写
	//--接下来进入到了Lesson2-3，所以上面的2-2就注释掉了--
	// sort the vertices, t0, t1, t2 lower−to−upper (bubblesort yay!)，试试别人的写法
	if (t0.y > t1.y) std::swap(t0, t1);
	if (t0.y > t2.y) std::swap(t0, t2);
	if (t1.y > t2.y) std::swap(t1, t2);
	
	int total_height = t2.y - t0.y;  // 感觉 (t2.y - t1.y) 也可以

	for (int y = t0.y; y <= t1.y; y++)
	{
		int segment_height = t1.y - t0.y + 1;  // 为什么要加1？可能为了不让segment为0吧

		float alpha = (float)(y - t0.y) / total_height;
		float beta = (float)(y - t0.y) / segment_height; // 注意不要让分母segment_height为0

		Vec2i A = t0 + (t2 - t0) * alpha;
		Vec2i B = t0 + (t1 - t0) * beta;    // 边界B的一部分是由t0到t1构成的

		image.set(A.x, y, red);       // 红色的线段是A，绿色的线段是B
		image.set(B.x, y, green);     // y用同一个，x则是A.x和B.x，为什么？
									  // 也许是因为在枚举y，从t0.y到t1.y(先画t0~t1这部分)
	}
	*/

	/*
	// 这里是Lesson2-4, 增加的内容如下：交换x(如果A.x比B.x大)
	if (t0.y > t1.y) std::swap(t0, t1);
	if (t0.y > t2.y) std::swap(t0, t2);
	if (t1.y > t2.y) std::swap(t1, t2);

	int total_height = t2.y - t0.y;

	for (int y = t0.y; y <= t1.y; y++)
	{
		int segment_height = t1.y - t0.y + 1;

		float alpha = (float)(y - t0.y) / total_height;
		float beta = (float)(y - t0.y) / segment_height; // be careful with divisions by zero 

		Vec2i A = t0 + (t2 - t0) * alpha;
		Vec2i B = t0 + (t1 - t0) * beta;

		// Lesson 2-3 中线段是不连续的，这次为了克服它，我们交换xy。
		// 这里让A.x小于B.x，然后枚举x
		if (A.x > B.x) std::swap(A, B);
		for (int i = A.x; i <= B.x; i++)
		{
			image.set(i, y, color);    // 注意，由于int强制转换 (t0.y + i != A.y)
		}
	}

	// 上半部分和下半部分的区分是根据y来区分的，上面是t0.y~t1.y，这里是t1.y~t2.y
	for (int y = t1.y; y <= t2.y; y++)
	{
		int segment_height = t2.y - t1.y + 1;

		float alpha = (float)(y - t0.y) / total_height;
		float beta = (float)(y - t1.y) / segment_height;

		Vec2i A = t0 + (t2 - t0) * alpha;
		Vec2i B = t1 + (t2 - t1) * beta;

		if (A.x > B.x) std::swap(A, B);
		for (int j = A.x; j <= B.x; j++)
		{
			image.set(j, y, color);
		}
	}
	*/


/*****因为Lesson2-6中，加入了重心坐标barycentric，所以我们重写triangle()函数，这里就注掉了*******

	// Lesson2-5和Lesson2-4的效果一样，但是可读性下降，优点是更方便地维护和更改
	if (t0.y == t1.y && t0.y == t2.y)  // 也就是如果t0 t1 t2的y坐标是一样的，就直接返回了
									   // because I dont care about degenerate triangles  
		return;

	// t0 t1 t2按y坐标大小进行升序排列
	if (t0.y > t1.y) std::swap(t0, t1);
	if (t0.y > t2.y) std::swap(t0, t2);
	if (t1.y > t2.y) std::swap(t1, t2);

	int total_height = t2.y - t0.y;

	for (int i = 0; i < total_height; i++)   // 为什么不是 i <= total_height ? 难道因为i是从0开始的？
	{
		bool second_half = i > t1.y - t0.y || t1.y == t0.y;
		bool segment_height = second_half ? t2.y - t1.y : t1.y - t0.y; // second_half是true表示
																	   // t1.y=t0.y 或者 
																	   // i 比 t.1y到t0.y 的高度要大
																	   // 均说明i进入了第二阶段
																	   // 进入了第二阶段则从
																	   // t1.y - t0.y 换成 
																	   // t2.y - t1.y
		float alpha = (float)i / total_height;  // i表示的是y坐标上的高度差

		// 注意:在上述条件下，这里不能除0，也就是segment_height不能为0
		float beta = (i - (second_half ? t1.y - t0.y : 0)) / segment_height; 

		// Vec2i应该表示的是向量Vector，Vec2i里面的i代表的是整数int
		Vec2i A = t0 + (t2 - t0) * alpha;

		// 向量B的都是×beta
		Vec2i B = second_half ? t1 + (t2 - t1) * beta : t0 + (t1 - t0) * beta; 
		
		if (A.x > B.x) std::swap(A, B);

		for (int j = A.x; j <= B.x; j++)
		{
			image.set(j, t0.y + i, color);
		}
	}
}
*****因为Lesson2-6中，加入了重心坐标barycentric，所以我们重写triangle()函数，这里就注掉了****/


void triangle(Vec2i* pts, TGAImage& image, TGAColor color)
{
	Vec2i bboxmin(image.get_width() - 1, image.get_height() - 1);    // 宽度和高度为什么要减1呢？
	Vec2i bboxmax(0, 0);
	Vec2i clamp(image.get_width() - 1, image.get_height() - 1);

	// 这个嵌套双层for循环应该是初始化赋值，只是我不明白为什么width和height要减1
	// clamp函数是什么意思？
	// bboxmin(bboxmax)[0], bboxmin(bboxmax)[1]代表什么意思？
	// pts[3][2]又是什么意思？为什么求bboxmin是max，bboxmax又是min?
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			// 为什么i是0 1 2，j是0 1呢？
			// bboxmin、bboxmax、clamp三个变量好像都是数组
			// 那么可能就是用struct Vec2中的union中的raw[2]来实例化
			bboxmin[j] = maxVec2i(0, minVec2(bboxmin[j], pts[i][j]));
			bboxmax[j] = minVec2(clamp[j], maxVec2(bboxmax[j], pts[i][j])); // 为什么是拿bboxmax
																				// 和pts来取大小？
		}
	}

	// 定义了2维向量P
	// 这一段是算2维向量P在边框中遍历像素，将向量P的x和y分量就想象成像素
	// bc_scrren存储的就是每一个特定像素的重心坐标
	Vec2i P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
	{
		for (P.y == bboxmin.y; P.y <= bboxmax.y; P.y++)
		{
			// 2维向量P的x和y分量在边框bboxmin和bboxmax中不断枚举
			// bboxmin和bboxmax是边界框的最大最小值，不过也是数组，不太明白bboxmax[0], bboxmax[1]代表什么
			Vec3f bc_screen = barycentric(pts, P); // barycentric函数返回的是点P的矢量方程式
												   // bc_screen直译是重心的屏幕，但是看到Vec3f
												   // 说明bc_screen是一个三维浮点型向量
			
			// bc_screen这个三维向量任意一个分量小于0都不可以
			// 如果这个像素的重心坐标具有至少一个负分量，那么这个像素点就会出现在三角形的外部
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;

			// 如果bc_screen的值符合要求了，就用2维向量P的分量x和y来画图
			image.set(P.x, P.y, color);
		}
	}
}


// 写一个cross_product函数
Vec3f cross_product(Vec3f vect_A, Vec3f vect_B)
{
	float x = vect_A[1] * vect_B[2] - vect_A[2] * vect_B[1];
	float y = vect_A[0] * vect_B[2] - vect_A[2] * vect_B[0];
	float z = vect_A[0] * vect_B[1] - vect_A[1] * vect_A[0];

	return Vec3f(x, y, z);
}

// 写一个maxVec2函数
int maxVec2i(int A, Vec2i B)
{
	if (A > B.x && A < B.y)
		return A;
	return B.x;
}

// 写一个maxVec2函数
Vec2i maxVec2(Vec2i A, Vec2i B)
{
	if (A.x > B.x && A.y > B.y)
		return A;
	return B;
}

// 写一个max函数
Vec2i minVec2(Vec2i A, Vec2i B)
{
	if (A.x < B.x && A.y < B.y)
		return A;
	return B;
}


// Lesson2-6, 也就是计算每一个像素的重心坐标这里，加入了新的函数barycentric()
Vec3f barycentric(Vec2i *pts, Vec2i P)  // barycentric是质心、重心的意思
{
	// 这里，pts传的是指针，也许是因为pts是一个数组
	// Vec3f则是代表三维向量，这里说明，重心是一个三维向量，u也是一个三维向量
	// 这里还没有定义cross()函数，和二维数组pts[][]的两维，各代表什么意思？

	// 这里为什么只有u没有v，u到底是什么？
	Vec3f u = cross_product(Vec3f(pts[2][0] - pts[0][0], pts[1][0] - pts[0][0], pts[0][0] - P[0]), 
					Vec3f(pts[2][1] - pts[0][1], pts[1][1] - pts[0][1], pts[0][1] - P[1]));

	// 因为'pts'和'P'的坐标均是整数值，所以'abs(u[2])'<1就意味着u[2]等于0
	// 这样就意味着————这个三角形没有成功地生成，这种情况下，会返回带有负值的坐标点
	if (std::abs(u[2]) < 1) return Vec3f(-1, 1, 1);    // 这里调用了Vec3f的ctor来生成一个临时变量
													   // 坐标值就是(-1, 1, 1)

	// u.x/u.z表示v，u.y/u.z表示u
	// u是一个三维变量，由上述cross()中的3个点，可以得到u的x y z分量
	// u向量的z分量，则跟二维向量P有关；x和y则和P无关
	return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);

}

/*
// Lesson 2-6 中加入了重心坐标，边界框，等概念，所以重新写main()函数
// 这个Lesson 2-5之前存在的main()函数就先注释掉了！

int main(int argc, char** argv)
{
	TGAImage image(width, height, TGAImage::RGB);  // RGB=3, 是ctor中的bpp

	//--定义3个Vec2i类型的数组t0, t1, t2，数组内均有3个元素---
	//--Vec2i(10, 70)中的10和70是ctor中的u和v，因为ctor中没有x和y相关的
	//--不知道u、v和x、y有什么关系？可以等价吗？
	Vec2i t0[3] = { Vec2i(10, 70), Vec2i(50, 160), Vec2i(70, 80) };
	Vec2i t1[3] = { Vec2i(180, 50), Vec2i(150, 1), Vec2i(70, 180) };
	Vec2i t2[3] = { Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180) };


	triangle(t0[0], t0[1], t0[2], image, red);
	triangle(t1[0], t1[1], t1[2], image, white);
	triangle(t2[0], t2[1], t2[2], image, green);

	image.flip_vertically(); // 想让原点在图像的左下角
	image.write_tga_file("output2-4.tga");

	return 0;

}
*/

// Lesson 2-6 加入了重心坐标和边界框后的main()函数
int main()
{
	// 这次这个新定义的不再叫image了，叫frame，框架
	// 其实和之前的image没啥区别，好理解罢了
	TGAImage frame(200, 200, TGAImage::RGB);

	Vec2i pts[3] = { Vec2i(10, 10), Vec2i(100, 30), Vec2i(190, 160) };

	triangle(pts, frame, TGAColor(255, 0, 0, 255));

	frame.flip_vertically();
	frame.write_tga_file("framebuffer.tga");

	return 0;
}

