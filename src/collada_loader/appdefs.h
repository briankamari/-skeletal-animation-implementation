#ifndef RABBITDEFS_H
#define RABBITDEFS_H

#include <cstdio>
#include <cstdint>
#include <cstring>

#include <d3d9.h>
#include <d3dx9.h>

#include "3d_math.h"

#define app_title  "rabbit collada mesh"

#define app_width  800
#define app_height 600


#define app_zero(x,y) { for(uint32_t i=0;i<y;( (uint8_t*)(x) )[i]=0 ,i++); }

#define app_error(x) { fprintf(stderr,"error %s l: %i f: %s \n",x,__LINE__,__FILE__); }
#define app_throw(x) { fprintf(stderr,"error %s l: %i f: %s \n",x,__LINE__,__FILE__); return false; }

#define app_error_hr(x) if(FAILED(x)){ app_error("hr"); }
#define app_throw_hr(x) if(FAILED(x)){ app_throw("hr"); }
#define app_releasecom(x) { if(x){ x->Release();x = 0; } }

#define app_scm(X,Y) (strcmp(X,Y)==0)

template <typename T,typename T2 = uint32_t >
struct _array {

    T * m_data;
    T2  m_size;
    T2  m_count;

    ~_array(){ clear(); }
    _array() : m_data(NULL),m_size(0),m_count(0) {}
    _array(const _array& x) : m_data(NULL),m_size(0),m_count(0){ copy(x); }
    void operator = (const _array& x) { copy(x);}

    _array(const char *str) : m_data(NULL),m_size(0),m_count(0) {
        if(!str){ return; }
        uint32_t len = strlen(str);
        if(len){
            clear();
            alloc(len+1);
            m_count = len;
            for(T2 i =0;i<m_count; i++){ m_data[i] = str[i]; }
        }
    }

    void operator = (const char* str) {
        if(!str){ return; }
        uint32_t len = strlen(str);
        if(len){
            clear();
            alloc(len+1);
            m_count = len;
            for(T2 i =0;i<m_count; i++){ m_data[i] = str[i]; }
        }
    }

    void copy (const _array& x){
        clear();
        if(x.m_count){
            alloc(x.m_count+1);
            m_count = x.m_count;
            for(T2 i =0;i<m_count; i++){ m_data[i] = x.m_data[i]; }
        }
    }

    void clear() { if(m_data){ delete [] m_data;m_data = NULL;m_size=m_count=0;} }

    void alloc(const T2& count){
        if(m_size >= count){ return; }

        T* buffer = new T[count];
        app_zero(buffer,sizeof(T)*count);
        if( m_data ){
            for(T2 i =0;i<m_size; i++){ buffer[i] = m_data[i]; }
            delete [] m_data;
        }
        m_data = buffer;
        m_size = count;
    }

    void allocate(const T2& count){
        clear();
        alloc(count+1);
        m_count=count;
    }

    void pushback(const T& val,bool p2 = false){
        T2 count = m_count+2;
        if(p2) { count = (m_size<=count)? count*2 : count; }
        alloc(count);
        m_data[m_count++] = val;
    }

    _array operator + (const _array& str){
        _array result;
        result.allocate(m_count+str.m_count);
        for(T2 i=0;i<m_count;i++){ result[i]=m_data[i]; }
        for(T2 i=0,ii=m_count;i<str.m_count;i++,ii++){ result[ii]=str[i]; }
        return result;
    }

    T& operator [](const T2& index){ return m_data[index]; }
    const T& operator [](const T2& index) const { return m_data[index]; }

};

typedef _array<char> _string;


typedef _vector2<float> _vec2;
typedef _vector3<float> _vec3;
typedef _vector4<float> _vec4;

typedef _matrix3<float> _mat3;
typedef _matrix4<float> _mat4;

typedef _array<float>     _float_array;
typedef _array<int32_t>   _int_array;
typedef _array<_string> _string_array;

typedef _array<_mat4>         _matrix_array;
typedef _array<_matrix_array> _transform_array;

struct _utility{
    static _float_array stringtofloatarray(const _string_array& strings ){
        _float_array result;
        for(uint32_t i=0; i<strings.m_count;i++){ result.pushback( float(atof(strings[i].m_data) ),true ); }
        return result;
    }
    static _int_array stringtointarray(const _string_array& strings ){
        _int_array result;
        for(uint32_t i=0; i<strings.m_count;i++){ result.pushback( int(atoi(strings[i].m_data) ),true ); }
        return result;
    }
    static _string_array stringsplit(const _string& string,char split = ' ' ){
        _string buffer;
        _string_array result;
        for(uint32_t i=0; i<string.m_count;i++){
            if(string[i] == split ){
                if(buffer.m_count>0){
                    result.pushback(buffer,true);
                    buffer.clear();
                }
            }else { buffer.pushback(string[i],true); }
        }
        if(buffer.m_count>0){ result.pushback(buffer,true); }
        return result;
    }
    static _float_array stringtofloatarray(const _string & string){
        _string_array array = stringsplit(string);
        return stringtofloatarray(array);
    }
    static _int_array stringtointarray(const _string & string){
        _string_array array = stringsplit(string);
        return stringtointarray(array);
    }
    static float lerp(float x,float y,float t) { return x*(1.0f - t)+y * t; }

