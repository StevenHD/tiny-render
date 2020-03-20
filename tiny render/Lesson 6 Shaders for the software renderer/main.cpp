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
//	virtual bool fragmentFirst(Vec3f bar, TGAColor& color) // bar����������
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
	mat<4, 4, float> uniform_M;   //  Projection*ModelView��������ռ䵽�ü��ռ�
	mat<4, 4, float> uniform_MIT; // (Projection*ModelView).invert_transpose()

	virtual Vec4f vertexThird(int iface, int nthvert)
	{
		varying_uv.set_col(nthvert, model->uv(iface, nthvert)); // ��ȡmodel������������Ϣ

		// ��ȡmodel�ļ��з�������������Ϣ���õ�����ǿ��
		varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert) * light_dir);

		// ��ȡmodel�еĶ���������Ϣ����Ƕ��4ά�����С�
		Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));

		// ������ɫ�����ص�����Ļ�ռ�ģ��Ҳ�������ֵ��������ô��4ά����Vec4f��
		return Viewport * Projection * ModelView * gl_Vertex;
	}

	virtual bool fragmentThird(Vec3f bar, TGAColor& color)
	{

		float intensity = varying_intensity * bar;
		Vec2f uv = varying_uv * bar;

		// ģ�͵���ɫcolor��ֵ��ͨ��diffuse������ȡ�ģ�diffuse������ȡ����diffusemap_�е���Ϣ
		// diffusemap_�е���Ϣ����_diffuse.tga�е���Ϣ��
		// �õ���diffuse(uv)����Ϣ���;���������������˺�Ĺ�ǿ��˺󣬵õ���color
		// diffuse����������յ���˼�������Ҵ�diffuse.tga�ļ�����ʾ���Ƿ�����ͼ����Ϣ
		// ֻ����ɫ���������ɫ
		color = model->diffuse(uv) * intensity;
		return false;
	}

	virtual Vec4f vertexNormalMapping(int iface, int nthvert)
	{
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file

		return Viewport * Projection * ModelView * gl_Vertex; // transform it to screen coordinates
															  // ����Ϊʲô����һ��Vec4f����
	}

	virtual bool fragmentNormalMapping(Vec3f bar, TGAColor& color)
	{
		Vec2f uv = varying_uv * bar;

		// uv����ͷ�����ͼ��ˣ��õ��µ�uv���꣬Ȼ��õ��µ���ɫ���ٵõ��µ�normal
		// uniform_MIT��ʲô��embed?Ȼ��ͨ��proj�õ��µľ����µľ�������µ�normal
		Vec3f n = proj<3>(uniform_MIT * embed<4>(model->normal(uv))).normalize();

		// Ϊʲô���ڹ��շ���light_dirҪ��uniform_M���ˣ�
		// �о�light_dir����Lesson 5�����е�(x, y, z)
		// ���������ӵĻ� n * l ������0��ô��
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
			// ����㷨��ʲô������ģ�ΪʲôҪ��255�Ƚϣ�
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

	// ��ͼ����
	viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);

	// ͸��
	projection(-1.f / (eye - center).norm());

	light_dir.normalize();

	TGAImage image(width, height, TGAImage::RGB);
	TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

	// GouraudShader shader;

	Shader shader;

	// ͨ��uniform_M���󣬶Զ������ת��
	shader.uniform_M = Projection * ModelView;

	// ͨ��uniform_MIT���󣬶Է��߽���ת��
	shader.uniform_MIT = (Projection * ModelView).invert_transpose();

	// ö��i��ÿһ����Ԫ
	for (int i = 0; i < model->nfaces(); i++) 
	{
		Vec4f screen_coords[3];

		// ö��j�Ǹ���Ԫ�ڵ�3������(�������꣬�������꣬���㷨��)
		for (int j = 0; j < 3; j++)
		{
			screen_coords[j] = shader.vertex(i, j); // ���ö�����ɫ����ת���������������ռ䵽��Ļ�ռ�
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