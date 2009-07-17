/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#ifndef _CXCORE_HPP_
#define _CXCORE_HPP_

#include "cxmisc.h"

#ifdef __cplusplus

#ifndef SKIP_INCLUDES
#include <algorithm>
#include <complex>
#include <map>
#include <new>
#include <string>
#include <vector>
#endif // SKIP_INCLUDES

namespace cv {

template<typename _Tp> class CV_EXPORTS Size_;
template<typename _Tp> class CV_EXPORTS Point_;
template<typename _Tp> class CV_EXPORTS Rect_;

typedef std::string String;
typedef std::basic_string<wchar_t> WString;

CV_EXPORTS String fromUtf16(const WString& str);
CV_EXPORTS WString toUtf16(const String& str);

class CV_EXPORTS Exception
{
public:
    Exception() { code = 0; line = 0; }
    Exception(int _code, const String& _err, const String& _func, const String& _file, int _line)
        : code(_code), err(_err), func(_func), file(_file), line(_line) {}
    Exception(const Exception& exc)
        : code(exc.code), err(exc.err), func(exc.func), file(exc.file), line(exc.line) {}
    Exception& operator = (const Exception& exc)
    {
        if( this != &exc )
        {
            code = exc.code; err = exc.err; func = exc.func; file = exc.file; line = exc.line;
        }
        return *this;
    }

    int code;
    String err;
    String func;
    String file;
    int line;
};

CV_EXPORTS String format( const char* fmt, ... );
CV_EXPORTS void error( const Exception& exc );

#ifdef __GNUC__
#define CV_Error( code, msg ) cv::error( cv::Exception(code, msg, __func__, __FILE__, __LINE__) )
#define CV_Error_( code, args ) cv::error( cv::Exception(code, cv::format args, __func__, __FILE__, __LINE__) )
#define CV_Assert( expr ) { if(!(expr)) cv::error( cv::Exception(CV_StsAssert, #expr, __func__, __FILE__, __LINE__) ); }
#else
#define CV_Error( code, msg ) cv::error( cv::Exception(code, msg, "", __FILE__, __LINE__) )
#define CV_Error_( code, args ) cv::error( cv::Exception(code, cv::format args, "", __FILE__, __LINE__) )
#define CV_Assert( expr ) { if(!(expr)) cv::error( cv::Exception(CV_StsAssert, #expr, "", __FILE__, __LINE__) ); }
#endif
    
#ifdef _DEBUG
#define CV_DbgAssert(expr) CV_Assert(expr)
#else
#define CV_DbgAssert(expr)
#endif

CV_EXPORTS void setNumThreads(int);
CV_EXPORTS int getNumThreads();
CV_EXPORTS int getThreadNum();

CV_EXPORTS int64 getTickCount();
CV_EXPORTS double getTickFrequency();

CV_EXPORTS void* fastMalloc(size_t);
CV_EXPORTS void fastFree(void* ptr);

template<typename _Tp> static inline _Tp* allocate(size_t n)
{
    _Tp* ptr = (_Tp*)fastMalloc(n*sizeof(ptr[0]));
    ::new(ptr) _Tp[n];
    return ptr;
}

template<typename _Tp> static inline void deallocate(_Tp* ptr, size_t n)
{
    for( size_t i = 0; i < n; i++ ) (ptr+i)->~_Tp();
    fastFree(ptr);
}

template<typename _Tp> static inline _Tp* alignPtr(_Tp* ptr, int n=(int)sizeof(_Tp))
{
    return (_Tp*)(((size_t)ptr + n-1) & -n);
}

static inline size_t alignSize(size_t sz, int n)
{
    return (sz + n-1) & -n;
}

CV_EXPORTS void setUseOptimized(bool);
CV_EXPORTS bool useOptimized();

template<typename _Tp> class CV_EXPORTS Allocator
{
public: 
    typedef _Tp value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    template<typename U> class rebind { typedef Allocator<U> other; };

    explicit Allocator() {}
    ~Allocator() {}
    explicit Allocator(Allocator const&) {}
    template<typename U>
    explicit Allocator(Allocator<U> const&) {}

    // address
    pointer address(reference r) { return &r; }
    const_pointer address(const_reference r) { return &r; }

    pointer allocate(size_type count, const void* =0)
    { return reinterpret_cast<pointer>(fastMalloc(count * sizeof (_Tp))); }

    void deallocate(pointer p, size_type) {fastFree(p); }

    size_type max_size() const
    { return max(static_cast<_Tp>(-1)/sizeof(_Tp), 1); }

    void construct(pointer p, const _Tp& v) { new(static_cast<void*>(p)) _Tp(v); }
    void destroy(pointer p) { p->~_Tp(); }
};

/////////////////////// Vec (used as element of multi-channel images ///////////////////// 

template<typename _Tp> class CV_EXPORTS DataDepth { public: enum { value = -1, fmt=(int)'\0' }; };

template<> class DataDepth<bool> { public: enum { value = CV_8U, fmt=(int)'u' }; };
template<> class DataDepth<uchar> { public: enum { value = CV_8U, fmt=(int)'u' }; };
template<> class DataDepth<schar> { public: enum { value = CV_8S, fmt=(int)'c' }; };
template<> class DataDepth<ushort> { public: enum { value = CV_16U, fmt=(int)'w' }; };
template<> class DataDepth<short> { public: enum { value = CV_16S, fmt=(int)'s' }; };
template<> class DataDepth<int> { public: enum { value = CV_32S, fmt=(int)'i' }; };
template<> class DataDepth<float> { public: enum { value = CV_32F, fmt=(int)'f' }; };
template<> class DataDepth<double> { public: enum { value = CV_64F, fmt=(int)'d' }; };
template<typename _Tp> class DataDepth<_Tp*> { public: enum { value = CV_USRTYPE1, fmt=(int)'r' }; };

template<typename _Tp, int cn> class CV_EXPORTS Vec
{
public:
    typedef _Tp value_type;
    enum { depth = DataDepth<_Tp>::value, channels = cn, type = CV_MAKETYPE(depth, channels) };
    
    Vec();
    Vec(_Tp v0);
    Vec(_Tp v0, _Tp v1);
    Vec(_Tp v0, _Tp v1, _Tp v2);
    Vec(_Tp v0, _Tp v1, _Tp v2, _Tp v3);
    Vec(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4);
    Vec(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5);
    Vec(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5, _Tp v6);
    Vec(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5, _Tp v6, _Tp v7);
    Vec(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5, _Tp v6, _Tp v7, _Tp v8);
    Vec(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5, _Tp v6, _Tp v7, _Tp v8, _Tp v9);
    Vec(const Vec<_Tp, cn>& v);
    static Vec all(_Tp alpha);
    _Tp dot(const Vec& v) const;
    double ddot(const Vec& v) const;
    Vec cross(const Vec& v) const;
    template<typename T2> operator Vec<T2, cn>() const;
    operator CvScalar() const;
    _Tp operator [](int i) const;
    _Tp& operator[](int i);

    _Tp val[cn];
};

typedef Vec<uchar, 2> Vec2b;
typedef Vec<uchar, 3> Vec3b;
typedef Vec<uchar, 4> Vec4b;

typedef Vec<short, 2> Vec2s;
typedef Vec<short, 3> Vec3s;
typedef Vec<short, 4> Vec4s;

typedef Vec<int, 2> Vec2i;
typedef Vec<int, 3> Vec3i;
typedef Vec<int, 4> Vec4i;

typedef Vec<float, 2> Vec2f;
typedef Vec<float, 3> Vec3f;
typedef Vec<float, 4> Vec4f;
typedef Vec<float, 6> Vec6f;

typedef Vec<double, 2> Vec2d;
typedef Vec<double, 3> Vec3d;
typedef Vec<double, 4> Vec4d;
typedef Vec<double, 6> Vec6d;

//////////////////////////////// Complex //////////////////////////////

template<typename _Tp> class CV_EXPORTS Complex
{
public:
    Complex();
    Complex( _Tp _re, _Tp _im=0 );
    Complex( const std::complex<_Tp>& c );
    template<typename T2> operator Complex<T2>() const;
    Complex conj() const;
    operator std::complex<_Tp>() const;

    _Tp re, im;
};

typedef Complex<float> Complexf;
typedef Complex<double> Complexd;

//////////////////////////////// Point_ ////////////////////////////////

template<typename _Tp> class CV_EXPORTS Point_
{
public:
    typedef _Tp value_type;
    
    Point_();
    Point_(_Tp _x, _Tp _y);
    Point_(const Point_& pt);
    Point_(const CvPoint& pt);
    Point_(const CvPoint2D32f& pt);
    Point_(const Size_<_Tp>& sz);
    Point_& operator = (const Point_& pt);
    template<typename _Tp2> operator Point_<_Tp2>() const;
    operator CvPoint() const;
    operator CvPoint2D32f() const;

    _Tp dot(const Point_& pt) const;
    double ddot(const Point_& pt) const;
    bool inside(const Rect_<_Tp>& r) const;
    
    _Tp x, y;
};

template<typename _Tp> class CV_EXPORTS Point3_
{
public:
    typedef _Tp value_type;
    
    Point3_();
    Point3_(_Tp _x, _Tp _y, _Tp _z);
    Point3_(const Point3_& pt);
	explicit Point3_(const Point_<_Tp>& pt);
    Point3_(const CvPoint3D32f& pt);
    Point3_(const Vec<_Tp, 3>& t);
    Point3_& operator = (const Point3_& pt);
    template<typename _Tp2> operator Point3_<_Tp2>() const;
    operator CvPoint3D32f() const;

    _Tp dot(const Point3_& pt) const;
    double ddot(const Point3_& pt) const;
    
    _Tp x, y, z;
};

//////////////////////////////// Size_ ////////////////////////////////

template<typename _Tp> class CV_EXPORTS Size_
{
public:
    typedef _Tp value_type;
    
    Size_();
    Size_(_Tp _width, _Tp _height);
    Size_(const Size_& sz);
    Size_(const CvSize& sz);
    Size_(const CvSize2D32f& sz);
    Size_(const Point_<_Tp>& pt);
    Size_& operator = (const Size_& sz);
    _Tp area() const;

    operator Size_<int>() const;
    operator Size_<float>() const;
    operator Size_<double>() const;
    operator CvSize() const;
    operator CvSize2D32f() const;

    _Tp width, height;
};

//////////////////////////////// Rect_ ////////////////////////////////

template<typename _Tp> class CV_EXPORTS Rect_
{
public:
    typedef _Tp value_type;
    
    Rect_();
    Rect_(_Tp _x, _Tp _y, _Tp _width, _Tp _height);
    Rect_(const Rect_& r);
    Rect_(const CvRect& r);
    Rect_(const Point_<_Tp>& org, const Size_<_Tp>& sz);
    Rect_(const Point_<_Tp>& pt1, const Point_<_Tp>& pt2);
    Rect_& operator = ( const Rect_& r );
    Point_<_Tp> tl() const;
    Point_<_Tp> br() const;
    
    Size_<_Tp> size() const;
    _Tp area() const;

    operator Rect_<int>() const;
    operator Rect_<float>() const;
    operator Rect_<double>() const;
    operator CvRect() const;

    bool contains(const Point_<_Tp>& pt) const;

    _Tp x, y, width, height;
};

typedef Point_<int> Point2i;
typedef Point2i Point;
typedef Size_<int> Size2i;
typedef Size2i Size;
typedef Rect_<int> Rect;
typedef Point_<float> Point2f;
typedef Point_<double> Point2d;
typedef Size_<float> Size2f;
typedef Point3_<int> Point3i;
typedef Point3_<float> Point3f;
typedef Point3_<double> Point3d;

class CV_EXPORTS RotatedRect
{
public:
    RotatedRect();
    RotatedRect(const Point2f& _center, const Size2f& _size, float _angle);
    RotatedRect(const CvBox2D& box);
    Rect boundingRect() const;
    operator CvBox2D() const;
    Point2f center;
    Size2f size;
    float angle;
};

//////////////////////////////// Scalar_ ///////////////////////////////

template<typename _Tp> class CV_EXPORTS Scalar_ : public Vec<_Tp, 4>
{
public:
    Scalar_();
    Scalar_(_Tp v0, _Tp v1, _Tp v2=0, _Tp v3=0);
    Scalar_(const CvScalar& s);
    Scalar_(_Tp v0);
    static Scalar_<_Tp> all(_Tp v0);
    operator CvScalar() const;

    template<typename T2> operator Scalar_<T2>() const;

    Scalar_<_Tp> mul(const Scalar_<_Tp>& t, double scale=1 ) const;
    template<typename T2> void convertTo(T2* buf, int channels, int unroll_to=0) const;
};

typedef Scalar_<double> Scalar;

//////////////////////////////// Range /////////////////////////////////

class CV_EXPORTS Range
{
public:
    Range();
    Range(int _start, int _end);
    Range(const CvSlice& slice);
    int size() const;
    bool empty() const;
    static Range all();
    operator CvSlice() const;

