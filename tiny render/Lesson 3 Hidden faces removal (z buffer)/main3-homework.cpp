#include <vector>
#include <cmath>
#include <limits>
#include "tgaimage3-homework.h"
#include "model3-homework.h"
#include "geometry3-homework.h"

const int width = 800;
const int height = 800;
const int depth = 255;

Model* model = NULL;
int* zbuffer = NULL;
Vec3f light_dir(0, 0, -1);

void triangle(Vec3i t0, Vec3i t1, Vec3i t2, Vec2i uv0, Vec2i uv1, Vec2i uv2, 
					TGAImage& image, float intensity, int* zbuffer)
{
	if (t0.y == t1.y && t0.y == t2.y) return; // i dont care about degenerate triangles

	if (t0.y > t1.y) 
	{
		std::swap(t0, t1);
		std::swap(uv0, uv1); 
	}

	if (t0.y > t2.y)
	{
		std::swap(t0, t2);
		std::swap(uv0, uv2); 
	}

	if (t1.y > t2.y) 
	{
		std::swap(t1, t2);
		std::swap(uv1, uv2); 
	}

	int total_height = t2.y - t0.y;
	for (int i = 0; i < total_height; i++)
	{
		bool second_half = i > t1.y - t0.y || t1.y == t0.y;

		int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;

		float alpha = (float)i / total_height;
		float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height; // be careful: with above conditions no division by zero here

		Vec3i A = t0 + Vec3f(t2 - t0) * alpha;
		Vec3i B = second_half ? t1 + Vec3f(t2 - t1) * beta : t0 + Vec3f(t1 - t0) * beta;

		Vec2i uvA = uv0 + (uv2 - uv0) * alpha;
		Vec2i uvB = second_half ? uv1 + (uv2 - uv1) * beta : uv0 + (uv1 - uv0) * beta;

		if (A.x > B.x) 
		{
			std::swap(A, B);
			std::swap(uvA, uvB); 
		}

		for (int j = A.x; j <= B.x; j++) 
		{
			float phi = (B.x == A.x ? 1. : (float)(j - A.x) / (float)(B.x - A.x));

			Vec3i   P = Vec3f(A) + Vec3f(B - A) * phi;
			Vec2i uvP = uvA + (uvB - uvA) * phi;

			int idx = P.x + P.y * width;

			if (zbuffer[idx] < P.z) 
			{
				zbuffer[idx] = P.z;
				TGAColor color = model->diffuse(uvP);  // ȡ�����������ɫ
				image.set(P.x, P.y, TGAColor(color.r * intensity, color.g * intensity, color.b * intensity));
			}
		}
	}
}

int main(int argc, char** argv) 
{
	if (2 == argc)
	{
		model = new Model(argv[1]);
	}
	else
	{
		model = new Model("obj/african_head.obj");
	}

	zbuffer = new int[width * height];
	for (int i = 0; i < width * height; i++) 
	{
		zbuffer[i] = std::numeric_limits<int>::min();
	}

	{
		// draw the model
		TGAImage image(width, height, TGAImage::RGB);
		for (int i = 0; i < model->nfaces(); i++) 
		{
			std::vector<int> face = model->face(i);

			Vec3i screen_coords[3];
			Vec3f world_coords[3];

			for (int j = 0; j < 3; j++)
			{
				Vec3f v = model->vert(face[j]);

				screen_coords[j] = Vec3i((v.x + 1.) * width / 2., (v.y + 1.) * height / 2., (v.z + 1.) * depth / 2.);
				world_coords[j] = v;
			}

			Vec3f n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);

			n.normalize();
			float intensity = n * light_dir;

			if (intensity > 0)
			{
				Vec2i uv[3];
				for (int k = 0; k < 3; k++) 
				{
					uv[k] = model->uv(i, k);
				}

				triangle(screen_coords[0], screen_coords[1], screen_coords[2], 
							uv[0], uv[1], uv[2], image, intensity, zbuffer);
			}
		}

		image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
		image.write_tga_file("output.tga");
	}

	{
		// dump z-buffer (debugging purposes only)
		TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
		for (int i = 0; i < width; i++) 
		{
			for (int j = 0; j < height; j++) 
			{
				zbimage.set(i, j, TGAColor(zbuffer[i + j * width], 1));
			}
		}
		zbimage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
		zbimage.write_tga_file("zbuffer.tga");
	}

	delete model;
	delete[] zbuffer;
	return 0;
}
