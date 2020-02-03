#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include <string>
#include "geometry.h"
#include "tgaimage.h"

class Model 
{
private:
	std::vector<Vec3f> verts_;
	std::vector<std::vector<Vec3i> > faces_; //Vec3i means that vertex、uv、normal
											 //So, Vec3f是哪3个部分?
	//大家注意一点：<std::vector<Vec3i> >最后的呢个空格是必不可少的。
	//因为typename也是一个STL容器，也就是这里的vector。
	//之所以定义的时候要在>>符号之间加上空格，因为一些使用C++11之间的标准的编译器会把它是为移位操作
	std::vector<Vec3f> norms_;
	std::vector<Vec2f> uv_;
	
	TGAImage diffusemap_;
	TGAImage normalmap_;
	TGAImage specularmap_;

	void load_texture(std::string filename, const char* suffix, TGAImage& img);

public:
	Model(const char* filename);
	~Model();

	int nverts();
	int nfaces();

	Vec3f normal(int ith_face, int nth_vert); //normal表示的是法线
	Vec3f normal(Vec2f uv);

	Vec3f vert(int i);
	Vec3f vert(int ith_face, int nth_vert);

	Vec2f uv(int ith_face, int nth_vert);

	TGAColor diffuse(Vec2f uv); //这个diffuse是哪个章节的内容？
	float specular(Vec2f uv);   //这个specular是哪个章节的内容？

	std::vector<int> face(int idx); //这个c代表的是脸的哪些部分？idx我记得是索引的表示
};



#endif // !__MODEL_H__
