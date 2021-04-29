#pragma once
#include "tgaimage.h"
#include "geometry.h"

class IShader
{
public:
	mat4 ModelView;
	mat4 ViewPort;
	mat4 Projection;

	//virtual ~IShader();
	virtual vec4 vertex(int iface, int nthvert) = 0;
	virtual bool fragment(vec3 bar, TGAColor& color) = 0;
	void viewport(int x, int y, int w, int h, int depth);
	void projection(float coeff = 0.f);
	void lookAt(vec3 eye, vec3 center, vec3 up);
	void triangle(vec4* pts, IShader& shader, TGAImage& image, TGAImage& zbuffer);
};

