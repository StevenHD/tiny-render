// Lesson 1-7 中添加了“线框渲染”后
// 才有了"Model0.h"这个文件

#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry3-2.h"

class Model 
{

private:
	std::vector<Vec3f> verts_;
	std::vector<std::vector<int> > faces_;  // 这个faces是无数张脸，faces这个vector存储的就是每一张face
											// 每一张face我的理解是每一个triangle

public:
	Model(const char* filename);
	~Model();

	int nverts();
	int nfaces();

	Vec3f vert(int i);

	std::vector<int> face(int idx);
};

#endif //__MODEL_H__