#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include <string>
#include "geometry.h"
#include "tgaimage.h"

class Model {
private:
    std::vector<vec4> verts;     // 顶点数组
    std::vector<vec4> norms;     // 法线数组
    std::vector<vec2> tex;       // 纹理坐标数组

    // 高级版索引存储方式：三个独立的整数数组
    std::vector<int> facet_vrt;
    std::vector<int> facet_nrm;
    std::vector<int> facet_tex;

    // 纹理图片
    TGAImage diffusemap_;
    TGAImage normalmap_;
    TGAImage specularmap_;

    // 私有辅助函数
    void load_texture(std::string filename, const char* suffix, TGAImage& img);

public:
    Model(const std::string filename);
    ~Model();

    int nverts() const;
    int nfaces() const;

    vec4 vert(const int i) const;
    vec4 vert(const int iface, const int nthvert) const;

    // 【关键兼容接口】供 main.cpp 画线框使用
    std::vector<int> face(int idx);

    // 纹理和法线相关接口
    vec2 uv(const int iface, const int nthvert) const;
    vec4 normal(const int iface, const int nthvert) const;
    vec4 normal(const vec2& uv) const;

    const TGAImage& diffuse() const;
    const TGAImage& specular() const;
};

#endif //__MODEL_H__