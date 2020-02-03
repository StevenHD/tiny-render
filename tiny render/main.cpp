#include <vector>
#include <iostream>

#include <cmath>
#include <algorithm>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

Model* model	 = NULL;

const int width  = 800;		//ͼƬ�Ŀ��
const int height = 800;		//ͼƬ�ĸ߶�


Vec3f light_dir(1, 1, 1);   //���շ���
Vec3f		eye(0, -1, 3);  //�۾����ڵ�λ��
Vec3f	 center(0, 0, 0);	//����
Vec3f		 up(0, 1, 0);	//ViewPort

//////////////////////////////////////////////////////////////////////////////////////////

struct GouraudShader : public IShader
{
	Vec3f varying_intensity; // written by vertex shader, read by fragment shader
							 // �ɶ�����ɫ��д�룬��Ƭ����ɫ����ȡ

	virtual Vec4f vertex(int ith_face, int nth_vert) //����vertex��vec<4, float>��һֱ���������4��ͨ��������˼��
	{
		Vec4f gl_Vertex = embed<4>(model->vert(ith_face, nth_vert)); //read the vertex from .obj file
		gl_Vertex = ViewPort * Projection * ModelView * gl_Vertex;   //transform gl_Vertex to screen coordinates
		varying_intensity[nth_vert] = std::max(0.f, model->normal(ith_face, nth_vert) * light_dir);
		//  get diffuse lighting intensity  ��������ǿ
		return gl_Vertex; //���ض���
	}

	virtual bool fragment(Vec3f bar, TGAColor& color)
	{
		//fragment��lesson 5��û���Ҽ���������������أ�
		//bar������������˼
		float intensity = varying_intensity * bar;    //  ����bar����������Ӧ����ʲô�������
												      //  interpolate intensity for the current pixel
		color = TGAColor(255, 255, 255) * intensity;  //TGAColor(255,255,255)�����Ӧ����white
		return false;								  // return false ����˼�ǲ������������
	}
};

//void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color)
//{
//	bool steep = false;
//}

int main(int argc, char** argv)
{
	if (2 == argc)
		model = new Model(argv[1]);
	else
		model = new Model("obj/african_head.obj");

	lookat(eye, center, up);
	viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
	projection(-1.f / (eye - center).norm());
	light_dir.normalize();

	TGAImage image   (width, height, TGAImage::RGB);
	TGAImage zbuffer (width, height, TGAImage::GRAYSCALE); //zbuffer��Ӧ���ǻҶ�ͼGRAYSCALE
	
	GouraudShader shader;

	for (int i = 0; i < model->nfaces(); i++)
	{
		Vec4f screen_coords[3];
		/*std::vector<int> face = model->face(i);*/

		for (int j = 0; j < 3; j++)
		{
			/*Vec3f v0 = model->vert(face[j]);
			Vec3f v1 = model->vert(face[(j + 1) % 3]);

			int x0 = (v0.x + 1.) * width / 2.;
			int y0 = (v0.y + 1.) * height / 2.;
			int x1 = (v1.x + 1.) * width / 2.;
			int y1 = (v1.y + 1.) * height / 2.;*/

			screen_coords[j] = shader.vertex(i, j);
			/*line(x0, y0, x1, y1, image, white);*/
		}
		triangle(screen_coords, shader, image, zbuffer);
	}

	image.flip_vertically();  // ��ԭ�������ͼ������½�
	zbuffer.flip_vertically();

	image.	write_tga_file("output.tga");
	zbuffer.write_tga_file("zbuffer.tga");
	
	delete model;
	return 0;
}