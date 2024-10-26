#include <windows.h>
#include <d3dx9.h>

#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")

LPDIRECT3D9 d3d;
LPDIRECT3DDEVICE9 g_pd3dDevice;
LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL;

struct CUSTOMVERTEX
{
    FLOAT x, y, z;
    DWORD color;
};

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE)

// Define vertices for a solid white cube (moved to global scope)
CUSTOMVERTEX vertices[] =
{
    // Front face
    { -10.0f, -10.0f, -10.0f, 0xffffffff }, // white
    {  10.0f, -10.0f, -10.0f, 0xffffffff }, // white
    {  10.0f,  10.0f, -10.0f, 0xffffffff }, // white
    { -10.0f,  10.0f, -10.0f, 0xffffffff }, // white

    // Back face
    { -10.0f, -10.0f,  10.0f, 0xffffffff }, // white
    {  10.0f, -10.0f,  10.0f, 0xffffffff }, // white
    {  10.0f,  10.0f,  10.0f, 0xffffffff }, // white
    { -10.0f,  10.0f,  10.0f, 0xffffffff }, // white
};

VOID InitD3D(HWND hWnd);
VOID Render();
VOID CleanUp();
LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
VOID SetupVertices();
VOID Matrix_Set();

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int)
{
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
                      GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                      L"Direct3D Tutorial", NULL };
    RegisterClassEx(&wc);
    HWND hWnd = CreateWindow(L"Direct3D Tutorial", L"3D Rotating Cube",
        WS_OVERLAPPEDWINDOW, 100, 100, 500, 500,
        NULL, NULL, wc.hInstance, NULL);

    InitD3D(hWnd);

    ShowWindow(hWnd, SW_SHOWDEFAULT);
    UpdateWindow(hWnd);

    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
            Render();
    }

    CleanUp();
    UnregisterClass(L"Direct3D Tutorial", wc.hInstance);
    return 0;
}

VOID InitD3D(HWND hWnd)
{
    d3d = Direct3DCreate9(D3D_SDK_VERSION);

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = hWnd;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.BackBufferCount = 1;
    d3dpp.BackBufferWidth = 500;
    d3dpp.BackBufferHeight = 500;

    d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING,
        &d3dpp, &g_pd3dDevice);

    g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);  // Disable lighting
    SetupVertices();
}

VOID SetupVertices()
{
    g_pd3dDevice->CreateVertexBuffer(8 * sizeof(CUSTOMVERTEX),
        0, D3DFVF_CUSTOMVERTEX,
        D3DPOOL_MANAGED, &g_pVB, NULL);

    VOID* pVertices;
    g_pVB->Lock(0, sizeof(vertices), (void**)&pVertices, 0);
    memcpy(pVertices, vertices, sizeof(vertices));
    g_pVB->Unlock();
}

VOID Matrix_Set()
{
    D3DXMATRIXA16 matView;
    D3DXVECTOR3 Eye(0.0f, 0.0f, -100.0f);
    D3DXVECTOR3 At(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 Up(0.0f, 1.0f, 0.0f);
    D3DXMatrixLookAtLH(&matView, &Eye, &At, &Up);
    g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);

    D3DXMATRIXA16 matProj;
    D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.0f, 1.0f, 1000.0f);
    g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);

    static float index = 0.0f;
    index += 0.001f;  // Slow rotation
    D3DXMATRIXA16 matWorld;
    D3DXMatrixRotationY(&matWorld, index);
    g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
}

VOID Render()
{
    if (NULL == g_pd3dDevice)
        return;

    // Clear the back buffer with black color
    g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
    g_pd3dDevice->BeginScene();

    Matrix_Set();

    g_pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));
    g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);

    // Draw cube faces as triangles
    short indices[] =
    {
        // Front face
        0, 1, 2,  0, 2, 3,
        // Back face
        4, 5, 6,  4, 6, 7,
        // Left face
        4, 0, 3,  4, 3, 7,
        // Right face
        1, 5, 6,  1, 6, 2,
        // Top face
        3, 2, 6,  3, 6, 7,
        // Bottom face
        4, 5, 1,  4, 1, 0
    };

    g_pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 8, 12, indices, D3DFMT_INDEX16, vertices, sizeof(CUSTOMVERTEX));

    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}

VOID CleanUp()
{
    if (g_pVB != NULL)
        g_pVB->Release();

    if (g_pd3dDevice != NULL)
        g_pd3dDevice->Release();

    if (d3d != NULL)
        d3d->Release();
}

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}
