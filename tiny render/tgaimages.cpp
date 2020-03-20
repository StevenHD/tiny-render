#include <iostream>
#include <fstream>
#include <string.h>
#include <time.h>
#include <math.h>
#include "tgaimage.h"

TGAImage::TGAImage() : data(NULL), width(0), height(0), bytespp(0)
{

}

TGAImage::TGAImage(int w, int h, int bpp) : data(NULL), width(w), height(h), bytespp(bpp)
{
	// data、width、height、bytespp都是TGAImage类中定义的private成员
	// ()中的值就是初始化的值
	// 也就是说 data = NULL，width = w 等等

	unsigned long nbytes = width * height * bytespp;
	data = new unsigned char[nbytes];
	memset(data, 0, nbytes);
}

TGAImage::TGAImage(const TGAImage& img) : data(NULL), width(img.width), height(img.height), bytespp(img.bytespp)
{
	unsigned long nbytes = width * height * bytespp;
	data = new unsigned char[nbytes];
	memcpy(data, img.data, nbytes);
}

TGAImage::~TGAImage()
{
	if (data)
		delete[] data;
}

TGAImage& TGAImage::operator =(const TGAImage& img)
{
	/*if (this != img)*/
	if (this != &img)
	{
		if (data)
			delete[] data;
		
		width   = img.width;
		height  = img.height;
		bytespp = img.bytespp;

		unsigned long nbytes = width * height * bytespp;
		data = new unsigned char[nbytes];
		// data is "char"?
		memcpy(data, img.data, nbytes);
		// What is the difference between data and img.data?
	}
	return *this;
}

bool TGAImage::read_tga_file(const char* filename)
{
	if (data)
		delete[] data;
	data = NULL;

	std::ifstream in;
	in.open(filename, std::ios::binary);

	if (!in.is_open())
	{
		std::cerr << "can't open file " << filename << "\n";
		in.close();
		return false;
	}

	TGA_Header header;
	in.read((char*)&header, sizeof(header));

	if (!in.good())
	{
		in.close();
		std::cerr << "an error occured while reading the header\n";
		return false;
	}

	width   = header.width;
	height  = header.height;
	bytespp = header.bits_perpixel >> 3;

	if (width <= 0 || height <= 0 || (bytespp != GRAYSCALE && bytespp != RGB && bytespp != RGBA))
	{
		in.close();
		std::cerr << "bad bpp (or width / height) value\n";
		return false;
	}

	unsigned long nbytes = bytespp * width * height;
	data = new unsigned char[nbytes];

	if (3 == header.datatype_code || 2 == header.datatype_code)
	{
		in.read((char*)data, nbytes);
		//这个nbytes是什么啊？
		if (!in.good())
		{
			in.close();
			std::cerr << "an error occured while reading the data\n";
			return false;
		}
	}
	else if (10 == header.datatype_code || 11 == header.datatype_code)
	{
		if (!load_rle_data(in))
		{
			//load_rle_data 中的 rle 代表的是什么？
			in.close();
			std::cerr << "an occured while reading the data\n";
			return false;
		}
	}
	else
	{
		in.close();
		std::cerr << "unknown file format " << (int)header.datatype_code << "\n";
		return false;
	}

	if (!(header.image_descriptor & 0x20))
	{
		//0x20是什么意思？
		flip_vertically();
	}

	if (header.image_descriptor & 0x10)
	{
		flip_horizontally();
	}

	std::cerr << width << "x" << height << "/" << bytespp * 8 << "\n";
	in.close();
	return true;
}

bool TGAImage::load_rle_data(std::ifstream& in)
{
	unsigned long pixelcount   = width * height;
	unsigned long currentpixel = 0;
	unsigned long currentbyte  = 0;
	
	TGAColor colorbuffer;

	do
	{
		unsigned char chunkheader = 0;
		chunkheader = in.get();

		if (!in.good())
		{
			std::cerr << "an error occured while reading the data\n";
			return false;
		}

		if (chunkheader < 128)
		{
			chunkheader++;
			for (int i = 0; i < chunkheader; i++)
			{
				in.read((char*)colorbuffer.bgra, bytespp);
				if (!in.good())
				{
					std::cerr << "an error occured while reading the header\n";
					return false;
				}

				for (int t = 0; t < bytespp; t++)
					data[currentbyte++] = colorbuffer.bgra[t];
				currentpixel++;

				if (currentpixel > pixelcount)
				{
					std::cerr << "Too many pixels read\n";
					return false;
				}
			}
		}
		else
		{
			chunkheader -= 127;
			in.read((char*)colorbuffer.bgra, bytespp);
			
			if (!in.good())
			{
				std::cerr << "an error occured while reading the header\n";
				return false;
			}

			for (int i = 0; i < chunkheader; i++)
			{
				for (int t = 0; t < bytespp; t++)
				{
					data[currentbyte++] = colorbuffer.bgra[t];
				}
				currentpixel++;

				if (currentpixel > pixelcount)
				{
					std::cerr << "Too many pixels read\n";
					return false;
				}
			}
		}
	}

	while (currentpixel < pixelcount);
	//这个while没有循环体是什么意思？

	return true;
}

