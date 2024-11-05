#include "app.h"

#include "d3d_window.h"
#include "d3d_manager.h"

#include "grayman.h"

_vec3   _app::UP = _vec3(0.0f,1.0f,0.0f);
_app *  _app::_instance = NULL;
HINSTANCE _app::win32_instance = NULL;

_app::_app(){

    m_font = NULL;
    m_grayman = NULL;

    m_distance = 100.0f;

    m_up       = UP;
    m_look     = _vec3(0.0f,-1.0f,0.0f);
    m_target   = _vec3(0.0f,0.0f,0.0f);
    m_position = _vec3(0.0f,30.0f,0.0f);

    m_keyframe = 0;
    m_framespersecond = 0;
    m_last_frame_seconds = 0;
    m_last_frame_milliseconds = 0;

    m_y_pos = 0.0f;
    m_x_pos = 0.0f;

    m_yaw = -45.0f;
    m_pitch = -10.0f;
    m_yaw_pos = -45.0f;
    m_pitch_pos = -10.0f;
    m_x_cursor_pos = 0.0f;
    m_y_cursor_pos = 0.0f;
}

void _app::run(){

    m_grayman = new grayman();
    m_grayman->init();
    m_grayman->setbindpose();

    m_keyframe =1; // walk animation
    app->m_grayman->keyframe(1,19);
    app->m_grayman->m_animation_length =0.04f;

    _window->update();

    int64_t tickspersecond = 0;
    int64_t previous_timestamp = 0;
    QueryPerformanceFrequency((LARGE_INTEGER*)&tickspersecond);
    float secsPerCnt = 1.0f / (float)tickspersecond;

    QueryPerformanceCounter((LARGE_INTEGER*)&previous_timestamp);

    int frames = 0;
    float second =0.0f;
    while( testflags(app_running) ){

        //**d3d device test.  error exits app**
        removeflags(app_lostdev);
        HRESULT hr = _api_manager->m_d3ddevice->TestCooperativeLevel();

        if( hr == D3DERR_DEVICELOST ) {  addflags( app_lostdev );  }
        else if( hr == D3DERR_DRIVERINTERNALERROR ) {
            addflags( app_deverror );
            app_error("d3d device error");
        }else if( hr == D3DERR_DEVICENOTRESET ){
            _api_manager->reset();
            addflags( app_lostdev );
        }
        //*************************************

        int64_t currenttimestamp = 0;
        QueryPerformanceCounter((LARGE_INTEGER*)&currenttimestamp);

        m_last_frame_seconds = float(currenttimestamp - previous_timestamp) * secsPerCnt;
        m_last_frame_milliseconds = m_last_frame_seconds*1000.0f;
        second += m_last_frame_seconds;
        frames++;

        previous_timestamp = currenttimestamp;
        static _string stats;
        if(second >=1.0f){
            stats = _string("mspf: ")+_utility::floattostring(m_last_frame_milliseconds);
            stats = stats + _string(" fps: ");
            stats = stats + _utility::inttostring(frames);
            second = 0.0f;
            frames = 0;
        }
        if( !(testflags(app_lostdev)) ) {
            update();
            app_error_hr(_api_manager->m_d3ddevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255, 255, 255), 1.0f, 0));
            app_error_hr(_api_manager->m_d3ddevice->BeginScene());

            RECT rect;
            app_zero(&rect,sizeof(RECT));
            rect.right  = stats.m_count*5+40;
            rect.bottom = 20;
            m_font->DrawText(0,stats.m_data, -1,&rect, DT_CENTER | DT_VCENTER, D3DCOLOR_XRGB(55, 55, 55));
            m_grayman->update();

            app_error_hr(_api_manager->m_d3ddevice->EndScene());
            app_error_hr(_api_manager->m_d3ddevice->Present(0, 0, 0, 0));
        }
        if( testflags(app_deverror) ) { removeflags(app_running); }
        else { _window->update(); }
    }
    m_grayman->clear();
    clear();
}

bool _app::init(){

    win32_instance = GetModuleHandle(NULL);

    _window = new d3d_window();
    _api_manager = new d3d_manager();

    addflags(app_running);
    if(!_window->init()) { return false;}
    if(!_api_manager->init()) { return false; }


    D3DXFONT_DESC fontDesc;
    fontDesc.Height          = 10;
    fontDesc.Width           = 5;
    fontDesc.Weight          = FW_SEMIBOLD;
    fontDesc.MipLevels       = 0;
    fontDesc.CharSet         = DEFAULT_CHARSET;
    fontDesc.OutputPrecision = OUT_DEFAULT_PRECIS;
    fontDesc.Quality         = DEFAULT_QUALITY;
    fontDesc.PitchAndFamily  = DEFAULT_PITCH | FF_DONTCARE;

    //*crude solution to avoid compiler warning
    app_zero(fontDesc.FaceName,20);
    const char *fontname = "Times New Roman";
    for(int i=0;i<16;i++){ fontDesc.FaceName[i] = fontname[i]; }
    //********

    app_throw_hr(D3DXCreateFontIndirect(_api_manager->m_d3ddevice, &fontDesc, &m_font));

    float w = (float)_api_manager->m_d3dpp.BackBufferWidth;
    float h = (float)_api_manager->m_d3dpp.BackBufferHeight;
    m_projection = rt_perspectivefovrh(D3DX_PI * 0.25f, w,h, 1.0f, 1000.0f);

    return true;
}