    template <typename T>
    static void print_mat(const T& m, const int count = 3){

        for(int i=0;i<count;i++){
            for(int ii=0;ii<count;ii++) { printf("%.2f,",m[i][ii]); }
            printf("\n");
        }
    }

    static _string floattostring(const float& d){
        char buffer[20];
        app_zero(buffer,20);
        sprintf_s(buffer,20,"%f",d);
        return _string(buffer);
    }
    static _string inttostring(const int& i){
        char buffer[20];
        app_zero(buffer,20);
        sprintf_s(buffer,20,"%i",i);
        return _string(buffer);
    }

    static double degrees(double radians) {
        return radians * static_cast<double>(57.295779513082320876798154814105);
    }
    static double radians(double degrees) {
        return degrees * static_cast<double>(0.01745329251994329576923690768489);
    }

};
#define _degrees(X) _utility::degrees(X)
#define _radians(X) _utility::radians(X)

#define _lerp(X,Y,T) _utility::lerp(X,Y,T)

#define _print_mat(X,Y) _utility::print_mat(X,Y)

#define _stringtoints(X)   _utility::stringtointarray(X)
#define _stringtofloats(X) _utility::stringtofloatarray(X)

#define _stringsplit(X)    _utility::stringsplit(X)
#define _stringsplit_(X,Y)  _utility::stringsplit(X,Y)

struct _app;

struct d3d_window;
struct d3d_manager;

#define app          _app::_instance
#define _window      d3d_window::__window
#define _api_manager d3d_manager::_manager

#define _fx _api_manager->m_fx

#define app_init       0x1
#define app_running    0x2
#define app_fullscreen 0x4
#define app_vsync      0x8
#define app_lostdev    0x10
#define app_deverror   0x20

#define app_lmousedown 0x40
#define app_rmousedown 0x80

#define app_paused     0x100

struct app_flags {
    app_flags(): m_flags(0) {}
    uint32_t m_flags;

    void clear(){ m_flags = 0; }
    void addflags(const uint32_t & flags) { m_flags |= flags; }
    bool testflags(const uint32_t & flags) { return (m_flags&flags )!=0; }
    void removeflags(const uint32_t & flags) { m_flags &= ~flags; }
};

struct app_object : public app_flags {
    virtual bool init()=0;
    virtual void clear()=0;
    virtual bool update()=0;
};

struct _vertex {
    _vertex(){}
    _vertex(const _vertex& v){ copy(v); }
    void operator = (const _vertex& v){ copy(v); }
    void copy(const _vertex& v){
        m_vertex = v.m_vertex;
        m_normal = v.m_normal;
        m_uv = v.m_uv;
        m_bone_indexes = v.m_bone_indexes;
        m_bone_weights = v.m_bone_weights;
    }
    _vec3 m_vertex;
    _vec3 m_normal;
    _vec2 m_uv;
    _vec2 m_bone_indexes;
    _vec2 m_bone_weights;
};

struct _submesh {
    _submesh():m_vertex_buffer(NULL),m_index_buffer(NULL){}
    _submesh(const _submesh& sm) { copy(sm); }
    void operator = (const _submesh& sm) { copy(sm); }
    void copy(const _submesh& sm){
        m_indices = sm.m_indices;
        m_vertices = sm.m_vertices;
        m_vertex_buffer = sm.m_vertex_buffer;
        m_index_buffer  = sm.m_index_buffer;
    }
    _int_array m_indices;
    _array<_vertex> m_vertices;
    IDirect3DVertexBuffer9* m_vertex_buffer;
    IDirect3DIndexBuffer9*  m_index_buffer;

};
typedef _array<_submesh> _submeshes;
struct _mesh {
    _mesh(){}
    _mesh(const _mesh& m ){ copy(m); }
    void operator = (const _mesh& m){ copy(m); }
    void copy(const _mesh& m){
        m_bones     = m.m_bones;
        m_keyframes = m.m_keyframes;
        m_submeshes = m.m_submeshes;
    }
    _matrix_array m_bones;
    _transform_array m_keyframes;
    _array<_submesh> m_submeshes;
};

#endif // RABBITDEFS_H
