/*
	��Ⱦ�����յĳ��֣����߿ռ䷨����ͼ��������ͼ����Ӱ��ͼ

	��һ��:	1. ����ģ��Modelָ��ΪNULL
			2. ���崰�ڴ�С
			3. ����Ĺ��շ���light_dir���۲����۾�eye����������center���Զ����������up

	�ڶ���:	1. ���������ɫ��DepthShader����ɫ��Shader
			2. ������ɫ���е�VS��FS

	������:	1. main()������������Ȼ���z-buffer����Ӱ����shadow-buffer
			2. ��Ⱦ��Ӱ����shadowbuffer
			3. ��Ⱦ֡����(z-buffer)

	���Ĳ�:	1. �����ڴ�
			2. ���ؽ��
*/


////////////////////////��ӿ��ļ�////////////////////////////
#include <vector>
#include <limits>
#include <iostream>
#include <algorithm>

////////////////////////���ͷ�ļ�////////////////////////////
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl.h"


////////////////////////��һ�����������////////////////////////////
Model* model = NULL;
float* shadowbuffer = NULL;

const int width = 800;
const int height = 800;

// 4����������3ά������
Vec3f light_dir(1, 1, 0);
Vec3f center(0, 0, 0);
Vec3f eye(1, 1, 4);
Vec3f up(0, 1, 0);


////////////////////////�ڶ�����������ɫ��////////////////////////////
struct Shader : public IShader
{
	/////////////////����ת������Ͷ�����������/////////////////////////

	// ����3��4άת������(float)����ת���������꣬������ռ䵽�ü��ռ�
	mat<4, 4, float> uniform_M;

	// ת������������������ռ䵽�ü��ռ�
	mat<4, 4, float> uniform_MIT;

	// ת��֡����������֡��������Ļ����ת��Ϊ��Ӱ��������Ļ���� 
	mat<4, 4, float> uniform_M_Shadow;

	// ���������εĶ���������������꣬�����������������ɫ��������
	// ����������2ά3�о�����ʽ
	mat<2, 3, float> varying_uv;

	// ����������3ά3�о�����ʽ
	mat<3, 3, float> varying_tri_ver;  // �������ζ������꣬�������յ���Ļ�ռ�����


	///////////////////������ɫ��/////////////////////////

	// ���캯����������ɫ������
	Shader(Matrix M, Matrix MIT, Matrix MShadow) : uniform_M(M), uniform_MIT(MIT), uniform_M_Shadow(MShadow),
													varying_uv(), varying_tri_ver() {}
	
	// ������ɫ�����麯��
	// ����������Ӿֲ��ռ䣬ת��������Ļ�ռ�
	virtual Vec4f vertex(int iface, int nthvert)
	{
		Vec4f obj_ver	 = embed<4>(model->vert(iface, nthvert));
		Vec4f screen_ver = Viewport * Projection * ModelView * obj_ver;

		// ����ÿ���������������uvֵ
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));

		// ����ͶӰ��Ķ���ֵ
		varying_tri_ver.set_col(nthvert, proj<3>(screen_ver / screen_ver[3]));

		// ���ر任��Ķ������꣬����Ƭ����ɫ��FS
		return screen_ver;
	}

	// Ƭ����ɫ�����麯��
	// Ƭ����ɫ���õ�ÿһ�����ص�������ɫֵ
	virtual bool fragment(Vec3f bc, TGAColor& color)
	{
		// �õ�Shadowbuffer�д洢�Ķ���ֵ
		// uniform_M_Shadow�ǽ����������֡����ת������Ӱ���壬ʹ����������bc�Զ�����в�ֵ

		Vec4f shadowBuffer_pts = uniform_M_Shadow * embed<4>(varying_tri_ver * bc);
		shadowBuffer_pts = shadowBuffer_pts / shadowBuffer_pts[3];

		// ����shadowbuffer�����е�����
		int idx = int(shadowBuffer_pts[0]) + int(shadowBuffer_pts[1]) * width; // 2άת1ά

		// ��Ӱ��ͼ�л���֡�z-fighting������ȳ�ͻ
		// ����һ����Magic Coefficient�����Ӷ�����z-fighting
		float shadow = .3 + .7 * (shadowbuffer[idx] < shadowBuffer_pts[2] + 43.34);

		// varying_uv��2��3�ľ���ͨ������������bc��ֵ���õ���ǰ���ص�uv��������
		Vec2f uv = varying_uv * bc;

		// ���巨������
		Vec3f normal = model->normal(uv);
		// ����������ת��������һ����
		Vec3f n = proj<3>(uniform_MIT * embed<4>(normal)).normalize();

		// ת�����շ���light_dir
		Vec3f lgd = proj<3>(uniform_M * embed<4>(light_dir)).normalize();

		// ���岢���㷴��� reflected light
		Vec3f re = (n * (n * lgd * 2.f) - lgd).normalize();

		// ���淴�䣬������uv����Ϊ����ļ���Ҳ����ͼ����������ͼ��Ϣ�洢��uv��
		// ��Ϊ���ӻ�ѹⷴ�䵽��������������ʹ����pow��ģ��������ͼ�;��淴��ı���
		float spec = pow(std::max(re.z, 0.0f), model->specular(uv));

		// �������ǹ���ǿ�ȵ�����ֵcos(���)
		float diff = std::max(0.f, n * lgd);

		// Phong shading
		TGAColor color_Mapping = model->diffuse(uv);

		// ͨ�� Phone Shading �Ĺ�ʽ
		// �õ�ÿһ���������յ�r g bֵ
		for (int i = 0; i < 3; i++)
		{
			color[i] = std::min<float>(20 + color_Mapping[i] * shadow * (1.2 * diff + .6 * spec), 255);
		}

		return false;
	}
};


