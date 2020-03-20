#include <vector>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <algorithm>

#include "tgaimage3-2.h"
#include "geometry3-2.h"
#include "model3-2.h"



const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

Model* model = NULL;

const int width = 800;
const int height = 800;

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color)
{
	bool steep = false;

	if (std::abs(x0 - x1) < std::abs(y0 - y1))
	{
		std::swap(x0, y0);
		std::swap(x1, y1);

		steep = true;
	}

	if (x0 > x1)
	{
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	for (int x = x0; x <= x1; x++)
	{
		float t = (x - x0) / (float)(x1 - x0);
		int y = y0 * 1. + (y1 - y0) * t;

		if (steep)
		{
			image.set(y, x, color);
		}

		else
		{
			image.set(x, y, color);
		}
	}
}


Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P)
{
	Vec3f s[2];
	for (int i = 2; i--; )
	{
		s[i][0] = C[i] - A[i];
		s[i][1] = B[i] - A[i];
		s[i][2] = A[i] - P[i];
	}

	Vec3f u = cross(s[0], s[1]);

	if (std::abs(u[2]) > 1e-2)  // 1e-2 represents 10^-2 which is 0.01 and it should be float not int.
								// You have to declare that as float.
								// Dont forget that u[2] is integer. 
								// If it is zero then triangle ABC is degenerate
		return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
	return Vec3f(-1, 1, 1);
}

void triangle(Vec3f* pts, float* zbuffer, TGAImage& image, TGAColor color)
{
	Vec2f bboxmin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

	Vec2f clamp(image.get_width() - 1, image.get_height() - 1);

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			bboxmin[j] = std::max(0.f,      std::min(bboxmin[j], pts[i][j]));
			bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
		}
	}


	Vec3f P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
	{
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
		{
			Vec3f bc_screen = barycentric(pts[0], pts[1], pts[2], P);

			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
			
			P.z = 0;
			for (int i = 0; i < 3; i++) P.z += pts[i][2] * bc_screen[i];

			if (zbuffer[int(P.x + P.y * width)] < P.z)
			{
				zbuffer[int(P.x + P.y * width)] = P.z;
				image.set(P.x, P.y, color);
			}
		}
	}
}


Vec3f world2screen(Vec3f v)
{
	return Vec3f(int((v.x + 1.) * width / 2. + .5), int((v.y + 1.) * height / 2. + .5), v.z);
}


int main(int argc, char** argv)
{
	{
		if (2 == argc)
			model = new Model(argv[1]);
		else
			model = new Model("obj/african_head.obj");

		float* zbuffer = new float[width * height];
		for (int i = width * height; i--; zbuffer[i] = -std::numeric_limits<float>::max());

		TGAImage image(width, height, TGAImage::RGB);

		for (int i = 0; i < model->nfaces(); i++)
		{
			std::vector<int> face = model->face(i);

			Vec3f pts[3];

			for (int i = 0; i < 3; i++) pts[i] = world2screen(model->vert(face[i]));
			triangle(pts, zbuffer, image, TGAColor(rand() % 255, rand() % 255, rand() % 255, 255));
		}

		image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
		image.write_tga_file("Lesson 3-2 clown.tga");

		delete model;
	}

	{
		if (2 == argc)
			model = new Model(argv[1]);
		else
			model = new Model("obj/african_head.obj");

		// Ϊ������ͷ������ϣ�������z-buffer
		float* zbuffer = new float[width * height];
		for (int i = width * height; i--; zbuffer[i] = -std::numeric_limits<float>::max());

		TGAImage image(width, height, TGAImage::RGB);
		
		Vec3f light_dir(0, 0, -1);
		for (int i = 0; i < model->nfaces(); i++)
		{
			std::vector<int> face = model->face(i);

			Vec3f real_pts[3];

			Vec3f pts[3];

			
			for (int j = 0; j < 3; j++)
			{
				real_pts[j] = model->vert(face[j]);
				pts[j] = world2screen(model->vert(face[j]));
				
			}

			Vec3f n = cross(real_pts[1] - real_pts[0], real_pts[2] - real_pts[0]);
			n.normalize();

			float intensity = n * light_dir;
			
			if (intensity < 0.)
			{
				triangle(pts, zbuffer, image, TGAColor(int(-intensity * 255), int(-intensity * 255),
					int(-intensity * 255), 255));
			}
			
		}

		image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
		image.write_tga_file("Lesson 3-2 lightIntensityLI.tga");


		delete model;
	}

	return 0;
}

