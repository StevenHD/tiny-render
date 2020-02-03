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
	std::vector<std::vector<Vec3i> > faces_; //Vec3i means that vertex��uv��normal
											 //So, Vec3f����3������?
	//���ע��һ�㣺<std::vector<Vec3i> >�����ظ��ո��Ǳز����ٵġ�
	//��ΪtypenameҲ��һ��STL������Ҳ���������vector��
	//֮���Զ����ʱ��Ҫ��>>����֮����Ͽո���ΪһЩʹ��C++11֮��ı�׼�ı������������Ϊ��λ����
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

	Vec3f normal(int ith_face, int nth_vert); //normal��ʾ���Ƿ���
	Vec3f normal(Vec2f uv);

	Vec3f vert(int i);
	Vec3f vert(int ith_face, int nth_vert);

	Vec2f uv(int ith_face, int nth_vert);

	TGAColor diffuse(Vec2f uv); //���diffuse���ĸ��½ڵ����ݣ�
	float specular(Vec2f uv);   //���specular���ĸ��½ڵ����ݣ�

	std::vector<int> face(int idx); //���c�������������Щ���֣�idx�Ҽǵ��������ı�ʾ
};



#endif // !__MODEL_H__