    int start, end;
};

/////////////////////////////// DataType ////////////////////////////////

template<typename _Tp> class DataType
{
public:
    typedef _Tp value_type;
    typedef value_type work_type;
    typedef value_type channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};

template<> class DataType<bool>
{
public:
    typedef bool value_type;
    typedef int work_type;
    typedef value_type channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};

template<> class DataType<uchar>
{
public:
    typedef uchar value_type;
    typedef int work_type;
    typedef value_type channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};

template<> class DataType<schar>
{
public:
    typedef schar value_type;
    typedef int work_type;
    typedef value_type channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};

template<> class DataType<ushort>
{
public:
    typedef ushort value_type;
    typedef int work_type;
    typedef value_type channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};

template<> class DataType<short>
{
public:
    typedef short value_type;
    typedef int work_type;
    typedef value_type channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};

template<> class DataType<int>
{
public:
    typedef int value_type;
    typedef value_type work_type;
    typedef value_type channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};

template<> class DataType<float>
{
public:
    typedef float value_type;
    typedef value_type work_type;
    typedef value_type channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};

template<> class DataType<double>
{
public:
    typedef double value_type;
    typedef value_type work_type;
    typedef value_type channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};

template<typename _Tp, int cn> class DataType<Vec<_Tp, cn> >
{
public:
    typedef Vec<_Tp, cn> value_type;
    typedef Vec<typename DataType<_Tp>::work_type, cn> work_type;
    typedef _Tp channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = cn,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};

template<typename _Tp> class DataType<std::complex<_Tp> >
{
public:
    typedef std::complex<_Tp> value_type;
    typedef value_type work_type;
    typedef _Tp channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = 2,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};

template<typename _Tp> class DataType<Complex<_Tp> >
{
public:
    typedef Complex<_Tp> value_type;
    typedef value_type work_type;
    typedef _Tp channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = 2,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};

template<typename _Tp> class DataType<Point_<_Tp> >
{
public:
    typedef Point_<_Tp> value_type;
    typedef Point_<typename DataType<_Tp>::work_type> work_type;
    typedef _Tp channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = 2,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};

template<typename _Tp> class DataType<Point3_<_Tp> >
{
public:
    typedef Point3_<_Tp> value_type;
    typedef Point3_<typename DataType<_Tp>::work_type> work_type;
    typedef _Tp channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = 3,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};

template<typename _Tp> class DataType<Size_<_Tp> >
{
public:
    typedef Size_<_Tp> value_type;
    typedef Size_<typename DataType<_Tp>::work_type> work_type;
    typedef _Tp channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = 2,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};

template<typename _Tp> class DataType<Rect_<_Tp> >
{
public:
    typedef Rect_<_Tp> value_type;
    typedef Rect_<typename DataType<_Tp>::work_type> work_type;
    typedef _Tp channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = 4,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};

template<typename _Tp> class DataType<Scalar_<_Tp> >
{
public:
    typedef Scalar_<_Tp> value_type;
    typedef Scalar_<typename DataType<_Tp>::work_type> work_type;
    typedef _Tp channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = 4,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};

template<> class DataType<Range>
{
public:
    typedef Range value_type;
    typedef value_type work_type;
    typedef int channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = 2,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};


//////////////////////////////// Vector ////////////////////////////////

// template vector class. It is similar to STL's vector,
// with a few important differences:
//   1) it can be created on top of user-allocated data w/o copying it
//   2) Vector b = a means copying the header,
//      not the underlying data (use clone() to make a deep copy)
template <typename _Tp> class CV_EXPORTS Vector
{
public:
    typedef _Tp value_type;
    typedef _Tp* iterator;
    typedef const _Tp* const_iterator;
    typedef _Tp& reference;
    typedef const _Tp& const_reference;

    struct CV_EXPORTS Hdr
    {
        Hdr() : data(0), datastart(0), refcount(0), size(0), capacity(0) {};
        _Tp* data;
        _Tp* datastart;
        int* refcount;
        size_t size;
        size_t capacity;
    };

    Vector();
    Vector(size_t _size);
    Vector(size_t _size, const _Tp& val);
    Vector(_Tp* _data, size_t _size, bool _copyData=false);
    template<int n> Vector(const Vec<_Tp, n>& vec);
    Vector(const std::vector<_Tp>& vec, bool _copyData=false);
    Vector(const Vector& d);
    Vector(const Vector& d, const Range& r);

    Vector<_Tp>& operator = (const Vector& d);
    ~Vector();
    Vector clone() const;
    void copyTo(Vector<_Tp>& vec) const;
    void copyTo(std::vector<_Tp>& vec) const;
    operator CvMat() const;
    
    _Tp& operator [] (size_t i);
    const _Tp& operator [] (size_t i) const;
    Vector operator() (const Range& r) const;
    _Tp& back();
    const _Tp& back() const;
    _Tp& front();
    const _Tp& front() const;

    _Tp* begin();
    _Tp* end();
    const _Tp* begin() const;
    const _Tp* end() const;

    void addref();
    void release();
    void set(_Tp* _data, size_t _size, bool _copyData=false);

    void reserve(size_t newCapacity);
    void resize(size_t newSize);
    Vector<_Tp>& push_back(const _Tp& elem);
    Vector<_Tp>& pop_back();
    size_t size() const;
    size_t capacity() const;
    bool empty() const;
    void clear();
    int type() const;

protected:
    Hdr hdr;
};

//////////////////// Generic ref-cointing pointer class for C/C++ objects ////////////////////////

template<typename _Tp> class CV_EXPORTS Ptr
{
public:
    Ptr();
    Ptr(_Tp* _obj);
    ~Ptr();
    Ptr(const Ptr& ptr);
    Ptr& operator = (const Ptr& ptr);
    void addref();
    void release();
    void delete_obj();
    bool empty() const;

    _Tp* operator -> ();
    const _Tp* operator -> () const;

    operator _Tp* ();
    operator const _Tp*() const;
protected:
    _Tp* obj;
    int* refcount;
};

//////////////////////////////// Mat ////////////////////////////////

class Mat;
template<typename M> class CV_EXPORTS MatExpr_Base_;
typedef MatExpr_Base_<Mat> MatExpr_Base;
template<typename E, typename M> class MatExpr_;
template<typename A1, typename M, typename Op> class MatExpr_Op1_;
template<typename A1, typename A2, typename M, typename Op> class MatExpr_Op2_;
template<typename A1, typename A2, typename A3, typename M, typename Op> class MatExpr_Op3_;
template<typename A1, typename A2, typename A3, typename A4,
        typename M, typename Op> class MatExpr_Op4_;
template<typename A1, typename A2, typename A3, typename A4,
        typename A5, typename M, typename Op> class MatExpr_Op5_;
template<typename M> class CV_EXPORTS MatOp_DivRS_;
template<typename M> class CV_EXPORTS MatOp_Inv_;
template<typename M> class CV_EXPORTS MatOp_MulDiv_;
template<typename M> class CV_EXPORTS MatOp_Repeat_;
template<typename M> class CV_EXPORTS MatOp_Set_;
template<typename M> class CV_EXPORTS MatOp_Scale_;
template<typename M> class CV_EXPORTS MatOp_T_;

typedef MatExpr_<MatExpr_Op4_<Size, int, Scalar,
    int, Mat, MatOp_Set_<Mat> >, Mat> MatExpr_Initializer;

template<typename _Tp> class MatIterator_;
template<typename _Tp> class MatConstIterator_;

enum { MAGIC_MASK=0xFFFF0000, TYPE_MASK=0x00000FFF, DEPTH_MASK=7 };

static inline size_t getElemSize(int type) { return CV_ELEM_SIZE(type); }

// matrix decomposition types
enum { DECOMP_LU=0, DECOMP_SVD=1, DECOMP_EIG=2, DECOMP_CHOLESKY=3, DECOMP_QR=4, DECOMP_NORMAL=16 };
enum { NORM_INF=1, NORM_L1=2, NORM_L2=4, NORM_TYPE_MASK=7, NORM_RELATIVE=8};
enum { CMP_EQ=0, CMP_GT=1, CMP_GE=2, CMP_LT=3, CMP_LE=4, CMP_NE=5 };
enum { GEMM_1_T=1, GEMM_2_T=2, GEMM_3_T=4 };
enum { DFT_INVERSE=1, DFT_SCALE=2, DFT_ROWS=4, DFT_COMPLEX_OUTPUT=16, DFT_REAL_OUTPUT=32,
    DCT_INVERSE = DFT_INVERSE, DCT_ROWS=DFT_ROWS };

class CV_EXPORTS Mat
{
public:
    Mat();
    Mat(int _rows, int _cols, int _type);
    Mat(int _rows, int _cols, int _type, const Scalar& _s);
    Mat(Size _size, int _type);
    Mat(const Mat& m);
    Mat(int _rows, int _cols, int _type, void* _data, size_t _step=AUTO_STEP);
    Mat(Size _size, int _type, void* _data, size_t _step=AUTO_STEP);
    Mat(const Mat& m, const Range& rowRange, const Range& colRange);
    Mat(const Mat& m, const Rect& roi);
    Mat(const CvMat* m, bool copyData=false);
    Mat(const IplImage* img, bool copyData=false);
    Mat( const MatExpr_Base& expr );
    ~Mat();
    Mat& operator = (const Mat& m);
    Mat& operator = (const MatExpr_Base& expr);

