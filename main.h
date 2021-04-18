#include <windows.h>
#include <fstream>
#include <string>
#include <vector>
#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "winmm.lib")//time

//detours
#include "detours.h"
#if defined _M_X64
#pragma comment(lib, "detours.X64/detours.lib")
#elif defined _M_IX86
#pragma comment(lib, "detours.X86/detours.lib")
#endif

//DX Includes
//#include <DirectXMath.h>
//using namespace DirectX;

//dx sdk dir
#include <d3d9.h>
#include "DXSDK\d3dx9.h"
#if defined _M_X64
#pragma comment(lib, "DXSDK/x64/d3dx9.lib") 
#elif defined _M_IX86
#pragma comment(lib, "DXSDK/x86/d3dx9.lib")
#endif

#pragma warning (disable: 4244) 

//==========================================================================================================================//

//features & item states
int wallhack = 1;	
//bool chams = 0;
int aimbot = 1;
int esp = 1;
int aimkey = 1;
DWORD Daimkey = VK_SHIFT;
int aimsens = 1;
int aimfov = 3;
int aimheight = 4;
//int autoshoot = 1;

//==========================================================================================================================//

//globals
HMODULE Hand;

UINT Stride;
D3DVERTEXBUFFER_DESC vdesc;

IDirect3DVertexDeclaration9* pDecl;
D3DVERTEXELEMENT9 decl[MAXD3DDECLLENGTH];
UINT numElements;

UINT mStartregister;
UINT mVectorCount;

IDirect3DVertexShader9* vShader;
UINT vSize;

IDirect3DPixelShader9* pShader;
UINT pSize;

IDirect3DBaseTexture9* pTexture;
IDirect3DTexture9* pCurrentTex;

IDirect3DTexture9 *texture;
D3DSURFACE_DESC sDesc;
DWORD qCRC;
DWORD qCRC2;
D3DLOCKED_RECT pLockedRect;

D3DVIEWPORT9 Viewport;
float ScreenCX;
float ScreenCY;

LPDIRECT3DTEXTURE9 Red, Green, Blue, Yellow, Magenta;

IDirect3DPixelShader9* shadRed;
IDirect3DPixelShader9* shadGreen;
IDirect3DPixelShader9* shadDarkRed;
IDirect3DPixelShader9* shadDarkGreen;
IDirect3DPixelShader9* shadBlue;
IDirect3DPixelShader9* shadYellow;

bool initonce = false;

int outline_x = -1; //outline coordinates
int outline_y = -1;

//==========================================================================================================================//

// getdir & log
using namespace std;
char dlldir[512];
char* GetDirFile(char *name)
{
	static char pldir[512];
	strcpy_s(pldir, dlldir);
	strcat_s(pldir, name);
	return pldir;
}

void Log(const char *fmt, ...)
{
	if (!fmt)	return;

	char		text[4096];
	va_list		ap;
	va_start(ap, fmt);
	vsprintf_s(text, fmt, ap);
	va_end(ap);

	ofstream logfile(GetDirFile((PCHAR)"log.txt"), ios::app);
	if (logfile.is_open() && text)	logfile << text << endl;
	logfile.close();
}

//==========================================================================================================================//

void SaveCfg()
{
	ofstream fout;
	fout.open(GetDirFile((PCHAR)"picaim.ini"), ios::trunc);
	fout << "wallhack " << wallhack << endl;
	//fout << "chams " << chams << endl;
	fout << "aimbot " << aimbot << endl;
	fout << "esp " << esp << endl;
	fout << "aimkey " << aimkey << endl;
	fout << "aimsens " << aimsens << endl;
	fout << "aimfov " << aimfov << endl;
	fout << "aimheight " << aimheight << endl;
	//fout << "autoshoot " << autoshoot << endl;
	fout.close();
}

void LoadCfg()
{
	ifstream fin;
	string Word = "";
	fin.open(GetDirFile((PCHAR)"picaim.ini"), ifstream::in);
	fin >> Word >> wallhack;
	//fin >> Word >> chams;
	fin >> Word >> aimbot;
	fin >> Word >> esp;
	fin >> Word >> aimkey;
	fin >> Word >> aimsens;
	fin >> Word >> aimfov;
	fin >> Word >> aimheight;
	//fin >> Word >> autoshoot;
	fin.close();
}

//==========================================================================================================================//

//calc distance
float GetDistance(float Xx, float Yy, float xX, float yY)
{
	return sqrt((yY - Yy) * (yY - Yy) + (xX - Xx) * (xX - Xx));
}

