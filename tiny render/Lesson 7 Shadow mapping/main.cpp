#include <vector>
#include <limits>
#include <iostream>
#include <algorithm>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl.h"

Model* model = NULL;
float* shadowbuffer = NULL;

const int width = 800;
const int height = 800;

Vec3f light_dir(1, 1, 0);
//Vec3f       eye(1, 1, 4);
Vec3f       eye(0, 1, 10);
Vec3f    center(0, 0, 0);
Vec3f        up(0, 1, 0);

struct Shader : public IShader
{
	mat<4, 4, float> uniform_M;   //  Projection*ModelView 将世界空间转换为裁剪空间的矩阵
	mat<4, 4, float> uniform_MIT; // 这个矩阵用来转换法线向量

	mat<4, 4, float> uniform_Mshadow; // transform framebuffer screen coordinates 
									  // to shadowbuffer screen coordinates
									  // 将帧缓冲区屏幕坐标转换为阴影缓冲区屏幕坐标 
	
	mat<2, 3, float> varying_uv;  // 三角形纹理坐标, written by the VS
								  // read by the FS

	mat<3, 3, float> varying_tri; // triangle coordinates before Viewport transform 
								  // ViewPort变换之前的三角形顶点坐标，而不是屏幕坐标
								  // written by VS, read by FS

	Shader(Matrix M, Matrix MIT, Matrix MS) : uniform_M(M), uniform_MIT(MIT), 
								uniform_Mshadow(MS), varying_uv(), varying_tri() {}

	virtual Vec4f vertex(int iface, int nthvert)
	{
		Vec4f object_ver = embed<4>(model->vert(iface, nthvert));
		Vec4f screen_ver = Viewport * Projection * ModelView * object_ver;

		// 加载顶点的纹理坐标uv值
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		
		// 下面两行代码的更改，会让本来比较明亮的tga文件变暗
		varying_tri.set_col(nthvert, proj<3>(screen_ver / screen_ver[3]));
		/*varying_tri.set_col(nthvert, model->vert(iface, nthvert));*/

		return screen_ver;
	}

	virtual bool fragment(Vec3f bar, TGAColor& color)
	{
		// corresponding point in the shadow buffer
		Vec4f shadowbuffer_pt = uniform_Mshadow * embed<4>(varying_tri * bar);
		shadowbuffer_pt = shadowbuffer_pt / shadowbuffer_pt[3];  // sb是shadow buffer

		int idx = int(shadowbuffer_pt[0]) + int(shadowbuffer_pt[1]) * width; // index in the shadowbuffer array

		// magic coeff to avoid z-fighting
		// 避免深度冲突
		float shadow = .3 + .7 * (shadowbuffer[idx] < shadowbuffer_pt[2] + 43.34); 
		
		// varying_uv本是2×3的矩阵，通过和bc插值，得到当前像素的uv坐标向量
		Vec2f uv = varying_uv * bar;                 // interpolate uv for the current pixel

		// 先得到法线向量
		Vec3f normal = model->normal(uv);
		// 得到经过转换，并归一化后的法线向量
		Vec3f n = proj<3>(uniform_MIT * embed<4>(normal)).normalize(); 

		// 转换光照方向light_dir
		Vec3f l = proj<3>(uniform_M * embed<4>(light_dir)).normalize(); // light vector

		// 定义并计算反射光 reflected light
		Vec3f r = (n * (n * l * 2.f) - l).normalize();
		
		// 镜面反射
		// 因为镜子会把光反射到无数个方向，所以我们使用幂pow来模拟纹理贴图和镜面反射的表现
		float spec = pow(std::max(r.z, 0.0f), model->specular(uv));

		// 漫反射则是光照强度的余弦值
		float diff = std::max(0.f, n * l);
		
		// Phong shading
		// c表示贴图颜色
		TGAColor c = model->diffuse(uv);

		// 通过Phone Shading 的公式
		// 得到最终的Phong着色
		for (int i = 0; i < 3; i++) 
			color[i] = std::min<float>(20 + c[i] * shadow * (1.2 * diff + .6 * spec), 255);

		return false;
	}
};