    operator MatExpr_<Mat, Mat>() const;

    Mat row(int y) const;
    Mat col(int x) const;
    Mat rowRange(int startrow, int endrow) const;
    Mat rowRange(const Range& r) const;
    Mat colRange(int startcol, int endcol) const;
    Mat colRange(const Range& r) const;
    Mat diag(int d=0) const;
    static Mat diag(const Mat& d);

    Mat clone() const;
    void copyTo( Mat& m ) const;
    void copyTo( Mat& m, const Mat& mask ) const;
    void convertTo( Mat& m, int rtype, double alpha=1, double beta=0 ) const;

    void assignTo( Mat& m, int type=-1 ) const;
    Mat& operator = (const Scalar& s);
    Mat& setTo(const Scalar& s, const Mat& mask=Mat());
    Mat reshape(int _cn, int _rows=0) const;

    MatExpr_<MatExpr_Op2_<Mat, double, Mat, MatOp_T_<Mat> >, Mat>
    t() const;
    MatExpr_<MatExpr_Op2_<Mat, int, Mat, MatOp_Inv_<Mat> >, Mat>
        inv(int method=DECOMP_LU) const;
    MatExpr_<MatExpr_Op4_<Mat, Mat, double, char, Mat, MatOp_MulDiv_<Mat> >, Mat>
    mul(const Mat& m, double scale=1) const;
    MatExpr_<MatExpr_Op4_<Mat, Mat, double, char, Mat, MatOp_MulDiv_<Mat> >, Mat>
    mul(const MatExpr_<MatExpr_Op2_<Mat, double, Mat, MatOp_Scale_<Mat> >, Mat>& m, double scale=1) const;
    MatExpr_<MatExpr_Op4_<Mat, Mat, double, char, Mat, MatOp_MulDiv_<Mat> >, Mat>    
    mul(const MatExpr_<MatExpr_Op2_<Mat, double, Mat, MatOp_DivRS_<Mat> >, Mat>& m, double scale=1) const;

    Mat cross(const Mat& m) const;
    double dot(const Mat& m) const;

    static MatExpr_Initializer zeros(int rows, int cols, int type);
    static MatExpr_Initializer zeros(Size size, int type);
    static MatExpr_Initializer ones(int rows, int cols, int type);
    static MatExpr_Initializer ones(Size size, int type);
    static MatExpr_Initializer eye(int rows, int cols, int type);
    static MatExpr_Initializer eye(Size size, int type);

    void create(int _rows, int _cols, int _type);
    void create(Size _size, int _type);
    void addref();
    void release();

    void locateROI( Size& wholeSize, Point& ofs ) const;
    Mat& adjustROI( int dtop, int dbottom, int dleft, int dright );
    Mat operator()( Range rowRange, Range colRange ) const;
    Mat operator()( const Rect& roi ) const;

    operator CvMat() const;
    operator IplImage() const;
    bool isContinuous() const;
    size_t elemSize() const;
    size_t elemSize1() const;
    int type() const;
    int depth() const;
    int channels() const;
    size_t step1() const;
    Size size() const;
    bool empty() const;

    uchar* ptr(int y=0);
    const uchar* ptr(int y=0) const;

    template<typename _Tp> _Tp* ptr(int y=0);
    template<typename _Tp> const _Tp* ptr(int y=0) const;

    enum { MAGIC_VAL=0x42FF0000, AUTO_STEP=0, CONTINUOUS_FLAG=CV_MAT_CONT_FLAG };

    int flags;
    int rows, cols;
    size_t step;
    uchar* data;

    int* refcount;
    uchar* datastart;
    uchar* dataend;
};


// Multiply-with-Carry RNG
class CV_EXPORTS RNG
{
public:
    enum { A=4164903690U, UNIFORM=0, NORMAL=1 };

    RNG();
    RNG(uint64 _state);
    unsigned next();

    operator uchar();
    operator schar();
    operator ushort();
    operator short();
    operator unsigned();
    operator int();
    operator float();
    operator double();
    int uniform(int a, int b);
    float uniform(float a, float b);
    double uniform(double a, double b);
    void fill( Mat& mat, int distType, const Scalar& a, const Scalar& b );

    uint64 state;
};

class CV_EXPORTS TermCriteria
{
public:
    enum { COUNT=1, MAX_ITER=COUNT, EPS=2 };

    TermCriteria();
    TermCriteria(int _type, int _maxCount, double _epsilon);
    TermCriteria(const CvTermCriteria& criteria);
    operator CvTermCriteria() const;
    
