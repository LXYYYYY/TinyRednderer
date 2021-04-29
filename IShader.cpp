#include "IShader.h"

vec3 barycentric(vec2 A, vec2 B, vec2 C, vec2 P) {
    vec3 s[2];
    for (int i = 2; i--; ) {
        s[i][0] = C[i] - A[i];
        s[i][1] = B[i] - A[i];
        s[i][2] = A[i] - P[i];
    }
    vec3 u = cross(s[0], s[1]);// (u,v,1)同时垂直于s[0]和s[1],那么就是叉乘结果
    if (std::abs(u[2]) > 1e-2) // 讨论的是二维上的三角形，如果u的z分量不是1那么就不在三角形内
        return vec3(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
    return vec3(-1, 1, 1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}

void IShader::viewport(int x, int y, int w, int h, int depth)
{
    this->ViewPort = mat4::identity();
    this->ViewPort[0][3] = double(x) + w / 2.f;
    this->ViewPort[1][3] = double(y) + h / 2.f;
    this->ViewPort[2][3] = depth / 2.f;
    this->ViewPort[0][0] = w / 2.f;
    this->ViewPort[1][1] = h / 2.f;
    this->ViewPort[2][2] = depth / 2.f;
}

void IShader::projection(float coeff)
{
    this->Projection = mat4::identity();
    this->Projection[3][2] = coeff;
}

void IShader::lookAt(vec3 eye, vec3 center, vec3 up)
{
    vec3 z = (eye - center).normalize();
    vec3 x = cross(z, up);
    vec3 y = cross(x, z);
    mat4 move2origin = move2origin.identity();
    mat4 rotate = rotate.identity();
    for (int i = 0; i < 3; i++)
    {
        move2origin[i][3] = -center[i];
        rotate[0][i] = x[i];
        rotate[1][i] = y[i];
        rotate[2][i] = z[i];
    }
    ModelView = rotate * move2origin;
}

void IShader::triangle(vec4* pts, IShader& shader, TGAImage& image, TGAImage& zbuffer)
{
    Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            bboxmin[j] = std::min(bboxmin[j], pts[i][j] / pts[i][3]);
            bboxmax[j] = std::max(bboxmax[j], pts[i][j] / pts[i][3]);
        }
    }
    vec2 P;
    TGAColor texcolor;
    //std::cout << "bboxmin.y:" << bboxmin.y << "bboxmax.y:" << bboxmax.y << "bboxmin.x:" << bboxmin.x << "bboxmax.x:" << bboxmax.x<<std::endl;
    for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
        for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
            Vec3f bc_screen = barycentric(proj<2>(pts[0] / pts[0][3]), proj<2>(pts[1] / pts[1][3]), proj<2>(pts[2] / pts[2][3]), proj<2>(P));
            float z = pts[0][2] * bc_screen.x + pts[1][2] * bc_screen.y + pts[2][2] * bc_screen.z;
            float w = pts[0][3] * bc_screen.x + pts[1][3] * bc_screen.y + pts[2][3] * bc_screen.z;        
            int frag_depth = std::max(0, std::min(255, int(z / w + .5)));
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0 || zbuffer.get(P.x, P.y)[0] > frag_depth) continue;
            bool discard = shader.fragment(bc_screen, texcolor);
            //std::cout << texcolor.r << " " << texcolor.g << " " << texcolor.b << std::endl;
            if (!discard)
            {
                zbuffer.set(P.x, P.y, TGAColor(frag_depth));
                //image.set(P.x, P.y, color);
                image.set(P.x, P.y, texcolor);
            }
        }
    }
}
