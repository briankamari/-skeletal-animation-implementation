#include "grayman.h"

#include "app.h"

#include "d3d_window.h"
#include "d3d_manager.h"

grayman::grayman(){
	m_animation_second = 0.0f;
	m_animation_length = 0.0f;
	m_current_keyframe = 0;
	m_start_keyframe   = 0;
	m_end_keyframe     = 0;
}

bool grayman::init(){

	m_animation_length = 0.25f;

	//* due to blender's up axis being Z
	m_world = _rotate(float(_radians(-90.0f)),_vec3(1.0f,0.0f,0.0f));
	//**********************************

	if(!_app::readrtmeshfile("grayman._mesh",&m_mesh) ){ app_throw("readmesh"); }
	if(m_mesh.m_bones.m_count){ m_keyframe_buffer.allocate(m_mesh.m_bones.m_count); }

	for(uint32_t i=0;i<m_mesh.m_submeshes.m_count;i++){

		app_throw_hr(_api_manager->m_d3ddevice->CreateVertexBuffer(
			m_mesh.m_submeshes[i].m_vertices.m_count * sizeof(_vertex),
			D3DUSAGE_WRITEONLY,0, D3DPOOL_MANAGED,	&(m_mesh.m_submeshes[i].m_vertex_buffer), 0));
		if(!m_mesh.m_submeshes[i].m_vertex_buffer){ app_throw("vertex buffer"); }

		_vertex * v = 0;
		app_throw_hr(m_mesh.m_submeshes[i].m_vertex_buffer->Lock(0, 0, (void**)&v, 0));
		for(uint32_t ii=0;ii<m_mesh.m_submeshes[i].m_vertices.m_count;ii++){
			v[ii] = m_mesh.m_submeshes[i].m_vertices[ii];
			//st to uv************************
			v[ii].m_uv.y = 1.0f-v[ii].m_uv.y;
			//********************************
		}
		app_throw_hr(m_mesh.m_submeshes[i].m_vertex_buffer->Unlock());

		app_throw_hr(_api_manager->m_d3ddevice->CreateIndexBuffer(
			m_mesh.m_submeshes[i].m_indices.m_count * sizeof(WORD),
			D3DUSAGE_WRITEONLY,D3DFMT_INDEX16,
			D3DPOOL_MANAGED, &(m_mesh.m_submeshes[i].m_index_buffer), 0));
		if(!m_mesh.m_submeshes[i].m_index_buffer){ app_throw("index_buffer"); }

		WORD* indices = 0;
		app_throw_hr(m_mesh.m_submeshes[i].m_index_buffer->Lock(0, 0, (void**)&(indices), 0));
		for(uint32_t ii=0;ii<m_mesh.m_submeshes[i].m_indices.m_count/3;ii++){

			//* conversion from right hand( opengl ) to left hand( direct3d ) Coordinate Systems
			//* requires clockwise rotation of triangles
			/*https://learn.microsoft.com/en-us/windows/win32/direct3d9/coordinate-systems*/
			uint32_t pos = ii*3;
			indices[pos  ] = WORD(m_mesh.m_submeshes[i].m_indices[pos]);
			indices[pos+1] = WORD(m_mesh.m_submeshes[i].m_indices[pos+2]);
			indices[pos+2] = WORD(m_mesh.m_submeshes[i].m_indices[pos+1]);

		}
		app_throw_hr(m_mesh.m_submeshes[i].m_index_buffer->Unlock());
	}
	return true;
}
void grayman::clear(){
	for(uint32_t i=0;i<m_mesh.m_submeshes.m_count;i++){
		app_releasecom(m_mesh.m_submeshes[i].m_index_buffer);
		app_releasecom(m_mesh.m_submeshes[i].m_vertex_buffer);
	}
}
bool grayman::update(){

	static _mat4 m_mat;

	if( ((m_end_keyframe - m_start_keyframe)>0) ){
		if(m_animation_second>=m_animation_length){
			m_animation_second=0;
			if(m_current_keyframe<m_end_keyframe){ m_current_keyframe++; }
			else{ m_current_keyframe = m_start_keyframe; }
		}
		else { m_animation_second += app->m_last_frame_seconds; }
	}

	m_mat = m_world*app->m_view*app->m_projection;
	app_throw_hr(_fx->SetMatrix(_api_manager->m_hmvp,(D3DXMATRIX*)&m_mat ));
	app_throw_hr(_fx->SetMatrix(_api_manager->m_hworld, (D3DXMATRIX*)&m_world));

	app_throw_hr(_api_manager->m_d3ddevice->SetVertexDeclaration(_api_manager->m_vertex_declaration));
	app_throw_hr(_fx->SetMatrixArray(_api_manager->m_hbones,currentkeyframe(), m_mesh.m_bones.m_count));


	for(uint32_t i=0;i<m_mesh.m_submeshes.m_count;i++){

		app_throw_hr(_api_manager->m_d3ddevice->SetStreamSource(0, m_mesh.m_submeshes[i].m_vertex_buffer, 0, sizeof(_vertex)));
		app_throw_hr(_api_manager->m_d3ddevice->SetIndices(m_mesh.m_submeshes[i].m_index_buffer));

		app_throw_hr(_fx->Begin(NULL, 0));
		app_throw_hr(_fx->BeginPass(0));
		app_throw_hr(_api_manager->m_d3ddevice->DrawIndexedPrimitive(
			D3DPT_TRIANGLELIST, 0, 0,
			m_mesh.m_submeshes[i].m_vertices.m_count, 0,
			m_mesh.m_submeshes[i].m_indices.m_count/3));
		app_throw_hr(_fx->EndPass());
		app_throw_hr(_fx->End());
	}
	return true;
}

void  grayman::keyframe(const uint32_t&start,const uint32_t&end){

	if( ( end<m_mesh.m_keyframes.m_count ) && (end>start) ){
		m_start_keyframe = m_current_keyframe = start;
		m_end_keyframe   = end;
	}
}

D3DXMATRIX*  grayman::currentkeyframe(){

	if( ((m_end_keyframe - m_start_keyframe)>0) ){

		int32_t current_key;
		int32_t previous_key;

		if(m_current_keyframe == m_end_keyframe ){
			current_key  = m_start_keyframe;
			previous_key = m_current_keyframe;
		}
		else{
			current_key  = m_current_keyframe+1;
			previous_key = m_current_keyframe;
		}
		for(uint32_t i=0;i<m_mesh.m_bones.m_count;i++){
			float* buffer   = (float*) &( m_keyframe_buffer[i] );
			float* previous = (float*) &( m_mesh.m_keyframes[previous_key][i] );
			float* current  = (float*) &( m_mesh.m_keyframes[current_key][i] );
			for(uint32_t ii=0;ii<16;ii++){
				buffer[ii] = _lerp(previous[ii],current[ii],  m_animation_second/m_animation_length );
			}
		}
		return (D3DXMATRIX*)&(m_keyframe_buffer[0]);
	}
	return (D3DXMATRIX*)&(m_mesh.m_bones[0]);
}