    int type;
    int maxCount;
    double epsilon;
};

CV_EXPORTS Mat cvarrToMat(const CvArr* arr, bool copyData=false, bool allowND=true);
CV_EXPORTS Mat extractImageCOI(const CvArr* arr);

CV_EXPORTS void add(const Mat& a, const Mat& b, Mat& c, const Mat& mask);
CV_EXPORTS void subtract(const Mat& a, const Mat& b, Mat& c, const Mat& mask);
CV_EXPORTS void add(const Mat& a, const Mat& b, Mat& c);
CV_EXPORTS void subtract(const Mat& a, const Mat& b, Mat& c);
CV_EXPORTS void add(const Mat& a, const Scalar& s, Mat& c, const Mat& mask=Mat());
CV_EXPORTS void subtract(const Mat& a, const Scalar& s, Mat& c, const Mat& mask=Mat());

CV_EXPORTS void multiply(const Mat& a, const Mat& b, Mat& c, double scale=1);
CV_EXPORTS void divide(const Mat& a, const Mat& b, Mat& c, double scale=1);
CV_EXPORTS void divide(double scale, const Mat& b, Mat& c);

CV_EXPORTS void subtract(const Scalar& s, const Mat& a, Mat& c, const Mat& mask=Mat());
CV_EXPORTS void scaleAdd(const Mat& a, double alpha, const Mat& b, Mat& c);
CV_EXPORTS void addWeighted(const Mat& a, double alpha, const Mat& b,
                            double beta, double gamma, Mat& c);
CV_EXPORTS void convertScaleAbs(const Mat& a, Mat& c, double alpha=1, double beta=0);
CV_EXPORTS void LUT(const Mat& a, const Mat& lut, Mat& b);

CV_EXPORTS Scalar sum(const Mat& m);
CV_EXPORTS int countNonZero( const Mat& m );

CV_EXPORTS Scalar mean(const Mat& m);
CV_EXPORTS Scalar mean(const Mat& m, const Mat& mask);
CV_EXPORTS void meanStdDev(const Mat& m, Scalar& mean, Scalar& stddev, const Mat& mask=Mat());
CV_EXPORTS double norm(const Mat& a, int normType=NORM_L2);
CV_EXPORTS double norm(const Mat& a, const Mat& b, int normType=NORM_L2);
CV_EXPORTS double norm(const Mat& a, int normType, const Mat& mask);
CV_EXPORTS double norm(const Mat& a, const Mat& b,
                       int normType, const Mat& mask);
CV_EXPORTS void normalize( const Mat& a, Mat& b, double alpha=1, double beta=0,
                          int norm_type=NORM_L2, int rtype=-1, const Mat& mask=Mat());

CV_EXPORTS void minMaxLoc(const Mat& a, double* minVal,
                          double* maxVal=0, Point* minLoc=0,
                          Point* maxLoc=0, const Mat& mask=Mat());
CV_EXPORTS void reduce(const Mat& m, Mat& dst, int dim, int rtype, int dtype=-1);
CV_EXPORTS void merge(const Vector<Mat>& mv, Mat& dst);
CV_EXPORTS void split(const Mat& m, Vector<Mat>& mv);
CV_EXPORTS void mixChannels(const Vector<Mat>& src, Vector<Mat>& dst,
                            const Vector<int>& fromTo);
CV_EXPORTS void flip(const Mat& a, Mat& b, int flipCode);

CV_EXPORTS void repeat(const Mat& a, int ny, int nx, Mat& b);
static inline Mat repeat(const Mat& src, int ny, int nx)
{
    if( nx == 1 && ny == 1 ) return src;
    Mat dst; repeat(src, ny, nx, dst); return dst;
}

CV_EXPORTS void bitwise_and(const Mat& a, const Mat& b, Mat& c, const Mat& mask=Mat());
CV_EXPORTS void bitwise_or(const Mat& a, const Mat& b, Mat& c, const Mat& mask=Mat());
CV_EXPORTS void bitwise_xor(const Mat& a, const Mat& b, Mat& c, const Mat& mask=Mat());
CV_EXPORTS void bitwise_and(const Mat& a, const Scalar& s, Mat& c, const Mat& mask=Mat());
CV_EXPORTS void bitwise_or(const Mat& a, const Scalar& s, Mat& c, const Mat& mask=Mat());
CV_EXPORTS void bitwise_xor(const Mat& a, const Scalar& s, Mat& c, const Mat& mask=Mat());
CV_EXPORTS void bitwise_not(const Mat& a, Mat& c);
CV_EXPORTS void absdiff(const Mat& a, const Mat& b, Mat& c);
CV_EXPORTS void absdiff(const Mat& a, const Scalar& s, Mat& c);
CV_EXPORTS void inRange(const Mat& src, const Mat& lowerb,
                        const Mat& upperb, Mat& dst);
CV_EXPORTS void inRange(const Mat& src, const Scalar& lowerb,
                        const Scalar& upperb, Mat& dst);
CV_EXPORTS void compare(const Mat& a, const Mat& b, Mat& c, int cmpop);
CV_EXPORTS void compare(const Mat& a, double s, Mat& c, int cmpop);
CV_EXPORTS void min(const Mat& a, const Mat& b, Mat& c);
CV_EXPORTS void min(const Mat& a, double alpha, Mat& c);
CV_EXPORTS void max(const Mat& a, const Mat& b, Mat& c);
CV_EXPORTS void max(const Mat& a, double alpha, Mat& c);

CV_EXPORTS void sqrt(const Mat& a, Mat& b);
CV_EXPORTS void pow(const Mat& a, double power, Mat& b);
CV_EXPORTS void exp(const Mat& a, Mat& b);
CV_EXPORTS void log(const Mat& a, Mat& b);
CV_EXPORTS float cubeRoot(float val);
CV_EXPORTS float fastAtan2(float y, float x);
CV_EXPORTS void polarToCart(const Mat& magnitude, const Mat& angle,
                            Mat& x, Mat& y, bool angleInDegrees=false);
CV_EXPORTS void cartToPolar(const Mat& x, const Mat& y,
                            Mat& magnitude, Mat& angle,
                            bool angleInDegrees=false);
CV_EXPORTS void phase(const Mat& x, const Mat& y, Mat& angle,
                            bool angleInDegrees=false);
CV_EXPORTS void magnitude(const Mat& x, const Mat& y, Mat& magnitude);
CV_EXPORTS bool checkRange(const Mat& a, bool quiet=true, Point* pt=0,
                           double minVal=-DBL_MAX, double maxVal=DBL_MAX);

CV_EXPORTS void gemm(const Mat& a, const Mat& b, double alpha,
                     const Mat& c, double gamma, Mat& d, int flags=0);
CV_EXPORTS void mulTransposed( const Mat& a, Mat& c, bool aTa,
                               const Mat& delta=Mat(),
                               double scale=1, int rtype=-1 );
CV_EXPORTS void transpose(const Mat& a, Mat& b);
CV_EXPORTS void transform(const Mat& src, Mat& dst, const Mat& m );
CV_EXPORTS void perspectiveTransform(const Mat& src, Mat& dst, const Mat& m );

CV_EXPORTS void completeSymm(Mat& a, bool lowerToUpper=false);
CV_EXPORTS void setIdentity(Mat& c, const Scalar& s=Scalar(1));
CV_EXPORTS double determinant(const Mat& m);
CV_EXPORTS Scalar trace(const Mat& m);
CV_EXPORTS double invert(const Mat& a, Mat& c, int flags=DECOMP_LU);
CV_EXPORTS bool solve(const Mat& a, const Mat& b, Mat& x, int flags=DECOMP_LU);
CV_EXPORTS void sort(const Mat& a, Mat& b, int flags);
CV_EXPORTS void sortIdx(const Mat& a, Mat& b, int flags);
CV_EXPORTS void solveCubic(const Mat& coeffs, Mat& roots);
CV_EXPORTS void solvePoly(const Mat& coeffs, Mat& roots, int maxIters=20, int fig=100);
CV_EXPORTS bool eigen(const Mat& a, Mat& eigenvalues);
CV_EXPORTS bool eigen(const Mat& a, Mat& eigenvalues, Mat& eigenvectors);

CV_EXPORTS void calcCovariation( const Vector<Mat>& data, Mat& covar, Mat& mean,
                                 int flags, int ctype=CV_64F);
CV_EXPORTS void calcCovariation( const Mat& data, Mat& covar, Mat& mean,
                                 int flags, int ctype=CV_64F);

class CV_EXPORTS PCA
{
public:
    PCA();
    PCA(const Mat& data, const Mat& mean, int flags, int maxComponents=0);
    PCA& operator()(const Mat& data, const Mat& mean, int flags, int maxComponents=0);
    Mat project(const Mat& vec) const;
    void project(const Mat& vec, Mat& result) const;
    Mat backProject(const Mat& vec) const;
    void backProject(const Mat& vec, Mat& result) const;

    Mat eigenvectors;
    Mat eigenvalues;
    Mat mean;
};

class CV_EXPORTS SVD
{
public:
    enum { MODIFY_A=1, NO_UV=2, FULL_UV=4 };
    SVD();
    SVD( const Mat& m, int flags=0 );
    SVD& operator ()( const Mat& m, int flags=0 );

    static void solveZ( const Mat& m, Mat& dst );
    void backSubst( const Mat& rhs, Mat& dst ) const;

