#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

// 构造函数：高级解析逻辑
Model::Model(const std::string filename) : verts(), norms(), tex(), facet_vrt(), facet_nrm(), facet_tex() {
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        // 读取顶点 v (存为 vec4)
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            vec4 v;
            for (int i = 0; i < 3; i++) iss >> v[i];
            v[3] = 1.f; // w 分量默认为 1
            verts.push_back(v);
        }
        // 读取法线 vn
        else if (!line.compare(0, 3, "vn ")) {
            iss >> trash >> trash;
            vec4 n;
            for (int i = 0; i < 3; i++) iss >> n[i];
            n[3] = 0.f;
            norms.push_back(n);
        }
        // 读取纹理 vt
        else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            vec2 uv;
            for (int i = 0; i < 2; i++) iss >> uv[i];
            tex.push_back(uv);
        }
        // 读取面 f
        else if (!line.compare(0, 2, "f ")) {
            int f, t, n;
            iss >> trash;
            int cnt = 0;
            while (iss >> f >> trash >> t >> trash >> n) {
                facet_vrt.push_back(--f);
                facet_tex.push_back(--t);
                facet_nrm.push_back(--n);
                cnt++;
            }
            if (3 != cnt) {
                std::cerr << "Error: obj 文件必须是三角形网格 (triangulated)" << std::endl;
                return;
            }
        }
    }
    std::cerr << "# v# " << nverts() << " f# " << nfaces() << " vt# " << tex.size() << " vn# " << norms.size() << std::endl;

    load_texture(filename, "_diffuse.tga", diffusemap_);
    load_texture(filename, "_nm_tangent.tga", normalmap_);
    load_texture(filename, "_spec.tga", specularmap_);
}

Model::~Model() {}

// 常用接口实现
int Model::nverts() const { return (int)verts.size(); }
int Model::nfaces() const { return (int)facet_vrt.size() / 3; }

vec4 Model::vert(const int i) const { return verts[i]; }

vec4 Model::vert(const int iface, const int nthvert) const {
    return verts[facet_vrt[iface * 3 + nthvert]];
}

// 【关键兼容接口】提取顶点索引给 main.cpp 用
std::vector<int> Model::face(int idx) {
    std::vector<int> face;
    for (int i = 0; i < 3; i++) {
        face.push_back(facet_vrt[idx * 3 + i]);
    }
    return face;
}

void Model::load_texture(std::string filename, const char* suffix, TGAImage& img) {
    std::string texfile(filename);
    size_t dot = texfile.find_last_of(".");
    if (dot != std::string::npos) {
        texfile = texfile.substr(0, dot) + std::string(suffix);
        if (img.read_tga_file(texfile.c_str())) {
            img.flip_vertically();
        }
    }
}

vec4 Model::normal(const int iface, const int nthvert) const { return norms[facet_nrm[iface * 3 + nthvert]]; }
vec4 Model::normal(const vec2& uv) const { return vec4(); } // 暂时留空，防止链接错误
vec2 Model::uv(const int iface, const int nthvert) const { return tex[facet_tex[iface * 3 + nthvert]]; }
const TGAImage& Model::diffuse()  const { return diffusemap_; }
const TGAImage& Model::specular() const { return specularmap_; }