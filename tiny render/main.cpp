/*
	渲染器最终的呈现：切线空间法线贴图、镜面贴图、阴影贴图

	第一步:	1. 定义模型Model指针为NULL
			2. 定义窗口大小
			3. 定义的光照方向light_dir、观察者眼睛eye、场景中心center、自定义的上向量up

	第二步:	1. 定义深度着色器DepthShader和着色器Shader
			2. 定义着色器中的VS和FS

	第三步:	1. main()函数，定义深度缓冲z-buffer和阴影缓冲shadow-buffer
			2. 渲染阴影缓冲shadowbuffer
			3. 渲染帧缓冲(z-buffer)

	第四步:	1. 清理内存
			2. 返回结果
*/


////////////////////////添加库文件////////////////////////////
#include <vector>
#include <limits>
#include <iostream>
#include <algorithm>

////////////////////////添加头文件////////////////////////////
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl.h"


////////////////////////第一步：定义参数////////////////////////////
Model* model = NULL;
float* shadowbuffer = NULL;

const int width = 800;
const int height = 800;

// 4个向量均是3维浮点型
Vec3f light_dir(1, 1, 0);
Vec3f center(0, 0, 0);
Vec3f eye(1, 1, 4);
Vec3f up(0, 1, 0);


////////////////////////第二步：定义着色器////////////////////////////
struct Shader : public IShader
{
	/////////////////定义转换矩阵和顶点坐标数据/////////////////////////

	// 定义3个4维转换矩阵(float)——转换顶点坐标，从世界空间到裁剪空间
	mat<4, 4, float> uniform_M;

	// 转换法线向量，从世界空间到裁剪空间
	mat<4, 4, float> uniform_MIT;

	// 转换帧缓冲区，将帧缓冲区屏幕坐标转换为阴影缓冲区屏幕坐标 
	mat<4, 4, float> uniform_M_Shadow;

	// 定义三角形的顶点坐标和纹理坐标，坐标是输入进顶点着色器的数据
	// 纹理坐标是2维3列矩阵形式
	mat<2, 3, float> varying_uv;

	// 顶点坐标是3维3列矩阵形式
	mat<3, 3, float> varying_tri_ver;  // 该三角形顶点坐标，不是最终的屏幕空间坐标


	///////////////////定义着色器/////////////////////////

	// 构造函数，生成着色器对象
	Shader(Matrix M, Matrix MIT, Matrix MShadow) : uniform_M(M), uniform_MIT(MIT), uniform_M_Shadow(MShadow),
													varying_uv(), varying_tri_ver() {}
	
	// 顶点着色器，虚函数
	// 将顶点坐标从局部空间，转换到，屏幕空间
	virtual Vec4f vertex(int iface, int nthvert)
	{
		Vec4f obj_ver	 = embed<4>(model->vert(iface, nthvert));
		Vec4f screen_ver = Viewport * Projection * ModelView * obj_ver;

		// 加载每个顶点的纹理坐标uv值
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));

		// 加载投影后的顶点值
		varying_tri_ver.set_col(nthvert, proj<3>(screen_ver / screen_ver[3]));

		// 返回变换后的顶点坐标，传给片段着色器FS
		return screen_ver;
	}

	// 片段着色器，虚函数
	// 片段着色器得到每一个像素的最终颜色值
	virtual bool fragment(Vec3f bc, TGAColor& color)
	{
		// 得到Shadowbuffer中存储的顶点值
		// uniform_M_Shadow是将顶点坐标从帧缓冲转换到阴影缓冲，使用重心坐标bc对顶点进行插值

		Vec4f shadowBuffer_pts = uniform_M_Shadow * embed<4>(varying_tri_ver * bc);
		shadowBuffer_pts = shadowBuffer_pts / shadowBuffer_pts[3];

		// 定义shadowbuffer数组中的索引
		int idx = int(shadowBuffer_pts[0]) + int(shadowBuffer_pts[1]) * width; // 2维转1维

		// 阴影贴图中会出现“z-fighting”，深度冲突
		// 增加一个“Magic Coefficient”，从而避免z-fighting
		float shadow = .3 + .7 * (shadowbuffer[idx] < shadowBuffer_pts[2] + 43.34);

		// varying_uv是2×3的矩阵，通过与重心坐标bc插值，得到当前像素的uv坐标向量
		Vec2f uv = varying_uv * bc;

		// 定义法线向量
		Vec3f normal = model->normal(uv);
		// 将法线向量转换，并归一化后
		Vec3f n = proj<3>(uniform_MIT * embed<4>(normal)).normalize();

		// 转换光照方向light_dir
		Vec3f lgd = proj<3>(uniform_M * embed<4>(light_dir)).normalize();

		// 定义并计算反射光 reflected light
		Vec3f re = (n * (n * lgd * 2.f) - lgd).normalize();

		// 镜面反射，参数是uv，因为纹理的加入也是贴图，将镜面贴图信息存储在uv中
		// 因为镜子会把光反射到无数个方向，所以使用幂pow来模拟纹理贴图和镜面反射的表现
		float spec = pow(std::max(re.z, 0.0f), model->specular(uv));

		// 漫反射是光照强度的余弦值cos(点乘)
		float diff = std::max(0.f, n * lgd);

		// Phong shading
		TGAColor color_Mapping = model->diffuse(uv);

		// 通过 Phone Shading 的公式
		// 得到每一个像素最终的r g b值
		for (int i = 0; i < 3; i++)
		{
			color[i] = std::min<float>(20 + color_Mapping[i] * shadow * (1.2 * diff + .6 * spec), 255);
		}

		return false;
	}
};


