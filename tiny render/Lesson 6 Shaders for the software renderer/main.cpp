#include <vector>
#include <iostream>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl.h"

Model* model = NULL;
const int width = 800;
const int height = 800;

Vec3f light_dir(1, 1, 1);
// Vec3f eye(1, 1, 3);
Vec3f eye(0, 0, 3);
Vec3f    center(0, 0, 0);
Vec3f        up(0, 1, 0);

//struct GouraudShader : public IShader
//{
//	Vec3f varying_intensity; // written by vertex shader, read by fragment shader
//
//	virtual Vec4f vertexFirst(int iface, int nthvert) 
//	{
//		Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
//		
//		gl_Vertex = Viewport * Projection * 
//						ModelView * gl_Vertex;     // transform it to screen coordinates
//
//		varying_intensity[nthvert] = std::max(0.f, 
//				model->normal(iface, nthvert) * light_dir); // get diffuse lighting intensity
//
//		return gl_Vertex;
//	}
//
//
//	virtual bool fragmentFirst(Vec3f bar, TGAColor& color) // bar是重心坐标
//	{
//		float intensity = varying_intensity * bar;   // interpolate intensity for the current pixel
//		color = TGAColor(255, 255, 255) * intensity; // well duh
//		return false;                              // no, we do not discard this pixel
//	}
//
//	virtual bool fragmentSecond(Vec3f bar, TGAColor& color)
//	{
//		float intensity = varying_intensity * bar;
//		if (intensity > .85) intensity = 1;
//		else if (intensity > .60) intensity = .80;
//		else if (intensity > .45) intensity = .60;
//		else if (intensity > .30) intensity = .45;
//		else if (intensity > .15) intensity = .30;
//		else intensity = .0;
//
//		color = TGAColor(255, 155, 0) * intensity;
//		return false;
//	}
//};

struct Shader : public IShader
{
	Vec3f varying_intensity;
	mat<2, 3, float> varying_uv;
	mat<4, 4, float> uniform_M;   //  Projection*ModelView，从世界空间到裁剪空间
	mat<4, 4, float> uniform_MIT; // (Projection*ModelView).invert_transpose()

	virtual Vec4f vertexThird(int iface, int nthvert)
	{
		varying_uv.set_col(nthvert, model->uv(iface, nthvert)); // 读取model中纹理坐标信息

		// 读取model文件中法线向量坐标信息，得到光照强度
		varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert) * light_dir);

		// 读取model中的顶点坐标信息，内嵌到4维向量中。
		Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));

		// 顶点着色器返回的是屏幕空间的，我猜是坐标值，不过怎么是4维向量Vec4f？
		return Viewport * Projection * ModelView * gl_Vertex;
	}

	virtual bool fragmentThird(Vec3f bar, TGAColor& color)
	{

		float intensity = varying_intensity * bar;
		Vec2f uv = varying_uv * bar;

		// 模型的颜色color的值是通过diffuse函数读取的，diffuse函数读取的是diffusemap_中的信息
		// diffusemap_中的信息则是_diffuse.tga中的信息，
		// 得到的diffuse(uv)的信息，和经过与重心坐标相乘后的光强相乘后，得到了color
		// diffuse是漫反射光照的意思，但是我打开diffuse.tga文件后，显示的是法线贴图的信息
		// 只是颜色是纹理的颜色
		color = model->diffuse(uv) * intensity;
		return false;
	}

	virtual Vec4f vertexNormalMapping(int iface, int nthvert)
	{
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file

		return Viewport * Projection * ModelView * gl_Vertex; // transform it to screen coordinates
															  // 但是为什么返回一个Vec4f变量
	}

	virtual bool fragmentNormalMapping(Vec3f bar, TGAColor& color)
	{
		Vec2f uv = varying_uv * bar;

		// uv坐标和法线贴图相乘，得到新的uv坐标，然后得到新的颜色，再得到新的normal
		// uniform_MIT是什么？embed?然后通过proj得到新的矩阵，新的矩阵就是新的normal
		Vec3f n = proj<3>(uniform_MIT * embed<4>(model->normal(uv))).normalize();

		// 为什么对于光照方向light_dir要用uniform_M来乘？
		// 感觉light_dir就是Lesson 5理论中的(x, y, z)
		// 不过这样子的话 n * l 不就是0了么？
		Vec3f l = proj<3>(uniform_M * embed<4>(light_dir)).normalize();

		float intensity = std::max(0.f, n * l);
		color = model->diffuse(uv) * intensity;

		return false;
	}

	virtual Vec4f vertex(int iface, int nthvert) // SpecularMapping
	{
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
		return Viewport * Projection * ModelView * gl_Vertex; // transform it to screen coordinates
	}

	virtual bool fragment(Vec3f bar, TGAColor& color) // SpecularMapping
	{
		Vec2f uv = varying_uv * bar;

		Vec3f n = proj<3>(uniform_MIT * embed<4>(model->normal(uv))).normalize();

		Vec3f l = proj<3>(uniform_M * embed<4>(light_dir)).normalize();

		Vec3f r = (n * (n * l * 2.f) - l).normalize();   // reflected light

		float spec = pow(std::max(r.z, 0.0f), model->specular(uv));
		float diff = std::max(0.f, n * l);

		TGAColor c = model->diffuse(uv);
		color = c;

		for (int i = 0; i < 3; i++) 
			// 这个算法是什么？具体的，为什么要和255比较？
			color[i] = std::min<float>(5 + c[i] * (diff + .6 * spec), 255);

		return false;
	}
};


int main(int argc, char** argv) 
{
	if (2 == argc) 
	{
		model = new Model(argv[1]);
	}
	else 
	{
		// model = new Model("obj/diablo3_pose.obj");
		model = new Model("obj/diablo3_pose.obj");
	}

	lookat(eye, center, up);

	// 视图窗口
	viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);

	// 透视
	projection(-1.f / (eye - center).norm());

	light_dir.normalize();

	TGAImage image(width, height, TGAImage::RGB);
	TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

	// GouraudShader shader;

	Shader shader;

	// 通过uniform_M矩阵，对顶点进行转换
	shader.uniform_M = Projection * ModelView;

	// 通过uniform_MIT矩阵，对法线进行转换
	shader.uniform_MIT = (Projection * ModelView).invert_transpose();

	// 枚举i是每一张面元
	for (int i = 0; i < model->nfaces(); i++) 
	{
		Vec4f screen_coords[3];

		// 枚举j是该面元内的3个顶点(顶点坐标，纹理坐标，顶点法线)
		for (int j = 0; j < 3; j++)
		{
			screen_coords[j] = shader.vertex(i, j); // 调用顶点着色器，转换顶点坐标从世界空间到屏幕空间
		}
		triangle(screen_coords, shader, image, zbuffer);
	}

	image.flip_vertically(); // to place the origin in the bottom left corner of the image
	zbuffer.flip_vertically();

	image.write_tga_file("Shaders-diablo3_pose.tga");
	zbuffer.write_tga_file("zbuffer-Shaders-diablo3_pose.tga");

	delete model;
	return 0;
}