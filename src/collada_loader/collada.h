#ifndef COLLADA_H
#define COLLADA_H
#include "tinyxml2.h"

#include "3d_math.h"

using namespace tinyxml2;


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
    _submesh(){}
    _submesh(const _submesh& sm) { copy(sm); }
    void operator = (const _submesh& sm) { copy(sm); }
    void copy(const _submesh& sm){
        m_indices = sm.m_indices;
        m_vertices = sm.m_vertices;
    }
    _int_array m_indices;
    _array<_vertex> m_vertices;

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
#define rt_firstchild(X) ( X=X->FirstChildElement() )
#define rt_nextsibling(X) ( X=X->NextSiblingElement() )

struct collada_source {
    collada_source(){}
    collada_source(const collada_source& s){ copy(s); }
    void operator = (const collada_source& s){ copy(s); }
    void copy(const collada_source& s){
        m_id = s.m_id;
        m_float_array  = s.m_float_array;
        m_string_array = s.m_string_array;
        m_accessor_source = s.m_accessor_source;
        m_accessor_count  = s.m_accessor_count;
        m_accessor_stride = s.m_accessor_stride;
    }
    _string m_id;
    _float_array  m_float_array;
    _string_array m_string_array;
    _string       m_accessor_source;
    uint32_t      m_accessor_count;
    uint32_t      m_accessor_stride;
};

struct collada_input{
    collada_input():m_offset(0),m_set(0) {}
    collada_input(const collada_input& i ) { copy(i); }
    void operator = (const collada_input& i) { copy(i); }
    void copy(const collada_input& i){
        m_semantic = i.m_semantic;
        m_source   = i.m_source;
        m_offset   = i.m_offset;
        m_set      = i.m_set;
    }
    _string   m_semantic;
    _string   m_source;
    uint16_t  m_offset;
    uint16_t  m_set;
};

struct collada_polylist{
    collada_polylist():m_count(0){}
    collada_polylist(const collada_polylist& p):m_count(0){ copy(p); }
    void operator = (const collada_polylist& p){ copy(p); }
    void copy(const collada_polylist& p){
        m_inputs   = p.m_inputs;
        m_vcount   = p.m_vcount;
        m_p        = p.m_p;
        m_material = p.m_material;
        m_count    = p.m_count;
    }
    _array<collada_input> m_inputs;
    _int_array m_vcount;
    _int_array m_p;
    _string    m_material;
    int32_t    m_count;
};

struct collada_controller {
    collada_controller():m_v_weight_count(0) {}
    collada_controller(const collada_controller& c) { copy(c); }
    void operator = (const collada_controller& c)   { copy(c); }
    void copy(const collada_controller& c) {
        m_id = c.m_id;
        m_name = c.m_name;
        m_joint_source = c.m_joint_source;
        m_joint_weight_source = c.m_joint_weight_source;
        m_inverse_bind_source = c.m_inverse_bind_source;
        m_bind_shape_matrix = c.m_bind_shape_matrix;
        m_vcount = c.m_vcount;
        m_v = c.m_v;
        m_v_weight_count = c.m_v_weight_count;
    }
    _string    m_id;
    _string    m_name;
    _string    m_joint_source;
    _string    m_joint_weight_source;
    _string    m_inverse_bind_source;
    _mat4      m_bind_shape_matrix;
    _int_array m_vcount;
    _int_array m_v;
    uint32_t   m_v_weight_count;
};

struct rt_node{
    rt_node():m_parent(NULL),m_children(NULL),m_count(0) { }
    rt_node(const rt_node& n);
    void operator = (const rt_node& n);
    rt_node * addnode(const _string& id,const _mat4& transform ){

        rt_node * node = new rt_node();
        node->m_parent = this;
        node->m_id = id;
        node->m_transform = transform;

        rt_node ** children = new rt_node*[m_count+1];

        if(m_children){
            for(uint32_t i=0;i<m_count;i++){ children[i] = m_children[i]; }
            delete [] m_children;
        }
        m_children = children;
        m_children[m_count] = node;
        m_count++;

        return node;
    }
    static void delete_nodes(rt_node * root){
        for(uint32_t i=0;i<root->m_count;i++){
            delete_nodes(root->m_children[i]);
        }
        delete root;
    }

    _string    m_id;
    _mat4      m_transform;
    rt_node *  m_parent;
    rt_node ** m_children;
    uint32_t   m_count;
};

struct rt_animations {
    rt_animations(){}
    rt_animations(const rt_animations& a){ copy(a); }
    void operator = (const rt_animations& a){ copy(a); }
    void copy(const rt_animations& a){
        m_target = a.m_target;
        m_output_source = a.m_output_source;
    }
    _string m_target;
    _string m_output_source;
};

struct collada {
    collada():m_root_node(NULL),m_bone_count(0),m_keyframe_count(0){}

    _array<collada_source> m_sources;

    _string m_vertexsource;

    _array <collada_polylist> m_polylists;

    collada_controller m_controller;

    int findsource(const char* id);

    bool loadfile(const char* path);

    bool readgeometries(XMLElement * element);
    void readanimations(XMLElement * element);
    void readcontrollers(XMLElement * element);
    void readvisualscenes(XMLElement * element);

    bool generatetransforms(rt_node*node, const _mat4&parent, int32_t animation_index);

    _string_array m_bonenames;

    _matrix_array m_transforms;
    _matrix_array m_inv_transforms;

    _string m_keyframe_source;
    _transform_array m_keyframes;
    _float_array m_keyframe_inputs;
    _array<rt_animations> m_animations;

    rt_node * m_root_node;
    uint16_t  m_bone_count;
    uint16_t  m_keyframe_count;
    bool genaratefile(const char* path);
};

#endif // COLLADA_H