void _app::clear(){
    app_releasecom(m_font);
    if(_api_manager ) { _api_manager->clear();}
}

bool _app::update(){

    //camera*********************************************
    m_pitch = (m_pitch >  45.0f ) ? 45.0f  : m_pitch;
    m_yaw   = (m_yaw   >  180.0f) ? 180.0f : m_yaw;
    m_pitch = (m_pitch < -90.0f ) ?-90.0f  : m_pitch;
    m_yaw   = (m_yaw   < -180.0f) ?-180.0f : m_yaw;

    _mat4  r = rt_yawpitchroll(float(_radians(m_yaw)),float(_radians(m_pitch)),0.0f);
    _vec4 t  = r*_vec4(0,0,m_distance,0.0f);
    _vec4 up = r*_vec4(UP.x,UP.y,UP.z,0.0f);

    m_up = _vec3(up.x,up.y,up.z);
    m_position = m_target + _vec3(t.x,t.y,t.z);
    m_look = rt_normalize(m_target-m_position);

    m_view = rt_lookatrh(m_position, m_target, m_up);
    //***************************************************


    POINT cursor_position;
    if (GetCursorPos(&cursor_position)) {
        m_x_cursor_pos = float(cursor_position.x);
        m_y_cursor_pos = float(cursor_position.y);
    }else{ app_error("cursor pos");}

    return true;
}

void _app::onlostdevice() {
    app_error_hr(_fx->OnLostDevice());
    app_error_hr(m_font->OnLostDevice());
}

void _app::onresetdevice() {
    app_error_hr(_fx->OnResetDevice());
    app_error_hr(m_font->OnResetDevice());

}

bool _app::readrtmeshfile(const char* path,_mesh * mesh){

    FILE * file = NULL;
    fopen_s(&file,path,"rb");
    if(!file){ app_throw("readrtmeshfile"); }

    char  header_[9];
    app_zero(header_,9);

    if( fread(header_,1,6,file) != 6 ){ app_throw("fread"); }
    if(!app_scm(header_,"_mesh_")) { app_throw("not _mesh_ file"); }

    uint16_t submesh_count_ = 0;
    if( fread((&submesh_count_),2,1,file) != 1 ){ app_throw("fread"); }
    printf("smcount : %u \n",submesh_count_);

    for(uint32_t i=0;i<submesh_count_;i++){
        _submesh submesh_;
        uint32_t index_count_ = 0;
        if( fread((&index_count_),4,1,file) != 1 ){ app_throw("fread"); }

        int32_t * indices = new int32_t[index_count_];
        if( fread(indices,4,index_count_,file) != index_count_ ){ app_throw("fread"); }
        for(uint32_t ii=0;ii<index_count_;ii++){
            submesh_.m_indices.pushback(indices[ii],true);
        }

        uint32_t vertex_count_ = 0;
        if( fread((&vertex_count_),4,1,file) != 1 ){ app_throw("fread"); }

        _vertex * vertices = new _vertex[vertex_count_];
        if( fread(vertices,sizeof(_vertex),vertex_count_,file) != vertex_count_ ){ app_throw("fread"); }
        for(uint32_t ii=0;ii<vertex_count_;ii++){
            submesh_.m_vertices.pushback(vertices[ii],true);
        }
        mesh->m_submeshes.pushback(submesh_,true);
    }
    uint16_t bone_count = 0;
    if( fread((&bone_count),2,1,file) != 1 ){ app_throw("fread"); }
    if(bone_count){

        printf("bone_count: %i \n",bone_count);
        mesh->m_bones.allocate(bone_count);
        if( fread( (&(mesh->m_bones[0])) ,sizeof(_mat4 ),bone_count,file) != bone_count ){ app_throw("fread"); }

        uint16_t keyframe_count = 0;
        if( fread( (&keyframe_count) ,2,1,file) != 1 ){ app_throw("fread"); }
        if( keyframe_count){
            printf("keyframes: %i \n",keyframe_count);
            for(uint32_t ii=0;ii<keyframe_count;ii++){
                _matrix_array keyframe;
                keyframe.allocate(bone_count);
                if( fread( &(keyframe[0]) ,sizeof(_mat4 ),bone_count,file) != bone_count ){ app_throw("fread"); }
                mesh->m_keyframes.pushback(keyframe);
            }
        }
    }
    fclose(file);
    return true;
}