////////////////////////第二步：定义深度着色器////////////////////////////
struct DepthShader : public IShader
{
	// 定义三角形的顶点坐标，矩阵形式
	mat<3, 3, float> varying_tri;

	// 构造函数初始化列表
	DepthShader() : varying_tri() {}

	// 顶点着色器
	virtual Vec4f vertex(int iface, int nthvert)
	{
		// 从.obj文件中读取顶点坐标数据
		Vec4f obj_ver_coord = embed<4>(model->vert(iface, nthvert));

		// 将读取到的顶点数据转换为屏幕空间坐标
		Vec4f screen_ver_coord = Viewport * Projection * ModelView * obj_ver_coord;

		// 对varying_tri矩阵中的每一个顶点类型(v/vt/vn)，均进行设置列来更新数值
		// 将4D对象投影回3D，x y z分量均除以w分量(透视投影)
		varying_tri.set_col(nthvert, proj<3>(screen_ver_coord / screen_ver_coord[3]));

		return screen_ver_coord;
	}

	virtual bool fragment(Vec3f bc, TGAColor& color)
	{
		// varying_tri就是顶点着色器传给片段着色器的顶点数据
		Vec3f depth_tri = varying_tri * bc;

		// 深度着色器DepthShader只需要考虑z值
		color = TGAColor(255, 255, 255) * (depth_tri.z / depth);

		return false;
	}
};



////////////////////////第三步：main()函数////////////////////////////

int main(int argc, char** argv)
{
	/////////////////////处理2个缓冲区////////////////////////

	// 定义帧缓冲，缓冲区大小为窗口大小
	float* zbuffer = new float[width * height];

	// 给全局变量shadowbuffer开辟缓冲区的空间大小
	shadowbuffer = new float[width * height];

	// 初始化zbuffer和shadowbuffer中每一个像素对应区域的初值，初值是float类型的负无穷大
	for (int i = 0; i < width * height; i++)
	{
		zbuffer[i] = shadowbuffer[i] = -std::numeric_limits<float>::max();
	}


	/////////////////////处理模型/////////////////////
	model = new Model("obj/african_head.obj");  // ("obj/dialo_pose3.obj")

	light_dir.normalize();

	{
		/////////////渲染深度缓冲区///////////////
		TGAImage DepthImage(width, height, TGAImage::RGB);


		lookat(light_dir, center, up); 	// 在depthShader中，light_dir光照方向就是“人眼方向eye”
		viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
		projection(0); 					// 也不进行透视投影，系数为0

		DepthShader depth_Shader;
		Vec4f screen_coords[3];			// 顶点数据转换为屏幕空间坐标

		// 枚举每一个face面元
		for (int i = 0; i < model->nfaces(); i++)
		{
			// 枚举每一个小三角形中的每一个顶点(共3个顶点)
			for (int j = 0; j < 3; j++)
			{
				// 调用顶点着色器vertex shader
				screen_coords[j] = depth_Shader.vertex(i, j);
			}

			// 每一个face(面元)对应一个三角形
			// 每处理完一个三角形中的3个顶点的值，调用光栅格化器triangle()
			triangle(screen_coords, depth_Shader, DepthImage, shadowbuffer);
		}

		DepthImage.flip_vertically(); // 图像的起点是左上角，这里将其变到左下角
		DepthImage.write_tga_file("Final render image.tga");
	}

	Matrix M_Local2Screen = Viewport * Projection * ModelView;

	{
		/////////////渲染帧缓冲区///////////////
		TGAImage finalFrame(width, height, TGAImage::RGB);

		Vec4f screen_coords[3];

		lookat(eye, center, up); // 这里的观察者不再是光照方向，而是定义的摄影机位置eye
		viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
		projection(-1.f / (eye - center).norm());  // 系数变为(-1/方向向量) 

		// 第3个参数的作用是将framebuffer帧缓冲区屏幕空间转换为shadowbuffer阴影缓冲区屏幕空间的矩阵
		Shader finalShader(ModelView, (Projection * ModelView).invert_transpose(),
			M_Local2Screen * (Viewport * Projection * ModelView).invert());

		for (int i = 0; i < model->nfaces(); i++)
		{
			for (int j = 0; j < 3; j++)
			{
				screen_coords[j] = finalShader.vertex(i, j);
			}

			triangle(screen_coords, finalShader, finalFrame, zbuffer);
		}

		finalFrame.flip_vertically();
		finalFrame.write_tga_file("Final frame buffer.tga");
	}

	////////////////////////第四步：清理内存、返回结果////////////////////////
	delete model;

	// 注意：缓冲区均是数组形式
	delete [] zbuffer;
	delete [] shadowbuffer;

	return 0;
}


/*
	好，到这里，带有阴影贴图、镜面贴图、法线贴图、纹理贴图的渲染就生成了。
	总结一下这次的失误：viewport()函数中后两个参数第一次写成了width,height，生成的image
	都是黑色的。改为(width * 3 / 4)和(width * 3 / 4)后就好了
*/