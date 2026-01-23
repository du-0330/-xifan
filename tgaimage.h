
#pragma once
#include <cstdint>
#include <fstream>
#include <vector>

#pragma pack(push,1)
struct TGAHeader {
    std::uint8_t  idlength = 0;
    std::uint8_t  colormaptype = 0;
    std::uint8_t  datatypecode = 0;
    std::uint16_t colormaporigin = 0;
    std::uint16_t colormaplength = 0;
    std::uint8_t  colormapdepth = 0;
    std::uint16_t x_origin = 0;
    std::uint16_t y_origin = 0;
    std::uint16_t width = 0;
    std::uint16_t height = 0;
    std::uint8_t  bitsperpixel = 0;
    std::uint8_t  imagedescriptor = 0;
};
#pragma pack(pop)

struct TGAColor {
    std::uint8_t bgra[4] = { 0,0,0,0 };
    std::uint8_t bytespp = 4;

    // 1. 默认构造函数（允许你不带参数创建颜色）
    TGAColor() = default;

    // 2. 带参数的构造函数（这是你 main.cpp 里报错缺少的那个！）
    // 参数顺序是 R, G, B, A。TGA 内部存储顺序通常是 B, G, R, A
    TGAColor(std::uint8_t R, std::uint8_t G, std::uint8_t B, std::uint8_t A = 255) : bytespp(4) {
        bgra[0] = B;
        bgra[1] = G;
        bgra[2] = R;
        bgra[3] = A;
    }

    // 3. 另外两种实用的构造函数（以后可能用到）
    TGAColor(std::uint8_t v) : bytespp(1) {
        for (int i = 0; i < 4; i++) bgra[i] = 0;
        bgra[0] = v;
    }

    TGAColor(const std::uint8_t* p, std::uint8_t bpp) : bytespp(bpp) {
        for (int i = 0; i < (int)bpp; i++) {
            bgra[i] = p[i];
        }
        for (int i = bpp; i < 4; i++) {
            bgra[i] = 0;
        }
    }

    std::uint8_t& operator[](const int i) { return bgra[i]; }
    const std::uint8_t& operator[](const int i) const { return bgra[i]; }
};

struct TGAImage {
    enum Format { GRAYSCALE=1, RGB=3, RGBA=4 };
    TGAImage() = default;
    TGAImage(const int w, const int h, const int bpp, TGAColor c = {});
    bool  read_tga_file(const std::string filename);
    bool write_tga_file(const std::string filename, const bool vflip=true, const bool rle=true) const;
    void flip_horizontally();
    void flip_vertically();
    TGAColor get(const int x, const int y) const;
    void set(const int x, const int y, const TGAColor &c);
    int width()  const;
    int height() const;
private:
    bool   load_rle_data(std::ifstream &in);
    bool unload_rle_data(std::ofstream &out) const;
    int w = 0, h = 0;
    std::uint8_t bpp = 0;
    std::vector<std::uint8_t> data = {};
};
