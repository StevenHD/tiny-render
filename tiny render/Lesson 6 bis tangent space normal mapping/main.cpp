#include <vector>
#include <limits>
#include <iostream>
#include <algorithm>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl.h"

Model* model = NULL;

const int width = 800;
const int height = 800;

Vec3f light_dir(1, 1, 0);
//Vec3f       eye(1, 1, 4);
Vec3f       eye(0, 1, 10);
Vec3f    center(0, 0, 0);
Vec3f        up(0, 1, 0);

// It is Phong shading
struct Shader : public IShader
{
	/*	geometry.h
	template<size_t DimRows, size_t DimCols, typename T> class mat
		vec<DimCols, T> rows[DimRows];*/

	// ͨ��ģ����Ķ��壬�õ�����varying_uv, DimRows��2�� DimCols��3
	// ���Զ������ Vec3f rows[2] varying_uv, ����һ��2��3�еľ���
	mat<2, 3, float> varying_uv;  // triangle uv coordinates, written by VS read by FS

	// varying_nrmͬ��
	mat<3, 3, float> varying_nrm; // ÿ�����㱻Ƭ����ɫ����ֵ
	mat<4, 3, float> varying_tri; // triangle coordinates (clip coordinates)
								  // written by VS, read by FS

	// �����εı�׼���豸����(Normalized Device Coordinates, NDC)
	// NDC���ꡪ�����������ڶ�����ɫ���д�������Ǳ�׼���豸������
	// NDC���ڲü��ռ�����꣬x��y��zֵ��-1.0��1.0��һС�οռ�
	// �κ����ڷ�Χ������궼�ᱻ����/�ü���������ʾ����Ļ��
	// NDC������һ����任�� Screen-space Coordinates
	// ���������ͨ��ViewPort����任���ɵ�
	// ������ Screen-space Coordinates�ᱻ�任��Ƭ��
	// Ȼ��Ƭ�����뵽Ƭ����ɫ����
	mat<3, 3, float> ndc_tri;	  // triangle in normalized device coordinates

	virtual Vec4f vertex(int iface, int nthvert)
	{
		/*
		void set_col(size_t idx, vec<DimRows, T> v) 
		{
			assert(idx < DimCols);
			for (size_t i = DimRows; i--; rows[i][idx] = v[i]);
		}
		*/

		// �ã���̫���set_col()�������ó���������iface��nthvert��������ʱ����0
		// ����idx��0��v��Vec2f�ı��������շ��ص���
		// rows[1][0] = v[1]
		// rows[0][0] = v[0]
		// v[0]��ʾ����xֵ��v[1]��ʾ����yֵ
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		varying_nrm.set_col(nthvert, proj<3>((Projection * ModelView).invert_transpose() * 
			embed<4>(model->normal(iface, nthvert), 0.f)));

		// Projection * ModelView���Ǿ���M�����Խ�����ռ�ת��Ϊ�ü��ռ�
		// �����embed���ǽ�����������Ƕ��4D��
		// ������˺�����ת����Ķ�������
		Vec4f gl_Vertex = Projection * ModelView * embed<4>(model->vert(iface, nthvert));

		// set_col����˼�Ҳ��� �������С�
		// ע�⣺rows[i][1]��rows[i][2]��rows[i][3]��һ��
		// ��Ϊgl_vertexҲ�Ǹ��� nthvert = 0, 1, 2 ���ֱ���õ�
		varying_tri.set_col(nthvert, gl_Vertex);

		ndc_tri.set_col(nthvert, proj<3>(gl_Vertex / gl_Vertex[3])); // ʹ����������ˣ�����w

		return gl_Vertex;
	}