////////////////////////�ڶ��������������ɫ��////////////////////////////
struct DepthShader : public IShader
{
	// ���������εĶ������꣬������ʽ
	mat<3, 3, float> varying_tri;

	// ���캯����ʼ���б�
	DepthShader() : varying_tri() {}

	// ������ɫ��
	virtual Vec4f vertex(int iface, int nthvert)
	{
		// ��.obj�ļ��ж�ȡ������������
		Vec4f obj_ver_coord = embed<4>(model->vert(iface, nthvert));

		// ����ȡ���Ķ�������ת��Ϊ��Ļ�ռ�����
		Vec4f screen_ver_coord = Viewport * Projection * ModelView * obj_ver_coord;

		// ��varying_tri�����е�ÿһ����������(v/vt/vn)����������������������ֵ
		// ��4D����ͶӰ��3D��x y z����������w����(͸��ͶӰ)
		varying_tri.set_col(nthvert, proj<3>(screen_ver_coord / screen_ver_coord[3]));

		return screen_ver_coord;
	}

	virtual bool fragment(Vec3f bc, TGAColor& color)
	{
		// varying_tri���Ƕ�����ɫ������Ƭ����ɫ���Ķ�������
		Vec3f depth_tri = varying_tri * bc;

		// �����ɫ��DepthShaderֻ��Ҫ����zֵ
		color = TGAColor(255, 255, 255) * (depth_tri.z / depth);

		return false;
	}
};



////////////////////////��������main()����////////////////////////////

int main(int argc, char** argv)
{
	/////////////////////����2��������////////////////////////

	// ����֡���壬��������СΪ���ڴ�С
	float* zbuffer = new float[width * height];

	// ��ȫ�ֱ���shadowbuffer���ٻ������Ŀռ��С
	shadowbuffer = new float[width * height];

	// ��ʼ��zbuffer��shadowbuffer��ÿһ�����ض�Ӧ����ĳ�ֵ����ֵ��float���͵ĸ������
	for (int i = 0; i < width * height; i++)
	{
		zbuffer[i] = shadowbuffer[i] = -std::numeric_limits<float>::max();
	}


	/////////////////////����ģ��/////////////////////
	model = new Model("obj/african_head.obj");  // ("obj/dialo_pose3.obj")

	light_dir.normalize();

	{
		/////////////��Ⱦ��Ȼ�����///////////////
		TGAImage DepthImage(width, height, TGAImage::RGB);


		lookat(light_dir, center, up); 	// ��depthShader�У�light_dir���շ�����ǡ����۷���eye��
		viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
		projection(0); 					// Ҳ������͸��ͶӰ��ϵ��Ϊ0

		DepthShader depth_Shader;
		Vec4f screen_coords[3];			// ��������ת��Ϊ��Ļ�ռ�����

		// ö��ÿһ��face��Ԫ
		for (int i = 0; i < model->nfaces(); i++)
		{
			// ö��ÿһ��С�������е�ÿһ������(��3������)
			for (int j = 0; j < 3; j++)
			{
				// ���ö�����ɫ��vertex shader
				screen_coords[j] = depth_Shader.vertex(i, j);
			}

			// ÿһ��face(��Ԫ)��Ӧһ��������
			// ÿ������һ���������е�3�������ֵ�����ù�դ����triangle()
			triangle(screen_coords, depth_Shader, DepthImage, shadowbuffer);
		}

		DepthImage.flip_vertically(); // ͼ�����������Ͻǣ����ｫ��䵽���½�
		DepthImage.write_tga_file("Final render image.tga");
	}

	Matrix M_Local2Screen = Viewport * Projection * ModelView;

	{
		/////////////��Ⱦ֡������///////////////
		TGAImage finalFrame(width, height, TGAImage::RGB);

		Vec4f screen_coords[3];

		lookat(eye, center, up); // ����Ĺ۲��߲����ǹ��շ��򣬶��Ƕ������Ӱ��λ��eye
		viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
		projection(-1.f / (eye - center).norm());  // ϵ����Ϊ(-1/��������) 

		// ��3�������������ǽ�framebuffer֡��������Ļ�ռ�ת��Ϊshadowbuffer��Ӱ��������Ļ�ռ�ľ���
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

	////////////////////////���Ĳ��������ڴ桢���ؽ��////////////////////////
	delete model;

	// ע�⣺����������������ʽ
	delete [] zbuffer;
	delete [] shadowbuffer;

	return 0;
}


/*
	�ã������������Ӱ��ͼ��������ͼ��������ͼ��������ͼ����Ⱦ�������ˡ�
	�ܽ�һ����ε�ʧ��viewport()�����к�����������һ��д����width,height�����ɵ�image
	���Ǻ�ɫ�ġ���Ϊ(width * 3 / 4)��(width * 3 / 4)��ͺ���
*/