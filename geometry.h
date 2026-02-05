#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__
#include <cmath>
#include <vector>
#include <cassert>
#include <iostream>

// Forward declaration for dt structure used in mat class
template<size_t DIM, typename T> struct dt;

template <size_t DimCols, size_t DimRows, typename T> class mat;

template <size_t DIM, typename T> struct vec {
    vec() { for (size_t i = DIM; i--; data_[i] = T()); }
    T& operator[](const size_t i) { assert(i < DIM); return data_[i]; }
    const T& operator[](const size_t i) const { assert(i < DIM); return data_[i]; }
private:
    T data_[DIM];
};

typedef vec<2, float> vec2;
typedef vec<3, float> vec3;
typedef vec<4, float> vec4;

template <typename T> struct vec<2, T> {
    vec() : x(T()), y(T()) {}
    vec(T X, T Y) : x(X), y(Y) {}
    template <class U> vec(const vec<2, U>& v) : x((T)v.x), y((T)v.y) {}
    T& operator[](const size_t i) { assert(i < 2); return i <= 0 ? x : y; }
    const T& operator[](const size_t i) const { assert(i < 2); return i <= 0 ? x : y; }
    T x, y;
};

template <typename T> struct vec<3, T> {
    vec() : x(T()), y(T()), z(T()) {}
    vec(T X, T Y, T Z) : x(X), y(Y), z(Z) {}
    template <class U> vec(const vec<3, U>& v) : x((T)v.x), y((T)v.y), z((T)v.z) {}
    T& operator[](const size_t i) { assert(i < 3); return i <= 0 ? x : (1 == i ? y : z); }
    const T& operator[](const size_t i) const { assert(i < 3); return i <= 0 ? x : (1 == i ? y : z); }
    float norm() { return std::sqrt(x * x + y * y + z * z); }
    vec<3, T>& normalize(T l = 1) { *this = (*this) * (l / norm()); return *this; }
    T x, y, z;
};

template <typename T> struct vec<4, T> {
    vec() : x(T()), y(T()), z(T()), w(T()) {}
    vec(T X, T Y, T Z, T W) : x(X), y(Y), z(Z), w(W) {}
    template <class U> vec(const vec<4, U>& v) : x((T)v.x), y((T)v.y), z((T)v.z), w((T)v.w) {}
    T& operator[](const size_t i) { assert(i < 4); return i <= 0 ? x : (1 == i ? y : (2 == i ? z : w)); }
    const T& operator[](const size_t i) const { assert(i < 4); return i <= 0 ? x : (1 == i ? y : (2 == i ? z : w)); }
    T x, y, z, w;
};

template<size_t DIM, typename T> T operator*(const vec<DIM, T>& lhs, const vec<DIM, T>& rhs) {
    T ret = T();
    for (size_t i = DIM; i--; ret += lhs[i] * rhs[i]);
    return ret;
}

template<size_t DIM, typename T> vec<DIM, T> operator+(vec<DIM, T> lhs, const vec<DIM, T>& rhs) {
    for (size_t i = DIM; i--; lhs[i] += rhs[i]);
    return lhs;
}

template<size_t DIM, typename T> vec<DIM, T> operator-(vec<DIM, T> lhs, const vec<DIM, T>& rhs) {
    for (size_t i = DIM; i--; lhs[i] -= rhs[i]);
    return lhs;
}

template<size_t DIM, typename T, typename U> vec<DIM, T> operator*(vec<DIM, T> lhs, const U& rhs) {
    for (size_t i = DIM; i--; lhs[i] = (T)(lhs[i] * rhs));
    return lhs;
}

template<size_t DIM, typename T, typename U> vec<DIM, T> operator/(vec<DIM, T> lhs, const U& rhs) {
    for (size_t i = DIM; i--; lhs[i] = (T)(lhs[i] / rhs));
    return lhs;
}

template<size_t LEN, size_t DIM, typename T> vec<LEN, T> embed(const vec<DIM, T>& v, T fill = 1) {
    vec<LEN, T> ret;
    for (size_t i = LEN; i--; ret[i] = (i < DIM ? v[i] : fill));
    return ret;
}

template<size_t LEN, size_t DIM, typename T> vec<LEN, T> proj(const vec<DIM, T>& v) {
    vec<LEN, T> ret;
    for (size_t i = LEN; i--; ret[i] = v[i]);
    return ret;
}

template <typename T> vec<3, T> cross(vec<3, T> v1, vec<3, T> v2) {
    return vec<3, T>(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x);
}

