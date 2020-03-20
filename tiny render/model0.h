// Lesson 1-7 ������ˡ��߿���Ⱦ����
// ������"Model0.h"����ļ�

#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry3-2.h"

class Model 
{

private:
	std::vector<Vec3f> verts_;
	std::vector<std::vector<int> > faces_;  // ���faces������������faces���vector�洢�ľ���ÿһ��face
											// ÿһ��face�ҵ������ÿһ��triangle

public:
	Model(const char* filename);
	~Model();

	int nverts();
	int nfaces();

	Vec3f vert(int i);

	std::vector<int> face(int idx);
};

#endif //__MODEL_H__