struct ModelInfo_t
{
	float pOutX, pOutY, RealDistance;
	float CrosshairDistance;
};
std::vector<ModelInfo_t>ModelInfo;

//w2s
void AddModels(LPDIRECT3DDEVICE9 Device)
{
	float xx, yy;
	D3DXMATRIX matrix, worldmatrix, m1;
	D3DXVECTOR4 position, input;

	Device->GetVertexShaderConstantF(36, m1, 4);//36, 4
	Device->GetVertexShaderConstantF(0, worldmatrix, 3);//0, 3

	input.x = worldmatrix._14;
	input.y = worldmatrix._24;
	input.z = worldmatrix._34 +(float)aimheight / 5.0f;
	input.w = 1.0f;

	D3DXMatrixTranspose(&matrix, &m1);
	//D3DXVec4Transform(&position, &input, &matrix);

	position.x = input.x * matrix._11 + input.y * matrix._21 + input.z * matrix._31 + matrix._41;
	position.y = input.x * matrix._12 + input.y * matrix._22 + input.z * matrix._32 + matrix._42;
	position.z = input.x * matrix._13 + input.y * matrix._23 + input.z * matrix._33 + matrix._43;
	position.w = input.x * matrix._14 + input.y * matrix._24 + input.z * matrix._34 + matrix._44;

	//RealDistance = Viewport.MinZ + position.z * (Viewport.MaxZ - Viewport.MinZ); //real distance

	if(position.w >= 1)
	{
	xx = ((position.x / position.w) * (Viewport.Width / 2.0f)) + Viewport.X + (Viewport.Width / 2.0f);
	yy = Viewport.Y + (Viewport.Height / 2.0f) - ((position.y / position.w) * (Viewport.Height / 2.0f));
	}
	else
	{
		xx = -1;
		yy = -1;
	}

	ModelInfo_t pModelInfo = { static_cast<float>(xx), static_cast<float>(yy), static_cast<float>(position.w) };
	ModelInfo.push_back(pModelInfo);
}

//==========================================================================================================================//

struct OutlineInfo_t
{
	float pOutX, pOutY, RealDistance;
	float CrosshairDistance;
};
std::vector<OutlineInfo_t>OutlineInfo;

//w2s
void AddOutlines(LPDIRECT3DDEVICE9 Device)
{
	float xx, yy;
	D3DXMATRIX matrix, worldmatrix, m1;
	D3DXVECTOR4 position, input;

	Device->GetVertexShaderConstantF(36, m1, 4);//36, 4
	Device->GetVertexShaderConstantF(0, worldmatrix, 3);//0, 3

	input.x = worldmatrix._14;
	input.y = worldmatrix._24;
	input.z = worldmatrix._34 + (float)aimheight / 5.0f;
	input.w = 1.0f;

	D3DXMatrixTranspose(&matrix, &m1);
	//D3DXVec4Transform(&position, &input, &matrix);

	position.x = input.x * matrix._11 + input.y * matrix._21 + input.z * matrix._31 + matrix._41;
	position.y = input.x * matrix._12 + input.y * matrix._22 + input.z * matrix._32 + matrix._42;
	position.z = input.x * matrix._13 + input.y * matrix._23 + input.z * matrix._33 + matrix._43;
	position.w = input.x * matrix._14 + input.y * matrix._24 + input.z * matrix._34 + matrix._44;

	//RealDistance = Viewport.MinZ + position.z * (Viewport.MaxZ - Viewport.MinZ); //real distance

	if (position.w >= 1)
	{
		xx = ((position.x / position.w) * (Viewport.Width / 2.0f)) + Viewport.X + (Viewport.Width / 2.0f);
		yy = Viewport.Y + (Viewport.Height / 2.0f) - ((position.y / position.w) * (Viewport.Height / 2.0f));
	}
	else
	{
		xx = -1;
		yy = -1;
	}

	OutlineInfo_t pOutlineInfo = { static_cast<float>(xx), static_cast<float>(yy), static_cast<float>(position.w) };
	OutlineInfo.push_back(pOutlineInfo);
}

//==========================================================================================================================//