    Mat u, w, vt;
};

CV_EXPORTS double mahalanobis(const Mat& v1, const Mat& v2, const Mat& icovar);
static inline double mahalonobis(const Mat& v1, const Mat& v2, const Mat& icovar)
{ return mahalanobis(v1, v2, icovar); }

CV_EXPORTS void dft(const Mat& src, Mat& dst, int flags=0, int nonzeroRows=0);
CV_EXPORTS void idft(const Mat& src, Mat& dst, int flags=0, int nonzeroRows=0);
CV_EXPORTS void dct(const Mat& src, Mat& dst, int flags=0);
CV_EXPORTS void idct(const Mat& src, Mat& dst, int flags=0);
CV_EXPORTS void mulSpectrums(const Mat& a, const Mat& b, Mat& c,
                             int flags, bool conjB=false);
CV_EXPORTS int getOptimalDFTSize(int vecsize);

enum { KMEANS_CENTERS_RANDOM=0, KMEANS_CENTERS_SPP=2, KMEANS_USE_INITIAL_LABELS=1 };
CV_EXPORTS int kmeans( const Mat& samples, int K,
                       Mat& labels, Mat& centers,
                       TermCriteria crit, int attempts=1,
                       int flags=KMEANS_CENTERS_SPP,
                       double* compactness=0);

CV_EXPORTS void seqToVector( const CvSeq* ptseq, Vector<Point>& pts );

CV_EXPORTS RNG& theRNG();
static inline int randi() { return (int)theRNG(); }
static inline unsigned randu() { return (unsigned)theRNG(); }
static inline float randf() { return (float)theRNG(); }
static inline double randd() { return (double)theRNG(); }
static inline void randu(Mat& dst, const Scalar& low, const Scalar& high)
{ theRNG().fill(dst, RNG::UNIFORM, low, high); }
static inline void randn(Mat& dst, const Scalar& mean, const Scalar& stddev)
{ theRNG().fill(dst, RNG::NORMAL, mean, stddev); }
CV_EXPORTS void randShuffle(Mat& dst, RNG& rng, double iterFactor=1.);
static inline void randShuffle(Mat& dst, double iterFactor=1.)
{ randShuffle(dst, theRNG(), iterFactor); }

CV_EXPORTS void line(Mat& img, Point pt1, Point pt2, const Scalar& color,
                     int thickness=1, int lineType=8, int shift=0);

CV_EXPORTS void rectangle(Mat& img, Point pt1, Point pt2,
                          const Scalar& color, int thickness=1,
                          int lineType=8, int shift=0);

CV_EXPORTS void circle(Mat& img, Point center, int radius,
                       const Scalar& color, int thickness=1,
                       int lineType=8, int shift=0);

CV_EXPORTS void ellipse(Mat& img, Point center, Size axes,
                        double angle, double startAngle, double endAngle,
                        const Scalar& color, int thickness=1,
                        int lineType=8, int shift=0);

CV_EXPORTS void ellipse(Mat& img, const RotatedRect& box, const Scalar& color,
                        int thickness=1, int lineType=8);

CV_EXPORTS void fillConvexPoly(Mat& img, const Vector<Point>& pts,
                               const Scalar& color, int lineType=8,
                               int shift=0);

CV_EXPORTS void fillPoly(Mat& img, const Vector<Vector<Point> >& pts,
                         const Scalar& color, int lineType=8, int shift=0,
                         Point offset=Point() );

CV_EXPORTS void polylines(Mat& img, const Vector<Vector<Point> >& pts, bool isClosed,
                          const Scalar& color, int thickness=1, int lineType=8, int shift=0 );

CV_EXPORTS bool clipLine(Size imgSize, Point& pt1, Point& pt2);

class CV_EXPORTS LineIterator
{
public:
    LineIterator(const Mat& img, Point pt1, Point pt2,
                 int connectivity=8, bool leftToRight=false);
    uchar* operator *();
    LineIterator& operator ++();
    LineIterator operator ++(int);

    uchar* ptr;
    int err, count;
    int minusDelta, plusDelta;
    int minusStep, plusStep;
};

CV_EXPORTS void ellipse2Poly( Point center, Size axes, int angle,
                              int arcStart, int arcEnd, int delta, Vector<Point>& pts );

enum
{
    FONT_HERSHEY_SIMPLEX = 0,
    FONT_HERSHEY_PLAIN = 1,
    FONT_HERSHEY_DUPLEX = 2,
    FONT_HERSHEY_COMPLEX = 3,
    FONT_HERSHEY_TRIPLEX = 4,
    FONT_HERSHEY_COMPLEX_SMALL = 5,
    FONT_HERSHEY_SCRIPT_SIMPLEX = 6,
    FONT_HERSHEY_SCRIPT_COMPLEX = 7,
    FONT_ITALIC = 16
};

CV_EXPORTS void putText( Mat& img, const String& text, Point org,
                         int fontFace, double fontScale, Scalar color,
                         int thickness=1, int linetype=8,
                         bool bottomLeftOrigin=false );

CV_EXPORTS Size getTextSize(const String& text, int fontFace,
                            double fontScale, int thickness,
                            int* baseLine);

///////////////////////////////// Mat_<_Tp> ////////////////////////////////////

template<typename _Tp> class CV_EXPORTS Mat_ : public Mat
{
public:
    typedef _Tp value_type;
    typedef typename DataType<_Tp>::channel_type channel_type;
    typedef MatIterator_<_Tp> iterator;
    typedef MatConstIterator_<_Tp> const_iterator;
    
    Mat_();
    Mat_(int _rows, int _cols);
    Mat_(int _rows, int _cols, const _Tp& value);
    explicit Mat_(Size _size);
    Mat_(Size _size, const _Tp& value);
    Mat_(const Mat& m);
    Mat_(const Mat_& m);
    Mat_(int _rows, int _cols, _Tp* _data, size_t _step=AUTO_STEP);
    Mat_(const Mat_& m, const Range& rowRange, const Range& colRange);
    Mat_(const Mat_& m, const Rect& roi);
    Mat_(const MatExpr_Base& expr);
    template<int n> explicit Mat_(const Vec<_Tp, n>& vec);
    Mat_(const Vector<_Tp>& vec);

    Mat_& operator = (const Mat& m);
    Mat_& operator = (const Mat_& m);
    Mat_& operator = (const _Tp& s);

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;

    void create(int _rows, int _cols);
    void create(Size _size);
    Mat_ cross(const Mat_& m) const;
    Mat_& operator = (const MatExpr_Base& expr);
    template<typename T2> operator Mat_<T2>() const;
    Mat_ row(int y) const;
    Mat_ col(int x) const;
    Mat_ diag(int d=0) const;
    Mat_ clone() const;

    MatExpr_<MatExpr_Op2_<Mat_, double, Mat_, MatOp_T_<Mat> >, Mat_> t() const;
    MatExpr_<MatExpr_Op2_<Mat_, int, Mat_, MatOp_Inv_<Mat> >, Mat_> inv(int method=DECOMP_LU) const;

    MatExpr_<MatExpr_Op4_<Mat_, Mat_, double, char, Mat_, MatOp_MulDiv_<Mat> >, Mat_>
    mul(const Mat_& m, double scale=1) const;
    MatExpr_<MatExpr_Op4_<Mat_, Mat_, double, char, Mat_, MatOp_MulDiv_<Mat> >, Mat_>
    mul(const MatExpr_<MatExpr_Op2_<Mat_, double, Mat_,
        MatOp_Scale_<Mat> >, Mat_>& m, double scale=1) const;
    MatExpr_<MatExpr_Op4_<Mat_, Mat_, double, char, Mat_, MatOp_MulDiv_<Mat> >, Mat_>    
    mul(const MatExpr_<MatExpr_Op2_<Mat_, double, Mat_,
        MatOp_DivRS_<Mat> >, Mat_>& m, double scale=1) const;

    size_t elemSize() const;
    size_t elemSize1() const;
    int type() const;
    int depth() const;
    int channels() const;
    size_t stepT() const;
    size_t step1() const;

    static MatExpr_Initializer zeros(int rows, int cols);
    static MatExpr_Initializer zeros(Size size);
    static MatExpr_Initializer ones(int rows, int cols);
    static MatExpr_Initializer ones(Size size);
    static MatExpr_Initializer eye(int rows, int cols);
    static MatExpr_Initializer eye(Size size);

    Mat_ reshape(int _rows) const;
    Mat_& adjustROI( int dtop, int dbottom, int dleft, int dright );
    Mat_ operator()( const Range& rowRange, const Range& colRange ) const;
    Mat_ operator()( const Rect& roi ) const;

    _Tp* operator [](int y);
    const _Tp* operator [](int y) const;

    _Tp& operator ()(int row, int col);
    const _Tp& operator ()(int row, int col) const;

    operator MatExpr_<Mat_, Mat_>() const;
    operator Vector<_Tp>() const;
};

//////////// Iterators & Comma initializers //////////////////

template<typename _Tp>
class CV_EXPORTS MatConstIterator_
{
public:
    typedef _Tp value_type;
    typedef int difference_type;

    MatConstIterator_();
    MatConstIterator_(const Mat_<_Tp>* _m);
    MatConstIterator_(const Mat_<_Tp>* _m, int _row, int _col=0);
    MatConstIterator_(const Mat_<_Tp>* _m, Point _pt);
    MatConstIterator_(const MatConstIterator_& it);

