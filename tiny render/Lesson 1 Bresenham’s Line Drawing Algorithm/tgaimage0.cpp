//**************
// TGA图像格式最大的特点是可以做出 不规则形状的 图形、图像文件，
// 一般图形、图像文件都为四方形，若需要有圆形、菱形甚至是缕空的图像文件时，TGA可就派上用场了! 
// 在工业设计领域，使用三维软件制作出来的图像可以利用TGA格式的优势，
// 在图像内部生成一个Alpha（通道），这个功能方便了在平面软件中的工作。
// TGA图像文件格式资料：https://dev.gameres.com/Program/Visual/Other/TGAFormat.htm
// TGA图像格式加载纹理编程实现：https://blog.csdn.net/xiajun07061225/article/details/7646058
//**************
// 可参考博客：http://www.cppblog.com/xczhang/archive/2007/12/23/39368.html
// 可参考博客：https://www.cnblogs.com/bigboom/articles/2646491.html


#include <iostream>
#include <fstream>
#include <string.h>
#include <time.h>
#include <math.h>

#include "tgaimage0.h"

TGAImage::TGAImage() : data(NULL), width(0), height(0), bytespp(0) 
{
}

TGAImage::TGAImage(int w, int h, int bpp) : data(NULL), width(w), height(h), bytespp(bpp) 
{
	unsigned long nbytes = width * height * bytespp;

	// new的用法：https://www.runoob.com/cplusplus/cpp-dynamic-memory.html
	// new的用法：https://blog.csdn.net/xiaorenwuzyh/article/details/44514815
	data = new unsigned char[nbytes];  // data是char*类型的

	//---给data赋值--
	memset(data, 0, nbytes);
}

TGAImage::TGAImage(const TGAImage& img)  // 这里img加了const的原因是img只是被读取，没有被改写
{
	width = img.width;
	height = img.height;
	bytespp = img.bytespp;

	unsigned long nbytes = width * height * bytespp;

	data = new unsigned char[nbytes];

	//* memcpy的用法：https://baike.baidu.com/item/memcpy/659918?fr=aladdin  *//
	memcpy(data, img.data, nbytes);
}

TGAImage::~TGAImage() 
{
	if (data)    // 这个data放在if()里是什么？data存放的是一个地址，只要地址存在if就判定为true，就delete
		delete[] data;
}


//---操作符重载---
//---成员函数的操作符重载---
//---成员函数和全域函数的区别在于是否含有this pointer---
TGAImage& TGAImage::operator =(const TGAImage& img) 
{
	if (this != &img) 
	{
		if (data) 
			delete[] data;

		width = img.width;
		height = img.height;
		bytespp = img.bytespp;

		unsigned long nbytes = width * height * bytespp;

		data = new unsigned char[nbytes];

		memcpy(data, img.data, nbytes);
	}

	return *this;
}

