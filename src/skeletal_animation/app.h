#ifndef _app_H
#define _app_H

#include "appdefs.h"

struct grayman;
struct _app : public app_flags {

    _app();

    void run();
    bool init();
    void clear();
    bool update();

    void onlostdevice();
    void onresetdevice();

    ID3DXFont* m_font;
    grayman * m_grayman;


    _vec3 m_up;
    _vec3 m_look;

    _vec3 m_target;
    _vec3 m_position;

    _mat4 m_view;
    _mat4 m_projection;

    float m_distance;
    float m_x_cursor_pos;
    float m_y_cursor_pos;

    int   m_keyframe;
    int   m_framespersecond;
    float m_last_frame_seconds;
    float m_last_frame_milliseconds;

    float m_y_pos;
    float m_x_pos;

    float m_yaw;
    float m_pitch;
    float m_yaw_pos;
    float m_pitch_pos;


    static _vec3 UP;

    static _app * _instance;
    static HINSTANCE win32_instance;

    static bool readrtmeshfile(const char* path,_mesh* submeshes);

};

#endif // _app_H
