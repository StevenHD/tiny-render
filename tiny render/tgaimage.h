#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <fstream>

#pragma pack(push, 1)
struct TGA_Header
{
	char id_length;
	char colormap_type;
	char datatype_code;

	short colormap_origin;
	short colormap_length;
	char colormap_depth;

	short x_origin;
	short y_origin;

	short width;
	short height;

	char bits_perpixel;
	char image_descriptor;
};
#pragma pack(pop)

////////////////////////////////////////////////////////////////////////////////////////////////////


struct TGAColor
{
	unsigned char bgra[4];
	unsigned char bytespp;

	TGAColor() : bgra(), bytespp(1)  //一个 : 后面是初始化
									 //bgra()是默认值
									 //bytespp(1)意思是 bytespp = 1
	{
		for (int i = 0; i < 4; i++)
			bgra[i] = 0;
	}

	TGAColor(unsigned char R, unsigned char G, unsigned char B, unsigned char A = 255) : bgra(), bytespp(4)
	{
		bgra[0] = B;
		bgra[1] = G;
		bgra[2] = R;
		bgra[3] = A;
	}

	TGAColor(unsigned char v) : bgra(), bytespp(1)
	{
		for (int i = 0; i < 4; i++)
			bgra[i] = 0;
		bgra[0] = v;  //这个v是什么？跟RGBA这些有什么区别？是顶点vertex吗？
	}

	TGAColor(const unsigned char* p, unsigned char bpp) : bgra(), bytespp(bpp)
	{
		//bpp是什么？bpp还作为了bytespp()的参数
		//定义指针加入了const
		for (int i = 0; i < (int)bpp; i++)
		{
			bgra[i] = p[i];
			//p本身定义的是个指针，在这里变成了数组p[]，其实数组本身就是指针
		}

		for (int i = bpp; i < 4; i++)
			bgra[i] = 0;
		//一共2个for循环，第一个for循环是i从0到bpp，后一个for循环是i从bpp到4。
		//第一个for循环是将bgra[]的值变成了p[]，第二个for循环是将bgra[]的值变成了0
	}

	unsigned char& operator[](const int i)
	{
		return bgra[i];
	}

	TGAColor operator *(float intensity) const //这一行的*应该是乘号*
	{
		TGAColor res = *this; //这一行的*应该是this指针的解指针符号
		intensity = (intensity > 1.f ? 1.f : (intensity < 0.f ? 0.f : intensity));

		for (int i = 0; i < 4; i++)
			res.bgra[i] = bgra[i] * intensity;
		return res;
	}
};


class TGAImage
{
protected:
	unsigned char* data;
	
	int width;
	int height;

	int bytespp;

	bool load_rle_data(std::ifstream& in);
	bool unload_rle_data(std::ofstream& out);


public:
	enum Format 
	{
		GRAYSCALE = 1, RGB = 3, RGBA = 4
		// enum 枚举，最后一个枚举项的最后面是不加  分号;  的
		//这句话是通道数的一个体现，灰度图一般是1个通道，彩色图一般是3个通道，也可以是4个通道
	};

	TGAImage();
	TGAImage(int w, int h, int bpp); //这个bpp是什么？
	TGAImage(const TGAImage& img);   //为什么img要被定义成const

	bool read_tga_file(const char* filename); //因为模型是在tga文件中的，所以要从tga文件中读取矩阵
											  //不过好奇为什么要把filename定义成const char* 的？
	bool write_tga_file(const char* filename, bool rle = true); //将内容写到tga文件中，不过rle是什么？

	bool flip_horizontally();
	bool flip_vertically();

	bool scale(int w, int h);

	TGAColor get(int x, int y);  //这个get函数是get什么的？坐标点(x, y)吗？

	/*bool set(int x, int y, TGAColor c);*/
	bool set(int x, int y, TGAColor& c);
	bool set(int x, int y, const TGAColor& c);
	//第一次写的时候忘记给类型为TGAColor的变量c加引用符号&了。
	//不过不理解为什么要把set函数写两遍，第二次写set函数是把变量c设置成const的了

	~TGAImage();

	TGAImage& operator = (const TGAImage& img);

	int get_width();
	int get_height();
	int get_bytespp();

	unsigned char* buffer(); //理论中buffer有ybuffer和zbuffer，不知道这个指的是哪个buffer
							 //不过为什么要把buffer定义成指针呢？
	void clear();
};


#endif // !__IMAGE_H__