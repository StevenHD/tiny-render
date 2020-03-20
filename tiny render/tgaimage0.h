#ifndef __IMAGE_H__
#define __IMAGE_H__

// 上面的是头文件中的防卫式声明

#include <fstream>

// #pragma pack是用来指定数据在内存中的对齐方式
// 具体的pragma pack(push, 1)和下面的#pragma pack(pop)具体解释看链接：
// https://blog.csdn.net/dongyanxia1000/article/details/72843568

#pragma pack(push,1)

// 有关TGA文件的描述：https://baike.baidu.com/item/tga/27071?fromtitle=tga%E6%96%87%E4%BB%B6&fromid=10437960&fr=aladdin

// 关于TGA_Header的信息：http://tgalib.sourceforge.net/api/r28.html

struct TGA_Header 
{
	char idlength;

	char colormaptype;
	char datatypecode;

	short colormaporigin;
	short colormaplength;
	char colormapdepth;

	short x_origin;
	short y_origin;

	short width;
	short height;

	char  bitsperpixel;
	char  imagedescriptor;
};

#pragma pack(pop)



struct TGAColor 
{
	// 1
	//* 1的解释 https://www.cnblogs.com/ggjucheng/archive/2011/12/13/2287009.html *//

	// union允许以不同的数据类型访问相同的内存，因为他们实际上是在内存中的相同位置
	// 它的声明和struct差不多，但功能完全不同
	// union声明的所有元素在内存中占有相同的物理空间,其大小是该声明的最大元素的大小
	// 每一个不同的数据类型。由于所有的元素都指的是内存中的同一位置，修改的内容之一，会影响其他元素的内容

	union 
	{
		struct 
		{
			unsigned char b, g, r, a;  // 这里定义了bgra，也就是蓝绿红alpha
		};

		unsigned char raw[4];
		unsigned int val;
	};

	// 以上这个union的用法是：struct和匿名union，因为union结束的分号前面没有变量名
	// 以上这个结构，struct嵌套union，union再嵌套struct
	// https://www.csdn.net/gather_2c/OtDaEgzsNzItYmxvZwO0O0OO0O0O.html //
	// 关于struct和union的区别，这里不赘述了，https://blog.csdn.net/wulong710/article/details/80287544 //

	int bytespp;

	//---不断地在重写TGAColor()这个操作函数---
	TGAColor() : val(0), bytespp(1) 
	{
	}

	TGAColor(unsigned char R, unsigned char G, unsigned char B, 
		unsigned char A) : b(B), g(G), r(R), a(A), bytespp(4) 
	{
	}

	TGAColor(int v, int bpp) : val(v), bytespp(bpp)
	{
	}

	TGAColor(const TGAColor& c) : val(c.val), bytespp(c.bytespp) 
	{
	}

	TGAColor(const unsigned char* p, int bpp) : val(0), bytespp(bpp) 
	{
		for (int i = 0; i < bpp; i++) 
		{
			raw[i] = p[i];
		}
	}

	//---操作符重载---
	// 有关操作符重载的内容：https://www.jianshu.com/writer#/notebooks/41885121/notes/60866169/preview
	TGAColor& operator =(const TGAColor& c) 
	{
		if (this != &c) 
		{
			bytespp = c.bytespp;
			val = c.val;
		}

		return *this;  // this在参数列表中隐藏了
	}
};


class TGAImage
{
protected:
	//*** data为什么是char*类型的？***
	unsigned char* data;  // data是用来控制整个图像的颜色值
						  // 一般对于图像数据的读取，都使用 char* 类型

	int width;			  // 整个图像的宽度
	int height;			  // 整个图像的高度

	int bytespp;		  // 每像素的字节数(3或4)——RGB或RGBA

	//---通过字面意思可以得到，load_rle_data的意思是
	//---加载游程编码的数据
	//---后面的读取文件和写入文件
	//---写入文件的要卸载游程编码的数据
	bool   load_rle_data(std::ifstream& in); 
	bool unload_rle_data(std::ofstream& out);

public:

	// 关于enum的解释：https://www.runoob.com/w3cnote/cpp-enums-intro-and-strongly-typed.html
	// 关于enum的解释2：https://www.runoob.com/w3cnote/cpp-enum-intro.html
	enum Format 
	{
		GRAYSCALE = 1, RGB = 3, RGBA = 4
	};

	TGAImage();
	TGAImage(int w, int h, int bpp);
	TGAImage(const TGAImage& img);

	bool read_tga_file(const char* filename);
	bool write_tga_file(const char* filename, bool rle = true);

	bool flip_horizontally();
	bool flip_vertically();

	bool scale(int w, int h);

	TGAColor get(int x, int y);

	bool set(int x, int y, TGAColor c);

	~TGAImage();

	TGAImage& operator =(const TGAImage& img);

	int get_width();
	int get_height();

	int get_bytespp();
	unsigned char* buffer();

	void clear();
};

#endif //__IMAGE_H__