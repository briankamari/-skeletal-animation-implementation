#ifndef D3D_MANAGER_H
#define D3D_MANAGER_H

#include "appdefs.h"

#include <d3d9.h>
#include <d3dx9.h>

struct d3d_manager{

    d3d_manager();
    bool init();
    void clear();

    bool reset();
    bool buildfx();

    ID3DXEffect* m_fx;

    D3DPRESENT_PARAMETERS m_d3dpp;

    IDirect3D9*           m_d3dobject;
    IDirect3DDevice9*     m_d3ddevice;
    IDirect3DVertexDeclaration9* m_vertex_declaration;

    D3DXHANDLE   m_hmvp;
    D3DXHANDLE   m_htech;
    D3DXHANDLE   m_hworld;
    D3DXHANDLE   m_hbones;

    static d3d_manager* _manager;

};

#endif // D3D_MANAGER_H