struct DepthShader : public IShader
{
	mat<3, 3, float> varying_tri; // 三角形的顶点坐标

	DepthShader() : varying_tri() {}

	virtual Vec4f vertex(int iface, int nthvert) 
	{
		Vec4f object_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex 
																 // from .obj file
		Vec4f screen_Vertex = Viewport * Projection * ModelView * object_Vertex; // transform it to 
																   // screen coordinates
		
		varying_tri.set_col(nthvert, proj<3>(screen_Vertex / screen_Vertex[3]));

		return screen_Vertex;
	}

	virtual bool fragment(Vec3f bar, TGAColor& color)
	{
		Vec3f p = varying_tri * bar;

		color = TGAColor(255, 255, 255) * (p.z / depth);
		/*color = TGAColor(255, 255, 255) * (p.z);*/
		return false;
	}
};

int main(int argc, char** argv)
{
	/*if (2 > argc) 
	{
		std::cerr << "Usage: " << argv[0] << "obj/model.obj" << std::endl;
		return 1;
	}*/


	float* zbuffer = new float[width * height];

	shadowbuffer = new float[width * height];

	// 同时初始化zbuffer和shadowbuffer的值
	// 初值定为float型的负无穷大
	/*for (int i = width * height; --i; )*/
	for (int i = 0; i < width * height; i++) // i不能<=w*h，会出现数组越界的情况
	{
		zbuffer[i] = shadowbuffer[i] = -std::numeric_limits<float>::max();
	}

	// model = new Model(argv[1]);
	model = new Model("obj/african_head.obj");
	light_dir.normalize();

	{
		// shaderbuffer——渲染阴影缓冲区
		TGAImage depth(width, height, TGAImage::RGB);

		// 声明depthshader
		DepthShader depthshader;

		Vec4f screen_coords[3];

		// 这里的lookat矩阵的第一个参数的光照方向
		lookat(light_dir, center, up);
		viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);

		// 由于0是透视投影中的系数，既然为0，所以就意味着没有透视投影
		// 因为这个是在渲染深度depth
		projection(0);

		for (int i = 0; i < model->nfaces(); i++) 
		{
			for (int j = 0; j < 3; j++) 
			{
				screen_coords[j] = depthshader.vertex(i, j);
			}

			// triangle()函数的最后一个参数，在栅格化的过程中，就是 z-buffer
			// shaderbuffer，实参；zbuffer是深度缓冲区，形参
			// 靠指针传递过去，zbuffer的值复制给shadowbuffer
			triangle(screen_coords, depthshader, depth, shadowbuffer);
		}

		depth.flip_vertically(); // to place the origin in the bottom left corner of the image
		depth.write_tga_file("depth.tga");
	}

	Matrix World2Screen_M = Viewport * Projection * ModelView; // 矩阵M就是把坐标从世界坐标转换成屏幕坐标

	{
		// rendering the frame buffer
		TGAImage frame(width, height, TGAImage::RGB);

		Vec4f screen_coords[3];

		// 注意lookat的第一个参数和上面的不一样
		// Shader是第2次渲染，将相机移动到法线位置重新计算Viewport矩阵和投影矩阵Projection
		// 然后将更新后的变换矩阵作为参数，调用着色器Shader
		lookat(eye, center, up);
		viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
		projection(-1.f / (eye - center).norm());

		// 第3个参数是将帧缓冲区屏幕空间转换为shadow屏幕空间的矩阵
		Shader shader(ModelView, (Projection * ModelView).invert_transpose(), 
			World2Screen_M * (Viewport * Projection * ModelView).invert());

		
		for (int i = 0; i < model->nfaces(); i++) 
		{
			for (int j = 0; j < 3; j++)
			{
				screen_coords[j] = shader.vertex(i, j);
			}

			triangle(screen_coords, shader, frame, zbuffer);
		}

		frame.flip_vertically(); // 因为一张图片的原点其实是从左上方开始的 
								 // 并不是我们之前所想的左下方，因为沿着垂直方向反转一下就好

		frame.write_tga_file("framebuffer-Shader-z-shaderbuffer.tga");
	}


	delete model;

	delete[] zbuffer;
	delete[] shadowbuffer;

	return 0;
}
