#ifndef __IMAGE_H__
#define __IMAGE_H__

// �������ͷ�ļ��еķ���ʽ����

#include <fstream>

// #pragma pack������ָ���������ڴ��еĶ��뷽ʽ
// �����pragma pack(push, 1)�������#pragma pack(pop)������Ϳ����ӣ�
// https://blog.csdn.net/dongyanxia1000/article/details/72843568

#pragma pack(push,1)

// �й�TGA�ļ���������https://baike.baidu.com/item/tga/27071?fromtitle=tga%E6%96%87%E4%BB%B6&fromid=10437960&fr=aladdin

// ����TGA_Header����Ϣ��http://tgalib.sourceforge.net/api/r28.html

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
	//* 1�Ľ��� https://www.cnblogs.com/ggjucheng/archive/2011/12/13/2287009.html *//

	// union�����Բ�ͬ���������ͷ�����ͬ���ڴ棬��Ϊ����ʵ���������ڴ��е���ͬλ��
	// ����������struct��࣬��������ȫ��ͬ
	// union����������Ԫ�����ڴ���ռ����ͬ������ռ�,���С�Ǹ����������Ԫ�صĴ�С
	// ÿһ����ͬ���������͡��������е�Ԫ�ض�ָ�����ڴ��е�ͬһλ�ã��޸ĵ�����֮һ����Ӱ������Ԫ�ص�����

	union 
	{
		struct 
		{
			unsigned char b, g, r, a;  // ���ﶨ����bgra��Ҳ�������̺�alpha
		};

		unsigned char raw[4];
		unsigned int val;
	};

	// �������union���÷��ǣ�struct������union����Ϊunion�����ķֺ�ǰ��û�б�����
	// ��������ṹ��structǶ��union��union��Ƕ��struct
	// https://www.csdn.net/gather_2c/OtDaEgzsNzItYmxvZwO0O0OO0O0O.html //
	// ����struct��union���������ﲻ׸���ˣ�https://blog.csdn.net/wulong710/article/details/80287544 //

	int bytespp;

	//---���ϵ�����дTGAColor()�����������---
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

	//---����������---
	// �йز��������ص����ݣ�https://www.jianshu.com/writer#/notebooks/41885121/notes/60866169/preview
	TGAColor& operator =(const TGAColor& c) 
	{
		if (this != &c) 
		{
			bytespp = c.bytespp;
			val = c.val;
		}

		return *this;  // this�ڲ����б���������
	}
};


class TGAImage
{
protected:
	//*** dataΪʲô��char*���͵ģ�***
	unsigned char* data;  // data��������������ͼ�����ɫֵ
						  // һ�����ͼ�����ݵĶ�ȡ����ʹ�� char* ����

	int width;			  // ����ͼ��Ŀ��
	int height;			  // ����ͼ��ĸ߶�

	int bytespp;		  // ÿ���ص��ֽ���(3��4)����RGB��RGBA

	//---ͨ��������˼���Եõ���load_rle_data����˼��
	//---�����γ̱��������
	//---����Ķ�ȡ�ļ���д���ļ�
	//---д���ļ���Ҫж���γ̱��������
	bool   load_rle_data(std::ifstream& in); 
	bool unload_rle_data(std::ofstream& out);

public:

	// ����enum�Ľ��ͣ�https://www.runoob.com/w3cnote/cpp-enums-intro-and-strongly-typed.html
	// ����enum�Ľ���2��https://www.runoob.com/w3cnote/cpp-enum-intro.html
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