bool TGAImage::read_tga_file(const char* filename) 
{
	if (data) 
		delete[] data;

	data = NULL;

	std::ifstream in;
	in.open(filename, std::ios::binary);   // ios::binary是因为tga是个二进制文件
	if (!in.is_open())
	{
		std::cerr << "can't open file " << filename << "\n";
		in.close();
		return false;
	}

	TGA_Header header;                       // 这个header是能干啥？
											 // 用来存储我们的文件头

	in.read((char*)&header, sizeof(header)); // header是对象，char*&类型的，后面的则是尺度大小
											 // 这个header是char*&类型，下面data是char*类型
											 // 也许是因为header是个通常的类变量，data是指针变量

	if (!in.good())  // 关于good: https://www.jianshu.com/p/e9fdc4cd3e0f 和 https://stackoverflow.com/questions/41926303/difference-between-ifstream-good-and-boolifstream
	{
		in.close();
		std::cerr << "an error occured while reading the header\n";
		return false;
	}

	width = header.width;    // 计算高度
	height = header.height;  // 计算宽度

	//--关于右移的用法：https://zhidao.baidu.com/question/1048852807976294819.html
	bytespp = header.bitsperpixel >> 3;  // 计算BPP

	// bytespp = bytes per pixel?  每个像素的字节？

	if (width <= 0 || height <= 0 || 
		(bytespp != GRAYSCALE && bytespp != RGB && bytespp != RGBA))   
	{
		in.close();
		std::cerr << "bad bpp (or width/height) value\n";  // bpp = bits per pixel 
														   // 表示色彩深度的单位
														   // 指每保存1像素图点所需要的比特数（bits per pixel）
		return false;
	}

	unsigned long nbytes = bytespp * width * height;

	data = new unsigned char[nbytes];

	//---如果文件头附合未压缩的文件头格式---
	if (3 == header.datatypecode || 2 == header.datatypecode) // 3 2?
	{
		//---读取未压缩的TGA文件---
		in.read((char*)data, nbytes); // data是要读取的对象(一般都是char*类型的)，nbytes就是尺度大小
		
		if (!in.good()) 
		{
			in.close();
			std::cerr << "an error occured while reading the data\n";
			return false;
		}
	}

	//---如果文件头附合压缩的文件头格式---
	else if (10 == header.datatypecode || 11 == header.datatypecode)  // 10 11?
	{
		//--读取压缩的TGA格式--
		if (!load_rle_data(in)) 
		{
			in.close();

			std::cerr << "an error occured while reading the data\n";
			return false;
		}
	}

	else 
	{
		in.close();

		std::cerr << "unknown file format " << (int)header.datatypecode << "\n";
		return false;
	}

	if (!(header.imagedescriptor & 0x20))  // 0x20是空格的意思吗？十进制的32
										   // 32代表32位TGA
	{
		flip_vertically();
	}

	if (header.imagedescriptor & 0x10)     // 0x10是什么意思？十进制的16
										   // 16代表16位TGA
	{
		flip_horizontally();
	}

	std::cerr << width << "x" << height << "/" << bytespp * 8 << "\n";

	in.close();

	return true;
}


bool TGAImage::load_rle_data(std::ifstream& in) 
{
	unsigned long pixelcount = width * height;  // 存储组成图像中的像素数
	unsigned long currentpixel = 0;             // 当前正在读取的像素
	unsigned long currentbyte = 0;				// 当前正在写入的图像数据的字节

	TGAColor colorbuffer;                       // 一个像素的存储空间

	//--开始循环--
	do                                         
	{
		//--存储Id块值的变量--
		unsigned char chunkheader = 0;          // 声明一个变量来存储“块”头
		chunkheader = in.get();   // get()针对二进制文件的读写：http://c.biancheng.net/cpp/biancheng/view/2231.html 和 http://c.biancheng.net/view/1534.html
		
		//--尝试读取块的头--
		if (!in.good()) 
		{
			std::cerr << "an error occured while reading the data\n";
			return false;
		}

		if (chunkheader < 128)  // 如果是RAW块(头)(<128)，头的值是颜色数(负数) 
								// 处理其它头字节之前，我们先读取它并且拷贝到内存中
		{
			chunkheader++;      // 变量值加1，以获取RAW像素的总数

			//--开始像素读取循环--
			for (int i = 0; i < chunkheader; i++)
			{
				//--尝试读取一个像素--
				in.read((char*)colorbuffer.raw, bytespp);

				if (!in.good()) 
				{
					std::cerr << "an error occured while reading the header\n";

					//--如果失败，返回false--
					return false;
				}

				for (int t = 0; t < bytespp; t++)
					data[currentbyte++] = colorbuffer.raw[t]; // 因为bytespp不是3就是4，对应RGB(A)
															  // 所以写“R”“G”“B”“A”字节
															  // 如果是32位图像，要写"A"字节

				//--上述currentbyte++则是依据每像素的字节数增加字节计数器--
				currentpixel++;   // 像素计数器加1 

				if (currentpixel > pixelcount) 
				{
					std::cerr << "Too many pixels read\n";
					return false;
				}
			}
		}

		else      // 如果是RLE头
				  // 如果头大于127，那么它是下一个像素值随后将要重复的次数。
		{
			chunkheader -= 127; // 减去127获得ID Bit的Rid
								// 要获取实际重复的数量，我们将它减去127以除去1bit的的头标示符
								// 读取下一个像素并且依照上述次数连续拷贝它到内存中

			//---读取下一个像素---
			in.read((char*)colorbuffer.raw, bytespp);

			if (!in.good()) 
			{
				std::cerr << "an error occured while reading the header\n";
				return false;
			}

			//---开始循环---
			for (int i = 0; i < chunkheader; i++) // 开启一个循环读取所有的颜色信息
												  // 它将会循环块头中指定的次数
												  // 并且每次循环读取和存储一个像素
			{
				//--拷贝“R, G, B ,A”字节--
				for (int t = 0; t < bytespp; t++)
					data[currentbyte++] = colorbuffer.raw[t];

				//--增加字节计数器--
				currentpixel++;

				if (currentpixel > pixelcount) 
				{
					std::cerr << "Too many pixels read\n";
					return false;
				}
			}
		}
		
		//---是否有更多像素要读取？开始循环走到最后---
	} while (currentpixel < pixelcount); 

	//--返回成功--
	return true;
}


