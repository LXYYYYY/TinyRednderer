#include <vector>
#include <cmath>
#include <cstdlib>
#include <limits>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "IShader.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
Model* model = NULL;
const int width = 1024;
const int height = 1024;
const int depth = 255;
Vec3i light_dir = vec3(1, 1, 1).normalize();
Vec3i camera(0, 0, 3);
Vec3i eye(1, 1, 3);
Vec3i center(0, 0, 0);
Vec3i up(0, 1, 0);

class GouraudShader : public IShader
{
public:
    vec3 varying_intensity;
    mat<2, 3> varying_uv;        // same as above
    virtual vec4 vertex(int iface, int nthvert)
    {
        vec4 gl_vertex = embed<4>(model->vert(iface, nthvert));
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));
        gl_vertex = ViewPort * Projection * ModelView * gl_vertex;
        varying_intensity[nthvert] = std::max(double(0.f), model->normal(iface, nthvert) * light_dir);
        return gl_vertex;
    }

    virtual bool fragment(vec3 bar, TGAColor& color)
    {
        float intensity = varying_intensity * bar;
        vec2 uv = varying_uv * bar;
        TGAColor texColor = model->diffuse(uv);
        //TGAColor texColor(255, 255, 255);
        //std::cout << intensity << " ";
        color = model->diffuse(uv) * intensity;
        return false;
    }


}; 

int main() {
    model = new Model("resources/model/african_head.obj");
    GouraudShader shader;

    shader.lookAt(eye, center, up);
    shader.viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4, depth);
    shader.projection(-1.f / (eye - center).norm());
    light_dir.normalize();

    TGAImage image(width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);
    for (int i = 0; i < model->nfaces(); i++) {

        vec4 screen_coords[3];
        for (int j = 0; j < 3; j++) screen_coords[j] = shader.vertex(i, j);
        //for (int j = 0; j < 4; j++)screen_coords[i][j] = int(screen_coords[i][j]);
        //std::cout << screen_coords[i];
        shader.triangle(screen_coords, shader, image, zbuffer);
        
    }

    image.flip_vertically(); // to place the origin in the bottom left corner of the image
    zbuffer.flip_vertically();
    image.write_tga_file("chapter6.tga");
    zbuffer.write_tga_file("zbuffer.tga");

    delete model;
    return 0;


        //std::vector<int> face = model->face(i);
        //Vec3f world_coords[3];
        //Vec3f pts[3];
        //for (int j = 0; j < 3; j++)
        //{
        //    Vec3f v = model->vert(face[j]);
        //    //pts[j] = world2screen(v);

        //    pts[j] = m2v(ViewPort * Projection * ModelView * v2m(v));
        //    for (int i = 0; i < 3; i++)pts[j][i] = int(pts[j][i]);
        //    //std::cout << pts[j];
        //    world_coords[j] = v;
        //}
        //Vec3f n = cross((world_coords[2] - world_coords[0]), (world_coords[1] - world_coords[0]));
        //n.normalize();
        //float intensity = n * light_dir;
        //if (intensity > 0)
        //{
        //    Vec2i uv[3];
        //    for (int k = 0; k < 3; k++)
        //    {
        //        uv[k] = model->uv(i, k);
        //    }
        //    triangle(pts, uv, zbuffer, image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255), intensity);
        //    //triangle(pts[0], pts[1], pts[2], uv[0], uv[1], uv[2], image, intensity, zbuffer);

        //}
};