HRESULT GenerateTexture(IDirect3DDevice9 *pDevice, IDirect3DTexture9 **ppD3Dtex, DWORD colour32)
{
	if (FAILED(pDevice->CreateTexture(8, 8, 1, 0, D3DFMT_A4R4G4B4, D3DPOOL_MANAGED, ppD3Dtex, NULL)))
		return E_FAIL;

	WORD colour16 = ((WORD)((colour32 >> 28) & 0xF) << 12)
		| (WORD)(((colour32 >> 20) & 0xF) << 8)
		| (WORD)(((colour32 >> 12) & 0xF) << 4)
		| (WORD)(((colour32 >> 4) & 0xF) << 0);

	D3DLOCKED_RECT d3dlr;
	(*ppD3Dtex)->LockRect(0, &d3dlr, 0, 0);
	WORD *pDst16 = (WORD*)d3dlr.pBits;

	for (int xy = 0; xy < 8 * 8; xy++)
		*pDst16++ = colour16;

	(*ppD3Dtex)->UnlockRect(0);

	return S_OK;
}

//generate shader
HRESULT GenerateShader(IDirect3DDevice9* pDevice, IDirect3DPixelShader9** pShader, float r, float g, float b, float a, bool setzBuf)
{
	char szShader[256];
	ID3DXBuffer* pShaderBuf = NULL;
	D3DCAPS9 caps;
	pDevice->GetDeviceCaps(&caps);
	int PXSHVER1 = (D3DSHADER_VERSION_MAJOR(caps.PixelShaderVersion));
	int PXSHVER2 = (D3DSHADER_VERSION_MINOR(caps.PixelShaderVersion));
	if (setzBuf)
		sprintf_s(szShader, "ps.%d.%d\ndef c0, %f, %f, %f, %f\nmov oC0,c0\nmov oDepth, c0.x", PXSHVER1, PXSHVER2, r, g, b, a);
	else
		sprintf_s(szShader, "ps.%d.%d\ndef c0, %f, %f, %f, %f\nmov oC0,c0", PXSHVER1, PXSHVER2, r, g, b, a);
	D3DXAssembleShader(szShader, sizeof(szShader), NULL, NULL, 0, &pShaderBuf, NULL);
	if (FAILED(pDevice->CreatePixelShader((const DWORD*)pShaderBuf->GetBufferPointer(), pShader)))return E_FAIL;
	return S_OK;
}

//==========================================================================================================================//

// sprite
LPD3DXSPRITE Sprite1 = NULL;
LPDIRECT3DTEXTURE9 SpriteTexture1 = NULL;
bool SpriteCreated = false;

// COM utils
template<class COMObject>
void SafeRelease(COMObject*& pResa)
{
	IUnknown* unknowna = pResa;
	if (unknowna)
	{
		unknowna->Release();
	}
	pResa = NULL;
}

// create sprite
bool CreateSprite(IDirect3DDevice9* pDevice)
{
	HRESULT hra;

	//hra = D3DXCreateTextureFromFileExA(pDevice, GetDirFile((PCHAR)"circle.png"), D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &SpriteTexture1);
	
	hra = D3DXCreateTextureFromFileA(pDevice, GetDirFile((PCHAR)"circle.png"), &SpriteTexture1); //circle.png in release directory
	//hra = D3DXCreateTextureFromFileA(pDevice, "C:\\ .. something \\Steam\\steamapps\\common\\Warface\\ .. number ..\\Bin64Release\\circle.png", &SpriteTexture1); //or load circle.png from specific directory

	//Log("GetDirFile == %s", GetDirFile((PCHAR)"circle.png"));

	if (FAILED(hra))
	{
		//Log("D3DXCreateTextureFromFile failed");
		SpriteCreated = false;
		return false;
	}

	hra = D3DXCreateSprite(pDevice, &Sprite1);

	if (FAILED(hra))
	{
		//Log("D3DXCreateSprite failed");
		SpriteCreated = false;
		return false;
	}

	SpriteCreated = true;

	return true;
}

// Delete work surfaces when device gets reset
void DeleteSprite()
{
	if (Sprite1 != NULL)
	{
		//Log("SafeRelease(Sprite1)");
		SafeRelease(Sprite1);
	}

	SpriteCreated = false;
}

// Draw Sprite
void DrawPic(IDirect3DDevice9* pDevice, IDirect3DTexture9* tex, int cx, int cy)
{
	if (SpriteCreated && Sprite1 != NULL && cx < (int)Viewport.Width && cy < (int)Viewport.Height)
	{
		//position = Pic Size in pixel / 2
		//64 -> 32
		D3DXVECTOR3 position;
		position.x = (float)cx - 32.0f;
		position.y = (float)cy - 32.0f;
		position.z = 0.0f;

		//draw pic
		Sprite1->Begin(D3DXSPRITE_ALPHABLEND);
		Sprite1->Draw(tex, NULL, NULL, &position, 0xFFFFFFFF);
		Sprite1->End();
	}
}