bool TGAImage::write_tga_file(const char* filename, bool rle) 
{
	unsigned char developer_area_ref[4] = { 0, 0, 0, 0 };
	unsigned char extension_area_ref[4] = { 0, 0, 0, 0 };

	unsigned char footer[18] = { 'T','R','U','E','V','I','S','I','O','N',
		'-','X','F','I','L','E','.','\0' };
	
	std::ofstream out;
	out.open(filename, std::ios::binary);

	if (!out.is_open()) 
	{
		std::cerr << "can't open file " << filename << "\n";
		out.close();
		return false;
	}

	TGA_Header header;

	memset((void*)&header, 0, sizeof(header));

	header.bitsperpixel = bytespp << 3;
	header.width = width;
	header.height = height;
	header.datatypecode = (bytespp == GRAYSCALE ? (rle ? 11 : 3) : (rle ? 10 : 2));
	header.imagedescriptor = 0x20; // top-left origin

	out.write((char*)&header, sizeof(header));

	if (!out.good()) 
	{
		out.close();
		std::cerr << "can't dump the tga file\n";

		return false;
	}

	if (!rle) 
	{
		out.write((char*)data, width * height * bytespp);

		if (!out.good())
		{
			std::cerr << "can't unload raw data\n";
			out.close();
			return false;
		}
	}

	else 
	{
		if (!unload_rle_data(out)) 
		{
			out.close();
			std::cerr << "can't unload rle data\n";
			return false;
		}
	}

	out.write((char*)developer_area_ref, sizeof(developer_area_ref));

	if (!out.good()) 
	{
		std::cerr << "can't dump the tga file\n";
		out.close();
		return false;
	}

	out.write((char*)extension_area_ref, sizeof(extension_area_ref));

	if (!out.good()) 
	{
		std::cerr << "can't dump the tga file\n";
		out.close();
		return false;
	}

	out.write((char*)footer, sizeof(footer));

	if (!out.good()) 
	{
		std::cerr << "can't dump the tga file\n";
		out.close();
		return false;
	}

	out.close();
	return true;
}

// TODO: it is not necessary to break a raw chunk 
// for two equal pixels (for the matter of the resulting size)

bool TGAImage::unload_rle_data(std::ofstream& out) 
{
	const unsigned char max_chunk_length = 128;
	unsigned long npixels = width * height;
	unsigned long curpix = 0;

	while (curpix < npixels) 
	{
		unsigned long chunkstart = curpix * bytespp;
		unsigned long curbyte = curpix * bytespp;
		unsigned char run_length = 1;

		bool raw = true;

		while (curpix + run_length < npixels && run_length < max_chunk_length) 
		{
			bool succ_eq = true;

			for (int t = 0; succ_eq && t < bytespp; t++) 
			{
				succ_eq = (data[curbyte + t] == data[curbyte + t + bytespp]);
			}

			curbyte += bytespp;

			if (1 == run_length) 
			{
				raw = !succ_eq;
			}

			if (raw && succ_eq) 
			{
				run_length--;
				break;
			}

			if (!raw && !succ_eq) 
			{
				break;
			}
			run_length++;
		}

		curpix += run_length;

		//--put是专用于输出单个字符的成员函数--
		// put()：http://examradar.com/put-get-functions/ 和 https://blog.csdn.net/sinat_36053757/article/details/68944229
		
		out.put(raw ? run_length - 1 : run_length + 127);

		if (!out.good()) 
		{
			std::cerr << "can't dump the tga file\n";
			return false;
		}

		out.write((char*)(data + chunkstart), (raw ? run_length * bytespp : bytespp));
		
		if (!out.good()) 
		{
			std::cerr << "can't dump the tga file\n";
			return false;
		}
	}
	return true;
}