template <size_t DIM, typename T> std::ostream& operator<<(std::ostream& out, vec<DIM, T>& v) {
    for (unsigned int i = 0; i < DIM; i++) {
        out << v[i] << " ";
    }
    return out;
}

/////////////////////////////////////////////////////////////////////////////////

template<size_t DimRows, size_t DimCols, typename T> class mat {
    vec<DimCols, T> rows[DimRows];
public:
    mat() {}

    vec<DimCols, T>& operator[](const size_t i) { assert(i < DimRows); return rows[i]; }
    const vec<DimCols, T>& operator[](const size_t i) const { assert(i < DimRows); return rows[i]; }

    vec<DimRows, T> col(const size_t i) const {
        assert(i < DimCols);
        vec<DimRows, T> ret;
        for (size_t j = DimRows; j--; ret[j] = rows[j][i]);
        return ret;
    }

    void set_col(size_t i, vec<DimRows, T> v) {
        assert(i < DimCols);
        for (size_t j = DimRows; j--; rows[j][i] = v[j]);
    }

    static mat<DimRows, DimCols, T> identity() {
        mat<DimRows, DimCols, T> ret;
        for (size_t i = DimRows; i--; )
            for (size_t j = DimCols; j--; ret[i][j] = (i == j));
        return ret;
    }

    T det() const {
        return dt<DimCols, T>::det(*this);
    }

    mat<DimRows, DimCols, T> invert() const {
        return dt<DimCols, T>::invert(*this);
    }

    mat<DimCols, DimRows, T> transpose() const {
        mat<DimCols, DimRows, T> ret;
        for (size_t i = DimCols; i--; ret[i] = this->col(i));
        return ret;
    }

    mat<DimRows - 1, DimCols - 1, T> get_minor(size_t row, size_t col) const {
        mat<DimRows - 1, DimCols - 1, T> ret;
        for (size_t i = DimRows - 1; i--; )
            for (size_t j = DimCols - 1; j--; )
                ret[i][j] = rows[i < row ? i : i + 1][j < col ? j : j + 1];
        return ret;
    }

    T cofactor(size_t row, size_t col) const {
        return get_minor(row, col).det() * ((row + col) % 2 ? -1 : 1);
    }
};

template<size_t DIM, typename T> struct dt {
    static T det(const mat<DIM, DIM, T>& src) {
        T ret = 0;
        for (size_t i = DIM; i--; ret += src[0][i] * src.cofactor(0, i));
        return ret;
    }

    static mat<DIM, DIM, T> invert(const mat<DIM, DIM, T>& src) {
        mat<DIM, DIM, T> ret;
        T determinant = src.det();
        for (size_t i = DIM; i--; )
            for (size_t j = DIM; j--; ret[i][j] = src.cofactor(i, j) / determinant);
        return ret.transpose();
    }
};

template<typename T> struct dt<1, T> {
    static T det(const mat<1, 1, T>& src) {
        return src[0][0];
    }

    static mat<1, 1, T> invert(const mat<1, 1, T>& src) {
        mat<1, 1, T> ret;
        ret[0][0] = (T)1 / src[0][0];
        return ret;
    }
};

template<size_t DimRows, size_t DimCols, typename T> vec<DimRows, T> operator*(const mat<DimRows, DimCols, T>& lhs, const vec<DimCols, T>& rhs) {
    vec<DimRows, T> ret;
    for (size_t i = DimRows; i--; ret[i] = lhs[i] * rhs);
    return ret;
}

template<size_t R1, size_t C1, size_t C2, typename T>mat<R1, C2, T> operator*(const mat<R1, C1, T>& lhs, const mat<C1, C2, T>& rhs) {
    mat<R1, C2, T> result;
    for (size_t i = R1; i--; )
        for (size_t j = C2; j--; result[i][j] = lhs[i] * rhs.col(j));
    return result;
}

template<size_t DimRows, size_t DimCols, typename T>mat<DimCols, DimRows, T> operator/(mat<DimRows, DimCols, T> lhs, const T& rhs) {
    for (size_t i = DimRows; i--; lhs[i] = lhs[i] / rhs);
    return lhs;
}

template <size_t DimRows, size_t DimCols, typename T> std::ostream& operator<<(std::ostream& out, mat<DimRows, DimCols, T>& m) {
    for (size_t i = 0; i < DimRows; i++) out << m[i] << std::endl;
    return out;
}

typedef mat<4, 4, float> Matrix;
typedef mat<3, 3, float> Matrix3x3;

#endif //__GEOMETRY_H__