    MatConstIterator_& operator = (const MatConstIterator_& it );
    _Tp operator *() const;
    _Tp operator [](int i) const;
    
    MatConstIterator_& operator += (int ofs);
    MatConstIterator_& operator -= (int ofs);
    MatConstIterator_& operator --();
    MatConstIterator_ operator --(int);
    MatConstIterator_& operator ++();
    MatConstIterator_ operator ++(int);
    Point pos() const;

protected:
    const Mat_<_Tp>* m;
    _Tp* ptr;
    _Tp* sliceEnd;
};


template<typename _Tp>
class CV_EXPORTS MatIterator_ : public MatConstIterator_<_Tp>
{
public:
    typedef _Tp* pointer;
    typedef _Tp& reference;
    typedef std::random_access_iterator_tag iterator_category;

    MatIterator_();
    MatIterator_(Mat_<_Tp>* _m);
    MatIterator_(Mat_<_Tp>* _m, int _row, int _col=0);
    MatIterator_(const Mat_<_Tp>* _m, Point _pt);
    MatIterator_(const MatIterator_& it);
    MatIterator_& operator = (const MatIterator_<_Tp>& it );

    _Tp& operator *() const;
    _Tp& operator [](int i) const;

    MatIterator_& operator += (int ofs);
    MatIterator_& operator -= (int ofs);
    MatIterator_& operator --();
    MatIterator_ operator --(int);
    MatIterator_& operator ++();
    MatIterator_ operator ++(int);
};

template<typename _Tp> class CV_EXPORTS MatOp_Iter_;

template<typename _Tp> class CV_EXPORTS MatCommaInitializer_ :
    public MatExpr_<MatExpr_Op1_<MatIterator_<_Tp>, Mat_<_Tp>, MatOp_Iter_<_Tp> >, Mat_<_Tp> >
{
public:
    MatCommaInitializer_(Mat_<_Tp>* _m);
    template<typename T2> MatCommaInitializer_<_Tp>& operator , (T2 v);
    operator Mat_<_Tp>() const;
    Mat_<_Tp> operator *() const;
    void assignTo(Mat& m, int type=-1) const;
};

template<typename _Tp> class VectorCommaInitializer_
{
public:
    VectorCommaInitializer_(Vector<_Tp>* _vec);
    template<typename T2> VectorCommaInitializer_<_Tp>& operator , (T2 val);
    operator Vector<_Tp>() const;
    Vector<_Tp> operator *() const;

protected:
    Vector<_Tp>* vec;
    int idx;
};

template<typename _Tp, size_t fixed_size=4096/sizeof(_Tp)+8> class CV_EXPORTS AutoBuffer
{
public:
    typedef _Tp value_type;

    AutoBuffer();
    AutoBuffer(size_t _size);
    ~AutoBuffer();

    void allocate(size_t _size);
    void deallocate();
    operator _Tp* ();
    operator const _Tp* () const;

protected:
    _Tp* ptr;
    size_t size;
    _Tp buf[fixed_size];
};

/////////////////////////// multi-dimensional dense matrix //////////////////////////

class MatND;
class SparseMat;

class CV_EXPORTS MatND
{
public:
    MatND();
    MatND(const Vector<int>& _sizes, int _type);
    MatND(const Vector<int>& _sizes, int _type, const Scalar& _s);
    MatND(const MatND& m);
    MatND(const MatND& m, const Vector<Range>& ranges);
    MatND(const CvMatND* m, bool copyData=false);
    //MatND( const MatExpr_BaseND& expr );
    ~MatND();
    MatND& operator = (const MatND& m);
    //MatND& operator = (const MatExpr_BaseND& expr);

    //operator MatExpr_<MatND, MatND>() const;

    MatND clone() const;
    MatND operator()(const Vector<Range>& ranges) const;

    void copyTo( MatND& m ) const;
    void copyTo( MatND& m, const MatND& mask ) const;
    void convertTo( MatND& m, int rtype, double alpha=1, double beta=0 ) const;

    void assignTo( MatND& m, int type=-1 ) const;
    MatND& operator = (const Scalar& s);
    MatND& setTo(const Scalar& s, const MatND& mask=MatND());
    MatND reshape(int newcn, const Vector<int>& newsz=Vector<int>()) const;

    void create(const Vector<int>& _sizes, int _type);
    void addref();
    void release();

    operator Mat() const;
    operator CvMatND() const;
    bool isContinuous() const;
    size_t elemSize() const;
    size_t elemSize1() const;
    int type() const;
    int depth() const;
    int channels() const;
    size_t step(int i) const;
    size_t step1(int i) const;
    Vector<int> size() const;
    int size(int i) const;

    uchar* ptr(int i0);
    const uchar* ptr(int i0) const;
    uchar* ptr(int i0, int i1);
    const uchar* ptr(int i0, int i1) const;
    uchar* ptr(int i0, int i1, int i2);
    const uchar* ptr(int i0, int i1, int i2) const;
    uchar* ptr(const int* idx);
    const uchar* ptr(const int* idx) const;

    enum { MAGIC_VAL=0x42FE0000, AUTO_STEP=-1,
        CONTINUOUS_FLAG=CV_MAT_CONT_FLAG, MAX_DIM=CV_MAX_DIM };

    int flags;
    int dims;

    int* refcount;
    uchar* data;
    uchar* datastart;
    uchar* dataend;

    struct
    {
        int size;
        size_t step;
    }
    dim[MAX_DIM];
};

class CV_EXPORTS NAryMatNDIterator
{
public:
    NAryMatNDIterator();
    NAryMatNDIterator(const Vector<MatND>& arrays);
    void init(const Vector<MatND>& arrays);

    NAryMatNDIterator& operator ++();
    NAryMatNDIterator operator ++(int);
    
    Vector<MatND> arrays;
    Vector<Mat> planes;