TGAColor TGAImage::get(int x, int y) 
{
	if (!data || x < 0 || y < 0 || x >= width || y >= height) 
	{
		return TGAColor();
	}

	return TGAColor(data + (x + y * width) * bytespp, bytespp);
}

bool TGAImage::set(int x, int y, TGAColor c)    // c在函数中就表示color 
{
	if (!data || x < 0 || y < 0 || x >= width || y >= height) 
	{
		return false;
	}

	// memcpy：https://baike.baidu.com/item/memcpy
	//**raw表示什么？**
	//**data在这时候是什么？(x+y*w)*bytespp又是什么意思？**
	memcpy(data + (x + y * width) * bytespp, c.raw, bytespp); // 要把c.raw复制给data
															  // bytespp表示要被复制的字节数

	return true;
}

int TGAImage::get_bytespp() 
{
	return bytespp;
}

int TGAImage::get_width() 
{
	return width;
}

int TGAImage::get_height() 
{
	return height;
}

bool TGAImage::flip_horizontally() 
{
	if (!data) 
		return false;

	int half = width >> 1;

	for (int i = 0; i < half; i++) 
	{
		for (int j = 0; j < height; j++) 
		{
			TGAColor c1 = get(i, j);
			TGAColor c2 = get(width - 1 - i, j);

			set(i, j, c2);
			set(width - 1 - i, j, c1);
		}
	}
	return true;
}

bool TGAImage::flip_vertically() 
{
	if (!data) 
		return false;

	unsigned long bytes_per_line = width * bytespp;

	//--关于new和delete的用法：https://www.jianshu.com/writer#/notebooks/41885121/notes/60962619/preview
	unsigned char* line = new unsigned char[bytes_per_line];

	int half = height >> 1;

	for (int j = 0; j < half; j++) 
	{
		unsigned long l1 = j * bytes_per_line;
		unsigned long l2 = (height - 1 - j) * bytes_per_line;

		// memmove的用法：http://c.biancheng.net/cpp/html/156.html
		memmove((void*)line, (void*)(data + l1), bytes_per_line);
		memmove((void*)(data + l1), (void*)(data + l2), bytes_per_line);
		memmove((void*)(data + l2), (void*)line, bytes_per_line);
	}

	delete[] line;
	return true;
}

unsigned char* TGAImage::buffer() 
{
	return data;
}

void TGAImage::clear() 
{
	memset((void*)data, 0, width * height * bytespp);
}

bool TGAImage::scale(int w, int h) 
{
	if (w <= 0 || h <= 0 || !data) 
		return false;

	unsigned char* tdata = new unsigned char[w * h * bytespp];

	int nscanline = 0;
	int oscanline = 0;
	int erry = 0;                                // erry表示y轴上的误差error

	unsigned long nlinebytes = w * bytespp;
	unsigned long olinebytes = width * bytespp;

	for (int j = 0; j < height; j++) 
	{
		int errx = width - w;                    // erry表示y轴上的误差error
		int nx = -bytespp;
		int ox = -bytespp;

		for (int i = 0; i < width; i++)
		{
			ox += bytespp;
			errx += w;

			while (errx >= (int)width)
			{
				errx -= width;
				nx += bytespp;

				memcpy(tdata + nscanline + nx, data + oscanline + ox, bytespp);
			}
		}

		erry += h;
		oscanline += olinebytes;

		while (erry >= (int)height) 
		{
			if (erry >= (int)height << 1) // it means we jump over a scanline 扫描线
				memcpy(tdata + nscanline + nlinebytes, tdata + nscanline, nlinebytes);

			erry -= height;
			nscanline += nlinebytes;
		}
	}

	delete[] data;
	data = tdata;

	width = w;
	height = h;

	return true;
}
