#include <vector>
#include <cmath>
#include <iostream>
#include <limits>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

Model* model = NULL;
const int width = 800;
const int height = 800;

// 【Day5 调整】改变光照方向，从侧面打光，立体感更强
vec3 light_dir(1, 1, 1);
// 【Day5 新增】定义相机参数
vec3       eye(0.5, 0.5, 1.5);    // 相机位置 (从右上方看)
vec3    center(0, 0, 0);    // 看向原点
vec3        up(0, 1, 0);    // 头顶朝上

// --- 重心坐标计算 ---
vec3 barycentric(vec2 A, vec2 B, vec2 C, vec2 P) {
    vec3 s[2];
    s[0][0] = C.x - A.x;
    s[0][1] = B.x - A.x;
    s[0][2] = A.x - P.x;
    s[1][0] = C.y - A.y;
    s[1][1] = B.y - A.y;
    s[1][2] = A.y - P.y;

    vec3 u = cross(s[0], s[1]);

    if (std::abs(u.z) > 1e-2) {
        return vec3(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
    }
    return vec3(-1, 1, 1);
}

// --- 画三角形 (Z-buffer) ---
void triangle(vec3 t0, vec3 t1, vec3 t2, float* zbuffer, TGAImage& image, TGAColor color) {
    int minx = image.get_width() - 1;
    int miny = image.get_height() - 1;
    int maxx = 0;
    int maxy = 0;

    minx = std::max(0, (int)std::min({ minx, (int)t0.x, (int)t1.x, (int)t2.x }));
    miny = std::max(0, (int)std::min({ miny, (int)t0.y, (int)t1.y, (int)t2.y }));
    maxx = std::min(image.get_width() - 1, (int)std::max({ maxx, (int)t0.x, (int)t1.x, (int)t2.x }));
    maxy = std::min(image.get_height() - 1, (int)std::max({ maxy, (int)t0.y, (int)t1.y, (int)t2.y }));

    vec2 P;
    for (P.x = minx; P.x <= maxx; P.x++) {
        for (P.y = miny; P.y <= maxy; P.y++) {
            vec3 bc_screen = barycentric(vec2(t0.x, t0.y), vec2(t1.x, t1.y), vec2(t2.x, t2.y), P);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;

            float z = 0;
            z += t0.z * bc_screen.x;
            z += t1.z * bc_screen.y;
            z += t2.z * bc_screen.z;

            int idx = int(P.x + P.y * width);
            if (zbuffer[idx] < z) {
                zbuffer[idx] = z;
                image.set(P.x, P.y, color);
            }
        }
    }
}

// --- 【Day5 核心】LookAt 矩阵 ---
// 作用：计算一个矩阵，把世界坐标系变换到相机坐标系
Matrix lookat(vec3 eye, vec3 center, vec3 up) {
    // 1. 算 Z 轴 (视线方向，从 center 指向 eye)
    vec3 z = (eye - center).normalize();
    // 2. 算 X 轴 (右方向，up 叉乘 z)
    vec3 x = cross(up, z).normalize();
    // 3. 算 Y 轴 (上方向，z 叉乘 x，保证绝对垂直)
    vec3 y = cross(z, x).normalize();

    Matrix Minv = Matrix::identity();
    Matrix Tr = Matrix::identity();

    for (int i = 0; i < 3; i++) {
        // 构造旋转矩阵 (逆矩阵等于转置矩阵，把基向量横着填进去)
        Minv[0][i] = x[i];
        Minv[1][i] = y[i];
        Minv[2][i] = z[i];
        // 构造平移矩阵 (把相机移回原点)
        Tr[i][3] = -eye[i];
    }
    // 先平移，再旋转
    return Minv * Tr;
}

// --- 视口变换矩阵 ---
Matrix viewport(int x, int y, int w, int h) {
    Matrix m = Matrix::identity();
    m[0][3] = x + w / 2.f;
    m[1][3] = y + h / 2.f;
    m[2][3] = 255.f / 2.f;

    m[0][0] = w / 2.f;
    m[1][1] = h / 2.f;
    m[2][2] = 255.f / 2.f;
    return m;
}

int main(int argc, char** argv) {
    if (2 == argc) {
        model = new Model(argv[1]);
    }
    else {
        // 记得确认路径
        model = new Model("F:/TinyRenderer/myrenderer/obj/african_head/african_head.obj");
        // model = new Model("obj/african_head.obj");
    }

    if (!model || model->nverts() == 0) {
        std::cerr << "Error: 模型加载失败" << std::endl;
        return 1;
    }

    float* zbuffer = new float[width * height];
    for (int i = 0; i < width * height; i++) {
        zbuffer[i] = -std::numeric_limits<float>::max();
    }

    TGAImage image(width, height, TGAImage::RGB);

    // 【Day5 核心流程】
    // 1. 生成 ModelView 矩阵 (世界 -> 相机)
    Matrix ModelView = lookat(eye, center, up);

    // 2. 生成 ViewPort 矩阵
    Matrix ViewPort = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);

    // 3. 生成 Projection 矩阵
    Matrix Projection = Matrix::identity();
    // 系数是 -1 / (相机到目标的距离)
    Projection[3][2] = -1.f / (eye - center).norm();

    // 4. 组合成总变换矩阵 (如果不习惯这么写，也可以在循环里一步步乘)
    Matrix Transform = ViewPort * Projection * ModelView;

    // --- 渲染循环 ---
    for (int i = 0; i < model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        vec3 screen_coords[3];
        vec3 world_coords[3];

        for (int j = 0; j < 3; j++) {
            vec4 v = model->vert(face[j]);

            // 【Day5 核心变换】
            // 原始坐标 -> ModelView(搬到相机前) -> Projection(透视变形) -> ViewPort(放到屏幕上)
            // --- 【修正后】正确的写法 ---
            vec4 p = Transform * v;  // 1. 先算出 4D 结果
            screen_coords[j] = proj<3>(p / p[3]); // 2. 必须除以 p[3] (也就是 w)！这才是透视除法！

            // 世界坐标保持原样，用于算光照
            world_coords[j] = proj<3>(v);
        }

        vec3 n = cross(world_coords[1] - world_coords[0], world_coords[2] - world_coords[0]);
        n.normalize();

        // 归一化光照方向，保证点乘结果正确
        float intensity = n * light_dir.normalize();

        if (intensity > 0) {
            TGAColor color(intensity * 255, intensity * 255, intensity * 255, 255);
            triangle(screen_coords[0], screen_coords[1], screen_coords[2], zbuffer, image, color);
        }
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");

    delete[] zbuffer;
    delete model;
    return 0;
}