    int nplanes;
protected:
    int iterdepth, idx;
};

CV_EXPORTS void add(const MatND& a, const MatND& b, MatND& c, const MatND& mask);
CV_EXPORTS void subtract(const MatND& a, const MatND& b, MatND& c, const MatND& mask);
CV_EXPORTS void add(const MatND& a, const MatND& b, MatND& c);
CV_EXPORTS void subtract(const MatND& a, const MatND& b, MatND& c);
CV_EXPORTS void add(const MatND& a, const Scalar& s, MatND& c, const MatND& mask=MatND());

CV_EXPORTS void multiply(const MatND& a, const MatND& b, MatND& c, double scale=1);
CV_EXPORTS void divide(const MatND& a, const MatND& b, MatND& c, double scale=1);
CV_EXPORTS void divide(double scale, const MatND& b, MatND& c);

CV_EXPORTS void subtract(const Scalar& s, const MatND& a, MatND& c, const MatND& mask=MatND());
CV_EXPORTS void scaleAdd(const MatND& a, double alpha, const MatND& b, MatND& c);
CV_EXPORTS void addWeighted(const MatND& a, double alpha, const MatND& b,
                            double beta, double gamma, MatND& c);

CV_EXPORTS Scalar sum(const MatND& m);
CV_EXPORTS int countNonZero( const MatND& m );

CV_EXPORTS Scalar mean(const MatND& m);
CV_EXPORTS Scalar mean(const MatND& m, const MatND& mask);
CV_EXPORTS void meanStdDev(const MatND& m, Scalar& mean, Scalar& stddev, const MatND& mask=MatND());
CV_EXPORTS double norm(const MatND& a, int normType=NORM_L2, const MatND& mask=MatND());
CV_EXPORTS double norm(const MatND& a, const MatND& b,
                       int normType=NORM_L2, const MatND& mask=MatND());
CV_EXPORTS void normalize( const MatND& a, MatND& b, double alpha=1, double beta=0,
                           int norm_type=NORM_L2, int rtype=-1, const MatND& mask=MatND());

CV_EXPORTS void minMaxLoc(const MatND& a, double* minVal,
                       double* maxVal, int* minIdx=0, int* maxIdx=0,
                       const MatND& mask=MatND());
CV_EXPORTS void merge(const Vector<MatND>& mv, MatND& dst);
CV_EXPORTS void split(const MatND& m, Vector<MatND>& mv);
CV_EXPORTS void mixChannels(const Vector<MatND>& src, Vector<MatND>& dst,
                            const Vector<int>& fromTo);

CV_EXPORTS void bitwise_and(const MatND& a, const MatND& b, MatND& c, const MatND& mask=MatND());
CV_EXPORTS void bitwise_or(const MatND& a, const MatND& b, MatND& c, const MatND& mask=MatND());
CV_EXPORTS void bitwise_xor(const MatND& a, const MatND& b, MatND& c, const MatND& mask=MatND());
CV_EXPORTS void bitwise_and(const MatND& a, const Scalar& s, MatND& c, const MatND& mask=MatND());
CV_EXPORTS void bitwise_or(const MatND& a, const Scalar& s, MatND& c, const MatND& mask=MatND());
CV_EXPORTS void bitwise_xor(const MatND& a, const Scalar& s, MatND& c, const MatND& mask=MatND());
CV_EXPORTS void bitwise_not(const MatND& a, MatND& c);
CV_EXPORTS void absdiff(const MatND& a, const MatND& b, MatND& c);
CV_EXPORTS void absdiff(const MatND& a, const Scalar& s, MatND& c);
CV_EXPORTS void inRange(const MatND& src, const MatND& lowerb,
                        const MatND& upperb, MatND& dst);
CV_EXPORTS void inRange(const MatND& src, const Scalar& lowerb,
                        const Scalar& upperb, MatND& dst);
CV_EXPORTS void compare(const MatND& a, const MatND& b, MatND& c, int cmpop);
CV_EXPORTS void compare(const MatND& a, double s, MatND& c, int cmpop);
CV_EXPORTS void min(const MatND& a, const MatND& b, MatND& c);
CV_EXPORTS void min(const MatND& a, double alpha, MatND& c);
CV_EXPORTS void max(const MatND& a, const MatND& b, MatND& c);
CV_EXPORTS void max(const MatND& a, double alpha, MatND& c);

CV_EXPORTS void sqrt(const MatND& a, MatND& b);
CV_EXPORTS void pow(const MatND& a, double power, MatND& b);
CV_EXPORTS void exp(const MatND& a, MatND& b);
CV_EXPORTS void log(const MatND& a, MatND& b);
CV_EXPORTS bool checkRange(const MatND& a, bool quiet=true, int* idx=0,
                           double minVal=-DBL_MAX, double maxVal=DBL_MAX);

typedef void (*ConvertData)(const void* from, void* to, int cn);
typedef void (*ConvertScaleData)(const void* from, void* to, int cn, double alpha, double beta);

CV_EXPORTS ConvertData getConvertElem(int fromType, int toType);
CV_EXPORTS ConvertScaleData getConvertScaleElem(int fromType, int toType);

template<typename _Tp> class CV_EXPORTS MatND_ : public MatND
{
public:
    typedef _Tp value_type;
    typedef typename DataType<_Tp>::channel_type channel_type;

    MatND_();
    MatND_(const Vector<int>& _sizes);
    MatND_(const Vector<int>& _sizes, const _Tp& _s);
    MatND_(const MatND& m);
    MatND_(const MatND_& m);
    MatND_(const MatND_& m, const Vector<Range>& ranges);
    MatND_(const CvMatND* m, bool copyData=false);
    MatND_& operator = (const MatND& m);
    MatND_& operator = (const MatND_& m);
    MatND_& operator = (const _Tp& s);

    void create(const Vector<int>& _sizes);
    template<typename T2> operator MatND_<T2>() const;
    MatND_ clone() const;
    MatND_ operator()(const Vector<Range>& ranges) const;

    size_t elemSize() const;
    size_t elemSize1() const;
    int type() const;
    int depth() const;
    int channels() const;
    size_t stepT(int i) const;
    size_t step1(int i) const;

    _Tp& operator ()(const int* idx);
    const _Tp& operator ()(const int* idx) const;

    _Tp& operator ()(int idx0, int idx1, int idx2);
    const _Tp& operator ()(int idx0, int idx1, int idx2) const;
};

/////////////////////////// multi-dimensional sparse matrix //////////////////////////

class SparseMatIterator;
class SparseMatConstIterator;

class CV_EXPORTS SparseMat
{
public:
    typedef SparseMatIterator iterator;
    typedef SparseMatConstIterator const_iterator;

    struct CV_EXPORTS Hdr
    {
        Hdr(const Vector<int>& _sizes, int _type);
        void clear();
        int refcount;
        int dims;
        int valueOffset;
        size_t nodeSize;
        size_t nodeCount;
        size_t freeList;
        Vector<uchar> pool;
        Vector<size_t> hashtab;
        int size[CV_MAX_DIM];
    };

    struct CV_EXPORTS Node
    {
        size_t hashval;
        size_t next;
        int idx[CV_MAX_DIM];
    };

    SparseMat();
    SparseMat(const Vector<int>& _sizes, int _type);
    SparseMat(const SparseMat& m);
    SparseMat(const Mat& m, bool try1d=false);
    SparseMat(const MatND& m);
    SparseMat(const CvSparseMat* m);
    ~SparseMat();
    SparseMat& operator = (const SparseMat& m);
    SparseMat& operator = (const Mat& m);
    SparseMat& operator = (const MatND& m);

    SparseMat clone() const;
    void copyTo( SparseMat& m ) const;
    void copyTo( Mat& m ) const;
    void copyTo( MatND& m ) const;
    void convertTo( SparseMat& m, int rtype, double alpha=1 ) const;
    void convertTo( Mat& m, int rtype, double alpha=1, double beta=0 ) const;
    void convertTo( MatND& m, int rtype, double alpha=1, double beta=0 ) const;

    void assignTo( SparseMat& m, int type=-1 ) const;

    void create(const Vector<int>& _sizes, int _type);
    void clear();
    void addref();
    void release();

    operator CvSparseMat*() const;
    size_t elemSize() const;
    size_t elemSize1() const;
    int type() const;
    int depth() const;
    int channels() const;
    Vector<int> size() const;
    int size(int i) const;
    int dims() const;
    size_t nzcount() const;
    
    size_t hash(int i0) const;
    size_t hash(int i0, int i1) const;
    size_t hash(int i0, int i1, int i2) const;
    size_t hash(const int* idx) const;
    
    uchar* ptr(int i0, int i1, bool createMissing, size_t* hashval=0);
    const uchar* get(int i0, int i1, size_t* hashval=0) const;
    uchar* ptr(int i0, int i1, int i2, bool createMissing, size_t* hashval=0);
    const uchar* get(int i0, int i1, int i2, size_t* hashval=0) const;
    uchar* ptr(const int* idx, bool createMissing, size_t* hashval=0);
    const uchar* get(const int* idx, size_t* hashval=0) const;

    void erase(int i0, int i1, size_t* hashval=0);
    void erase(int i0, int i1, int i2, size_t* hashval=0);
    void erase(const int* idx, size_t* hashval=0);

    SparseMatIterator begin();
    SparseMatConstIterator begin() const;
    SparseMatIterator end();
    SparseMatConstIterator end() const;

    uchar* value(Node* n);
    const uchar* value(const Node* n) const;
    Node* node(size_t nidx);
    const Node* node(size_t nidx) const;

    uchar* newNode(const int* idx, size_t hashval);
    void removeNode(size_t hidx, size_t nidx, size_t previdx);
    void resizeHashTab(size_t newsize);

    enum { MAGIC_VAL=0x42FD0000, MAX_DIM=CV_MAX_DIM, HASH_SCALE=0x5bd1e995, HASH_BIT=0x80000000 };

    int flags;
    Hdr* hdr;
};


CV_EXPORTS void minMaxLoc(const SparseMat& a, double* minVal,
                          double* maxVal, int* minIdx=0, int* maxIdx=0);
CV_EXPORTS double norm( const SparseMat& src, int normType );
CV_EXPORTS void normalize( const SparseMat& src, SparseMat& dst, double alpha, int normType );

class CV_EXPORTS SparseMatConstIterator
{
public:
    SparseMatConstIterator();
    SparseMatConstIterator(const SparseMat* _m);
    SparseMatConstIterator(const SparseMatConstIterator& it);

    SparseMatConstIterator& operator = (const SparseMatConstIterator& it);
    const uchar* value() const;
    const SparseMat::Node* node() const;
    
    SparseMatConstIterator& operator --();
    SparseMatConstIterator operator --(int);
    SparseMatConstIterator& operator ++();
    SparseMatConstIterator operator ++(int);

    const SparseMat* m;
    size_t hashidx;
    uchar* ptr;
};

class CV_EXPORTS SparseMatIterator : public SparseMatConstIterator
{
public:
    SparseMatIterator();
    SparseMatIterator(SparseMat* _m);
    SparseMatIterator(SparseMat* _m, const int* idx);
    SparseMatIterator(const SparseMatIterator& it);

    SparseMatIterator& operator = (const SparseMatIterator& it);
    uchar* value() const;
    SparseMat::Node* node() const;
    
