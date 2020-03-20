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
	mat<4, 4, float> uniform_M;   //  Projection*ModelView ������ռ�ת��Ϊ�ü��ռ�ľ���
	mat<4, 4, float> uniform_MIT; // �����������ת����������

	mat<4, 4, float> uniform_Mshadow; // transform framebuffer screen coordinates 
									  // to shadowbuffer screen coordinates
									  // ��֡��������Ļ����ת��Ϊ��Ӱ��������Ļ���� 
	
	mat<2, 3, float> varying_uv;  // ��������������, written by the VS
								  // read by the FS

	mat<3, 3, float> varying_tri; // triangle coordinates before Viewport transform 
								  // ViewPort�任֮ǰ�������ζ������꣬��������Ļ����
								  // written by VS, read by FS

	Shader(Matrix M, Matrix MIT, Matrix MS) : uniform_M(M), uniform_MIT(MIT), 
								uniform_Mshadow(MS), varying_uv(), varying_tri() {}

	virtual Vec4f vertex(int iface, int nthvert)
	{
		Vec4f object_ver = embed<4>(model->vert(iface, nthvert));
		Vec4f screen_ver = Viewport * Projection * ModelView * object_ver;

		// ���ض������������uvֵ
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		
		// �������д���ĸ��ģ����ñ����Ƚ�������tga�ļ��䰵
		varying_tri.set_col(nthvert, proj<3>(screen_ver / screen_ver[3]));
		/*varying_tri.set_col(nthvert, model->vert(iface, nthvert));*/

		return screen_ver;
	}

	virtual bool fragment(Vec3f bar, TGAColor& color)
	{
		// corresponding point in the shadow buffer
		Vec4f shadowbuffer_pt = uniform_Mshadow * embed<4>(varying_tri * bar);
		shadowbuffer_pt = shadowbuffer_pt / shadowbuffer_pt[3];  // sb��shadow buffer

		int idx = int(shadowbuffer_pt[0]) + int(shadowbuffer_pt[1]) * width; // index in the shadowbuffer array

		// magic coeff to avoid z-fighting
		// ������ȳ�ͻ
		float shadow = .3 + .7 * (shadowbuffer[idx] < shadowbuffer_pt[2] + 43.34); 
		
		// varying_uv����2��3�ľ���ͨ����bc��ֵ���õ���ǰ���ص�uv��������
		Vec2f uv = varying_uv * bar;                 // interpolate uv for the current pixel

		// �ȵõ���������
		Vec3f normal = model->normal(uv);
		// �õ�����ת��������һ����ķ�������
		Vec3f n = proj<3>(uniform_MIT * embed<4>(normal)).normalize(); 

		// ת�����շ���light_dir
		Vec3f l = proj<3>(uniform_M * embed<4>(light_dir)).normalize(); // light vector

		// ���岢���㷴��� reflected light
		Vec3f r = (n * (n * l * 2.f) - l).normalize();
		
		// ���淴��
		// ��Ϊ���ӻ�ѹⷴ�䵽������������������ʹ����pow��ģ��������ͼ�;��淴��ı���
		float spec = pow(std::max(r.z, 0.0f), model->specular(uv));

		// ���������ǹ���ǿ�ȵ�����ֵ
		float diff = std::max(0.f, n * l);
		
		// Phong shading
		// c��ʾ��ͼ��ɫ
		TGAColor c = model->diffuse(uv);

		// ͨ��Phone Shading �Ĺ�ʽ
		// �õ����յ�Phong��ɫ
		for (int i = 0; i < 3; i++) 
			color[i] = std::min<float>(20 + c[i] * shadow * (1.2 * diff + .6 * spec), 255);

		return false;
	}
};

struct DepthShader : public IShader
{
	mat<3, 3, float> varying_tri; // �����εĶ�������

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

	// ͬʱ��ʼ��zbuffer��shadowbuffer��ֵ
	// ��ֵ��Ϊfloat�͵ĸ������
	/*for (int i = width * height; --i; )*/
	for (int i = 0; i < width * height; i++) // i����<=w*h�����������Խ������
	{
		zbuffer[i] = shadowbuffer[i] = -std::numeric_limits<float>::max();
	}

	// model = new Model(argv[1]);
	model = new Model("obj/african_head.obj");
	light_dir.normalize();

	{
		// shaderbuffer������Ⱦ��Ӱ������
		TGAImage depth(width, height, TGAImage::RGB);

		// ����depthshader
		DepthShader depthshader;

		Vec4f screen_coords[3];

		// �����lookat����ĵ�һ�������Ĺ��շ���
		lookat(light_dir, center, up);
		viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);

		// ����0��͸��ͶӰ�е�ϵ������ȻΪ0�����Ծ���ζ��û��͸��ͶӰ
		// ��Ϊ���������Ⱦ���depth
		projection(0);

		for (int i = 0; i < model->nfaces(); i++) 
		{
			for (int j = 0; j < 3; j++) 
			{
				screen_coords[j] = depthshader.vertex(i, j);
			}

			// triangle()���������һ����������դ�񻯵Ĺ����У����� z-buffer
			// shaderbuffer��ʵ�Σ�zbuffer����Ȼ��������β�
			// ��ָ�봫�ݹ�ȥ��zbuffer��ֵ���Ƹ�shadowbuffer
			triangle(screen_coords, depthshader, depth, shadowbuffer);
		}

		depth.flip_vertically(); // to place the origin in the bottom left corner of the image
		depth.write_tga_file("depth.tga");
	}

	Matrix World2Screen_M = Viewport * Projection * ModelView; // ����M���ǰ��������������ת������Ļ����

	{
		// rendering the frame buffer
		TGAImage frame(width, height, TGAImage::RGB);

		Vec4f screen_coords[3];

		// ע��lookat�ĵ�һ������������Ĳ�һ��
		// Shader�ǵ�2����Ⱦ��������ƶ�������λ�����¼���Viewport�����ͶӰ����Projection
		// Ȼ�󽫸��º�ı任������Ϊ������������ɫ��Shader
		lookat(eye, center, up);
		viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
		projection(-1.f / (eye - center).norm());

		// ��3�������ǽ�֡��������Ļ�ռ�ת��Ϊshadow��Ļ�ռ�ľ���
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

		frame.flip_vertically(); // ��Ϊһ��ͼƬ��ԭ����ʵ�Ǵ����Ϸ���ʼ�� 
								 // ����������֮ǰ��������·�����Ϊ���Ŵ�ֱ����תһ�¾ͺ�

		frame.write_tga_file("framebuffer-Shader-z-shaderbuffer.tga");
	}


	delete model;

	delete[] zbuffer;
	delete[] shadowbuffer;

	return 0;
}
