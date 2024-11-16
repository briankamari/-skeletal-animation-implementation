#ifndef GRAYMAN_H
#define GRAYMAN_H

#include "appdefs.h"
#include "d3d_manager.h"

struct  grayman {

    grayman();

    virtual bool init();
    virtual void clear();
    virtual bool update();

    void setbindpose(){ m_current_keyframe=m_start_keyframe=m_end_keyframe=0;}
    void keyframe(const uint32_t &start, const uint32_t &end);
    D3DXMATRIX* currentkeyframe();

    _matrix_array m_keyframe_buffer;

    float   m_animation_second;
    float   m_animation_length;
    int32_t m_current_keyframe;
    int32_t m_start_keyframe;
    int32_t m_end_keyframe;

    _mat4 m_world;

    _mesh m_mesh;
};

#endif // GRAYMAN_H