    SparseMatIterator& operator ++();
    SparseMatIterator operator ++(int);
};


template<typename _Tp> class SparseMatIterator_;
template<typename _Tp> class SparseMatConstIterator_;

template<typename _Tp> class CV_EXPORTS SparseMat_ : public SparseMat
{
public:
    typedef SparseMatIterator_<_Tp> iterator;
    typedef SparseMatConstIterator_<_Tp> const_iterator;

    SparseMat_();
    SparseMat_(const Vector<int>& _sizes);
    SparseMat_(const SparseMat& m);
    SparseMat_(const SparseMat_& m);
    SparseMat_(const Mat& m);
    SparseMat_(const MatND& m);
    SparseMat_(const CvSparseMat* m);
    SparseMat_& operator = (const SparseMat& m);
    SparseMat_& operator = (const SparseMat_& m);
    SparseMat_& operator = (const Mat& m);
    SparseMat_& operator = (const MatND& m);

    SparseMat_ clone() const;
    void create(const Vector<int>& _sizes);
    operator CvSparseMat*() const;

    int type() const;
    int depth() const;
    int channels() const;
    
    _Tp& operator()(int i0, int i1, size_t* hashval=0);
    _Tp operator()(int i0, int i1, size_t* hashval=0) const;
    _Tp& operator()(int i0, int i1, int i2, size_t* hashval=0);
    _Tp operator()(int i0, int i1, int i2, size_t* hashval=0) const;
    _Tp& operator()(const int* idx, size_t* hashval=0);
    _Tp operator()(const int* idx, size_t* hashval=0) const;

    SparseMatIterator_<_Tp> begin();
    SparseMatConstIterator_<_Tp> begin() const;
    SparseMatIterator_<_Tp> end();
    SparseMatConstIterator_<_Tp> end() const;
};

template<typename _Tp> class CV_EXPORTS SparseMatConstIterator_ : public SparseMatConstIterator
{
public:
    typedef std::forward_iterator_tag iterator_category;
    
    SparseMatConstIterator_();
    SparseMatConstIterator_(const SparseMat_<_Tp>* _m);
    SparseMatConstIterator_(const SparseMatConstIterator_& it);

    SparseMatConstIterator_& operator = (const SparseMatConstIterator_& it);
    const _Tp& operator *() const;
    
    SparseMatConstIterator_& operator ++();
    SparseMatConstIterator_ operator ++(int);

protected:
    const SparseMat_<_Tp>* m;
    size_t hashidx;
    uchar* ptr;
};

template<typename _Tp> class CV_EXPORTS SparseMatIterator_ : public SparseMatConstIterator_<_Tp>
{
public:
    typedef std::forward_iterator_tag iterator_category;
    
    SparseMatIterator_();
    SparseMatIterator_(SparseMat_<_Tp>* _m);
    SparseMatIterator_(const SparseMatIterator_& it);

    SparseMatIterator_& operator = (const SparseMatIterator_& it);
    _Tp& operator *() const;
    
    SparseMatIterator_& operator ++();
    SparseMatIterator_ operator ++(int);
};

//////////////////// Fast Nearest-Neighbor Search Structure ////////////////////

class CV_EXPORTS KDTree
{
public:
    struct Node
    {
        Node() : idx(-1), left(-1), right(-1), boundary(0.f) {}
        Node(int _idx, int _left, int _right, float _boundary)
            : idx(_idx), left(_left), right(_right), boundary(_boundary) {}
        int idx;            // split dimension; >=0 for nodes (dim),
                            // < 0 for leaves (index of the point)
        int left, right;    // node indices of left and right branches
        float boundary;     // left if vec[dim]<=boundary, otherwise right
    };

    KDTree();
    KDTree(const Mat& _points, bool copyPoints=true);
    void build(const Mat& _points, bool copyPoints=true);

    void findNearest(const Mat& vec, int K, int Emax, Vector<int>* neighborsIdx,
        Mat* neighbors=0, Vector<float>* dist=0) const;
    void findNearest(const Vector<float>& vec, int K, int Emax, Vector<int>* neighborsIdx,
        Vector<float>* neighbors=0, Vector<float>* dist=0) const;
    void findOrthoRange(const Mat& minBounds, const Mat& maxBounds,
        Vector<int>* neighborsIdx, Mat* neighbors=0) const;
    void findOrthoRange(const Vector<float>& minBounds, const Vector<float>& maxBounds,
        Vector<int>* neighborsIdx, Vector<float>* neighbors=0) const;
    void getPoints(const Vector<int>& ids, Mat& pts) const;
    void getPoints(const Vector<int>& ids, Vector<float>& pts) const;
    Vector<float> at(int ptidx, bool copyData=false) const;

    Vector<Node> nodes;
    Mat points;
    int maxDepth;
};

//////////////////////////////////////// XML & YAML I/O ////////////////////////////////////

class CV_EXPORTS FileNode;

class CV_EXPORTS FileStorage
{
public:
    enum { READ=0, WRITE=1, APPEND=2 };
    enum { UNDEFINED=0, VALUE_EXPECTED=1, NAME_EXPECTED=2, INSIDE_MAP=4 };
    FileStorage();
    FileStorage(const String& filename, int flags);
    FileStorage(CvFileStorage* fs);
    virtual ~FileStorage();

    virtual bool open(const String& filename, int flags);
    virtual bool isOpened() const;
    virtual void release();

    FileNode getFirstTopLevelNode() const;
    FileNode root(int streamidx=0) const;
    FileNode operator[](const String& nodename) const;
    FileNode operator[](const char* nodename) const;

    CvFileStorage* operator *() { return fs; }
    const CvFileStorage* operator *() const { return fs; }
    void writeRaw( const String& fmt, const Vector<uchar>& vec );
    void writeObj( const String& name, const void* obj );

    static String getDefaultObjectName(const String& filename);

    Ptr<CvFileStorage> fs;
    String elname;
    Vector<char> structs;
    int state;
};

class CV_EXPORTS FileNodeIterator;

class CV_EXPORTS FileNode
{
public:
    enum { NONE=0, INT=1, REAL=2, FLOAT=REAL, STR=3, STRING=STR, REF=4, SEQ=5, MAP=6, TYPE_MASK=7,
        FLOW=8, USER=16, EMPTY=32, NAMED=64 };
    FileNode();
    FileNode(const CvFileStorage* fs, const CvFileNode* node);
    FileNode(const FileNode& node);
    FileNode operator[](const String& nodename) const;
    FileNode operator[](const char* nodename) const;
    FileNode operator[](int i) const;
    int type() const;
    int rawDataSize(const String& fmt) const;
    bool isNone() const;
    bool isSeq() const;
    bool isMap() const;
    bool isInt() const;
    bool isReal() const;
    bool isString() const;
    bool isNamed() const;
    String name() const;
    size_t count() const;
    operator int() const;
    operator float() const;
    operator double() const;
    operator String() const;

    FileNodeIterator begin() const;
    FileNodeIterator end() const;

    void readRaw( const String& fmt, Vector<uchar>& vec ) const;
    void* readObj() const;

    // do not use wrapper pointer classes for better efficiency
    const CvFileStorage* fs;
    const CvFileNode* node;
};

class CV_EXPORTS FileNodeIterator
{
public:
    FileNodeIterator();
    FileNodeIterator(const CvFileStorage* fs, const CvFileNode* node, size_t ofs=0);
    FileNodeIterator(const FileNodeIterator& it);
    FileNode operator *() const;
    FileNode operator ->() const;

    FileNodeIterator& operator ++();
    FileNodeIterator operator ++(int);
    FileNodeIterator& operator --();
    FileNodeIterator operator --(int);
    FileNodeIterator& operator += (int);
    FileNodeIterator& operator -= (int);

    FileNodeIterator& readRaw( const String& fmt, Vector<uchar>& vec,
                               size_t maxCount=(size_t)INT_MAX );

    const CvFileStorage* fs;
    const CvFileNode* container;
    CvSeqReader reader;
    size_t remaining;
};

////////////// convenient wrappers for operating old-style dynamic structures //////////////

// !!! NOTE that the wrappers are "thin", i.e. they do not call
// any element constructors/destructors

template<typename _Tp> class SeqIterator;

template<> inline void Ptr<CvMemStorage>::delete_obj()
{ cvReleaseMemStorage(&obj); }

typedef Ptr<CvMemStorage> MemStorage;

template<typename _Tp> class CV_EXPORTS Seq
{
public:
    Seq();
    Seq(const CvSeq* seq);
    Seq(const MemStorage& storage, int headerSize = sizeof(CvSeq));
    _Tp& operator [](int idx);
    const _Tp& operator[](int idx) const;
    SeqIterator<_Tp> begin() const;
    SeqIterator<_Tp> end() const;
    size_t size() const;
    int type() const;
    int depth() const;
    int channels() const;
    size_t elemSize() const;
    size_t index(const _Tp& elem) const;
    void push_back(const _Tp& elem);
    void push_front(const _Tp& elem);
    _Tp& front();
    const _Tp& front() const;
    _Tp& back();
    const _Tp& back() const;
    bool empty() const;

    void clear();
    void pop_front();
    void pop_back();

    void copyTo(Vector<_Tp>& vec, const Range& range=Range::all()) const;
    operator Vector<_Tp>() const;
    
    CvSeq* seq;
};

template<typename _Tp> class CV_EXPORTS SeqIterator : public CvSeqReader
{
public:
    SeqIterator();
    SeqIterator(const Seq<_Tp>& seq, bool seekEnd=false);
    void seek(size_t pos);
    size_t tell() const;
    _Tp& operator *();
    const _Tp& operator *() const;
    SeqIterator& operator ++();
    SeqIterator operator ++(int) const;
    SeqIterator& operator --();
    SeqIterator operator --(int) const;

    SeqIterator& operator +=(int);
    SeqIterator& operator -=(int);

    // this is index of the current element module seq->total*2
    // (to distinguish between 0 and seq->total)
    int index;
};

}

#endif // __cplusplus

#include "cxoperations.hpp"
#include "cxmat.hpp"

#endif /*_CXCORE_HPP_*/
