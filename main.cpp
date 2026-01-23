#include "tgaimage.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include "model.h"
#include "geometry.h"
#include <vector>
Model* model = NULL;
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
void line (int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    bool steep = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);

    int derror2 = std::abs(dy) * 2;
    int error2 = 0;
    int y = y0;

    for (int x = x0; x <= x1; x++) {
        if (steep) {
            image.set(y, x, color);
        }
        else {
            image.set(x, y, color);
        }
        error2 += derror2;
        if (error2 > dx) {
            y += (y1 > y0 ? 1 : -1);
            error2 -= dx * 2;
        }
    }
}
int main(int argc, char** argv) {
    if (2 == argc) {
        model = new Model(argv[1]);
    }
    else {
        model = new Model("F:/TinyRenderer/myrenderer/obj/african_head/african_head.obj");
    }

    // 检查加载是否成功
    if (model->nverts() == 0) {
        std::cerr << "Error: 模型加载失败，请检查文件路径！" << std::endl;
        return 1;
    }
    TGAImage image(800, 800, TGAImage::RGB);
    for (int i = 0; i < model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        for (int j = 0; j < 3; j++) {
            vec4 v0 = model->vert(face[j]);
            vec4 v1 = model->vert(face[(j + 1) % 3]);
            int x0 = (v0.x + 1.) * 800 / 2;
            int y0 = (v0.y + 1.) * 800 / 2;
            int x1 = (v1.x + 1.) * 800 / 2;
            int y1 = (v1.y + 1.) * 800 / 2;
            
            line(x0, y0, x1, y1, image, white);
        }
    }
    image.write_tga_file("output.tga"); // 保存文件

    // 释放模型内存
    delete model;
    return 0;
}