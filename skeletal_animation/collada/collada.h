#ifndef COLLADA_H
#define COLLADA_H

#include "appdefs.h"
#include "tinyxml2.h"

using namespace tinyxml2;

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