bool TGAImage::write_tga_file(const char* filename, bool rle)//整体就是返回个bool变量
{
	unsigned char developer_area_ref[4] = { 0, 0, 0, 0 };
	unsigned char extension_area_ref[4] = { 0, 0, 0, 0 };
	unsigned char footer[18] = { 'T', 'R', 'U', 'E', 'V', 'I', 'S', 'I', 'O', 'N', '-','X','F','I','L','E','.', '\0' };

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
	//(void*)&header这个操作很秀了

	header.bits_perpixel = bytespp << 3;
	header.width  = width;
	//话说这个header具体是什么？
	header.height = height;
	header.datatype_code = (bytespp == GRAYSCALE ? (rle ? 11 : 3) : (rle ? 10 : 2));
	header.image_descriptor = 0x20; //左上角原点
	
	out.write((char*)&header, sizeof(header));

	if (!out.good())
	{
		out.close();
		std::cerr << "can't dump the tga file\n"; //不能转储tga文件
		return false;
	}

	if (!rle)
	{
		out.write((char*)data, width * height * bytespp);
		if (!out.good())
		{
			std::cerr << "can't unload raw data\n"; //无法卸载原始数据
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
// good()表示文件流是否正常，eof表示文件流是否到结束了

/////////////////////////////TODO:不需要为两个相等的像素(对于结果大小而言)打破一个原始块////////////////////////////////////////
bool TGAImage::unload_rle_data(std::ofstream& out)
{
	const unsigned char max_chunk_length = 128;
	unsigned long npixels = width * height;
	// 像素是 w * h
	unsigned long curpix = 0;
	// curpix = current pixel
	
	while (curpix < npixels)
	{
		unsigned long chunkstart = curpix * bytespp;
		unsigned long curbyte = curpix * bytespp;
		//byte是干嘛的？
		unsigned char run_length = 1;
		//游程长度

		bool raw = true;
		while (curpix + run_length < npixels && run_length < max_chunk_length)
		{
			//这里对应哪里的概念？
			bool succ_eq = true;
			//succ_eq表示的是啥
			
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

bool TGAImage::set(int x, int y, TGAColor& c)
{
	if (!data || x < 0 || y < 0 || x >= width || y >= height)
		return false;

	memcpy(data + (x + y * width) * bytespp, c.bgra, bytespp);
	return true;
}

bool TGAImage::set(int x, int y, const TGAColor& c)
// c是color
{
	if (!data || x < 0 || y < 0 || x >= width || y >= height)
		return false;
	memcpy(data + (x + y * width) * bytespp, c.bgra, bytespp);
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
	unsigned char* line = new unsigned char[bytes_per_line];

	int half = height >> 1;
	for (int j = 0; j < half; j++)
	{
		unsigned long l1 = j * bytes_per_line;
		unsigned long l2 = (height - 1 - j) * bytes_per_line;

		memmove((void*)line, (void*)(data + l1), bytes_per_line);
		memmove((void*)(data + l1), (void*)(data + l2), bytes_per_line);
		memmove((void*)(data + l2), (void*)line, bytes_per_line);
	}

	delete[] line;
	return true;
}

unsigned char* TGAImage::buffer()
{
	//buffer竟然是char *型的
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
	//...

	int nscanline = 0;
	int oscanline = 0;
	int erry = 0;

	unsigned long nlinebytes = w * bytespp;
	unsigned long olinebytes = width * bytespp;
	
	for (int j = 0; j < height; j++)
	{
		int errx = width - w;
		int nx = -bytespp;
		int ox = -bytespp;

		for (int i = 0; i < width; i++)
		{
			ox += bytespp;
			//ox是什么鬼
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
			if (erry >= (int)height << 1)
				//这意味着我们跳过了一条扫描线
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