	virtual bool fragment(Vec3f bar, TGAColor& color) 
	{
		// �����bn��nrm�����Ͻڿε�û����ֻ��Ϊ�˼������ְ���
		Vec3f bn = (varying_nrm * bar).normalize();
		Vec2f uv = varying_uv * bar;

		// diff���ǹ���ǿ��

		/*float diff = std::max(0.f, bn * light_dir);*/
		// ��diff��д�ˣ���Ϊdiff�Ƚ���Ҫ
		mat<3, 3, float> A;
		A[0] = ndc_tri.col(1) - ndc_tri.col(0);  // (p1 - p0)��Ӧ(p0p1)����
												 // A[0][0]/A[0][1]/A[0][2]��Ӧx��y��z��������
		A[1] = ndc_tri.col(2) - ndc_tri.col(0);  // Same as above
		A[2] = bn;								 // bn��ʾ����ԭʼ��������n

		mat<3, 3, float> AI = A.invert();		 // AI��A�������

		// (varying_uv[0][1] - varying_uv[0][0]) ��ʾ(u1 - u0)
		// (varying_uv[0][2] - varying_uv[0][0]) ��ʾ(u2 - u0)
		Vec3f i = AI * Vec3f(varying_uv[0][1] - varying_uv[0][0],
							 varying_uv[0][2] - varying_uv[0][0], 
							 0);

		// (varying_uv[1][1] - varying_uv[1][0]) ��ʾ(v1 - v0)
		// (varying_uv[1][2] - varying_uv[1][0]) ��ʾ(v2 - v0)
		Vec3f j = AI * Vec3f(varying_uv[1][1] - varying_uv[1][0],
							 varying_uv[1][2] - varying_uv[1][0], 
							 0);

		/*
		void set_col(size_t idx, vec<DimRows, T> v)
		{
			assert(idx < DimCols);
			for (size_t i = DimRows; i--; rows[i][idx] = v[i]);
		}
		*/

		// Change of basis in 3D space
		// ����(i j bn)��Darboux����ϵ�Ļ�׼
		mat<3, 3, float> B;
		B.set_col(0, i.normalize()); //rows[2][0] = (u1 - u0), rows[1][0] = (u2 - u0), rows[0][0] = (0)
		B.set_col(1, j.normalize()); //rows[2][1] = (v1 - v0), rows[1][1] = (v2 - v0), rows[0][1] = (0)
		B.set_col(2, bn);  // rows[2][2] = bn.x, rows[1][2] = bn.y, rows[0][2] = bn.z

		// �µķ�������n(Darboux���)
		// �������������ת�������߿ռ�
		Vec3f n = (B * model->normal(uv)).normalize();

		float diff = std::max(0.f, n * light_dir);

		// diffuse()�õ�������������(u��v������)��ͨ��u��v�����꣬�õ��������ɫ
		// ������ɫ�͹�ǿ��ˣ����õ�����ͼ�ϵ���ɫcolor
		// ��������ɫcolor����our_gl.cpp�ļ��еĹ�դ����triangle()������Ӧ�õ�
		color = model->diffuse(uv) * diff; // ���������ô����룬����Ϊ�˸���color

		return false;
	}
};

int main(int argc, char** argv)
{
	float* zbuffer = new float[width * height];
	for (int i = width * height; i--; zbuffer[i] = -std::numeric_limits<float>::max());

	TGAImage frame(width, height, TGAImage::RGB);

	lookat(eye, center, up);

	// ��ͼ����
	viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);

	// ��͸��ϵ������͸�Ӿ���
	projection(-1.f / (eye - center).norm());

	light_dir = proj<3>((Projection * ModelView * embed<4>(light_dir, 0.f))).normalize();

	// for (int m = 1; m < argc; m++)
	// {
		// model = new Model(argv[m]);
		model = new Model("obj/african_head/african_head.obj");

		// ����һ����ɫ�����󣬴Ӷ����ǿ��Կ�ʼʹ����ɫ��
		Shader shader;

		for (int i = 0; i < model->nfaces(); i++)
		{

			for (int j = 0; j < 3; j++) 
			{
				shader.vertex(i, j);
			}
			triangle(shader.varying_tri, shader, frame, zbuffer);
		}
		delete model;
	//}

	frame.flip_vertically(); // to place the origin in the bottom left corner of the image
	frame.write_tga_file("framebuffer-BTNM.tga");

	delete[] zbuffer;
	return 0;
}