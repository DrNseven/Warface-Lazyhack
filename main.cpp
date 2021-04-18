//D3D outline aimbot by n7

#include "main.h" //helper funcs here


typedef HRESULT(APIENTRY* EndScene) (IDirect3DDevice9*);
EndScene EndScene_orig = 0;

typedef HRESULT(APIENTRY* DrawIndexedPrimitive)(IDirect3DDevice9*, D3DPRIMITIVETYPE, INT, UINT, UINT, UINT, UINT);
DrawIndexedPrimitive DrawIndexedPrimitive_orig = 0;

typedef HRESULT(APIENTRY *SetVertexDeclaration)(IDirect3DDevice9*, IDirect3DVertexDeclaration9*);
SetVertexDeclaration SetVertexDeclaration_orig = 0;

typedef HRESULT(APIENTRY* SetStreamSource)(IDirect3DDevice9*, UINT, IDirect3DVertexBuffer9*, UINT, UINT);
SetStreamSource SetStreamSource_orig = 0;

typedef HRESULT(APIENTRY* Reset)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
Reset Reset_orig = 0;


typedef HRESULT(APIENTRY *SetVertexShaderConstantF)(IDirect3DDevice9*, UINT, const float*, UINT);
SetVertexShaderConstantF SetVertexShaderConstantF_orig = 0;

typedef HRESULT(APIENTRY *SetVertexShader)(IDirect3DDevice9*, IDirect3DVertexShader9*);
SetVertexShader SetVertexShader_orig = 0;

typedef HRESULT(APIENTRY *SetPixelShader)(IDirect3DDevice9*, IDirect3DPixelShader9*);;
SetPixelShader SetPixelShader_orig = 0;

typedef HRESULT(APIENTRY *DrawPrimitive)(IDirect3DDevice9*, D3DPRIMITIVETYPE, UINT, UINT);
DrawPrimitive DrawPrimitive_orig = 0;

typedef HRESULT(APIENTRY *SetTexture)(IDirect3DDevice9*, DWORD, IDirect3DBaseTexture9 *);
SetTexture SetTexture_orig = 0;

typedef HRESULT(APIENTRY* Present) (IDirect3DDevice9*, const RECT *, const RECT *, HWND, const RGNDATA *);
Present Present_orig = 0;

//=====================================================================================================================

HRESULT APIENTRY EndScene_hook(IDirect3DDevice9* pDevice)
{
	if (pDevice == nullptr) return EndScene_orig(pDevice);

	if (!initonce)
	{
		//get viewport
		pDevice->GetViewport(&Viewport);
		ScreenCX = (float)Viewport.Width / 2.0f;
		ScreenCY = (float)Viewport.Height / 2.0f;

		//generate texture
		GenerateTexture(pDevice, &Red, D3DCOLOR_ARGB(255, 255, 0, 0));
		GenerateTexture(pDevice, &Green, D3DCOLOR_RGBA(0, 255, 0, 255));
		GenerateTexture(pDevice, &Blue, D3DCOLOR_ARGB(255, 0, 0, 255));
		GenerateTexture(pDevice, &Yellow, D3DCOLOR_ARGB(255, 255, 255, 0));
		GenerateTexture(pDevice, &Magenta, D3DCOLOR_ARGB(255, 255, 0, 255));

		//generate shader
		GenerateShader(pDevice, &shadRed, 1.0f, 0.0f, 0.0f, 1.0f, false);
		GenerateShader(pDevice, &shadGreen, 0.0f, 1.0f, 0.0f, 1.0f, false);
		GenerateShader(pDevice, &shadDarkRed, 0.1f, 0.0f, 0.0f, 0.1f, false);
		GenerateShader(pDevice, &shadDarkGreen, 0.0f, 0.1f, 0.0f, 0.1f, false);
		GenerateShader(pDevice, &shadBlue, 0.0f, 0.0f, 1.0f, 0.5f, true);
		GenerateShader(pDevice, &shadYellow, 1.0f, 1.0f, 0.0f, 0.5f, true);

		//load cfg settings
		LoadCfg();

		initonce = true;
	}

	//get screen again if pressing esc
	if (GetAsyncKeyState(VK_ESCAPE) & 1)
	{
		//get viewport
		pDevice->GetViewport(&Viewport);
		ScreenCX = (float)Viewport.Width / 2.0f;
		ScreenCY = (float)Viewport.Height / 2.0f;

		SaveCfg(); //save settings
	}

	//no menu yet
	if (GetAsyncKeyState(VK_INSERT) & 1) {
		SaveCfg(); //save settings

		//no menu yet
		//showmenu = !showmenu;
	}

	//create sprite
	if (!SpriteCreated)
		CreateSprite(pDevice);

	//test draw sprite
	DrawPic(pDevice, SpriteTexture1, 100, 100);


	//Shift|RMouse|LMouse|Ctrl|Alt|Space|X|C
	if (aimkey == 0) Daimkey = 0;
	if (aimkey == 1) Daimkey = VK_SHIFT;
	if (aimkey == 2) Daimkey = VK_RBUTTON;
	if (aimkey == 3) Daimkey = VK_LBUTTON;
	if (aimkey == 4) Daimkey = VK_CONTROL;
	if (aimkey == 5) Daimkey = VK_MENU;
	if (aimkey == 6) Daimkey = VK_SPACE;
	if (aimkey == 7) Daimkey = 0x58; //X
	if (aimkey == 8) Daimkey = 0x43; //C


	//do outline esp
	if (esp == 1)
		if (OutlineInfo.size() != NULL)
		{
			for (unsigned int i = 0; i < OutlineInfo.size(); i++)
			{
				if (OutlineInfo[i].pOutX > 1.0f && OutlineInfo[i].pOutY > 1.0f)
				{
					//pic esp
					//DrawPic(pDevice, SpriteTexture1, OutlineInfo[i].pOutX, OutlineInfo[i].pOutY);
				}
			}
		}

	//do outline aim
	if (aimbot == 1)
		if (OutlineInfo.size() != NULL)
		{
			UINT BestTarget = -1;
			DOUBLE fClosestPos = 99999;

			for (unsigned int i = 0; i < OutlineInfo.size(); i++)
			{
				//aimfov
				float radiusx = (aimfov * 5.0f) * (ScreenCX / 100.0f);
				float radiusy = (aimfov * 5.0f) * (ScreenCY / 100.0f);

				//get crosshairdistance
				OutlineInfo[i].CrosshairDistance = GetDistance(OutlineInfo[i].pOutX, OutlineInfo[i].pOutY, ScreenCX, ScreenCY);

				//if in fov
				if (OutlineInfo[i].pOutX >= ScreenCX - radiusx && OutlineInfo[i].pOutX <= ScreenCX + radiusx && OutlineInfo[i].pOutY >= ScreenCY - radiusy && OutlineInfo[i].pOutY <= ScreenCY + radiusy)

					//get closest/nearest target to crosshair
					if (OutlineInfo[i].CrosshairDistance < fClosestPos)
					{
						fClosestPos = OutlineInfo[i].CrosshairDistance;
						BestTarget = i;
					}
			}

			//if nearest target to crosshair
			if (BestTarget != -1)
			{
				//get outline coordinate
				outline_x = OutlineInfo[BestTarget].pOutX;
				outline_y = OutlineInfo[BestTarget].pOutY;
				/*
				//DrawPic(pDevice, SpriteTexture1, OutlineInfo[BestTarget].pOutX, OutlineInfo[BestTarget].pOutY);

				double DistX = OutlineInfo[BestTarget].pOutX - ScreenCX;
				double DistY = OutlineInfo[BestTarget].pOutY - ScreenCY;

				DistX /= (float)aimsens;
				DistY /= (float)aimsens;

				//aim
				if (GetKeyState(Daimkey) & 0x8000)
					mouse_event(MOUSEEVENTF_MOVE, (float)DistX, (float)DistY, 0, NULL);
				*/
			}
		}
	OutlineInfo.clear();



	//do model esp
	if (esp == 1)
	if (ModelInfo.size() != NULL)
	{
		for (unsigned int i = 0; i < ModelInfo.size(); i++)
		{
			if (ModelInfo[i].pOutX > 1.0f && ModelInfo[i].pOutY > 1.0f)
			{
				//pic esp
				DrawPic(pDevice, SpriteTexture1, ModelInfo[i].pOutX, ModelInfo[i].pOutY);
			}
		}
	}

	//do model aim
	if (aimbot == 1)
	if (ModelInfo.size() != NULL)
	{
		UINT BestTarget = -1;
		DOUBLE fClosestPos = 99999;

		for (unsigned int i = 0; i < ModelInfo.size(); i++)
		{
			//aimfov
			float radiusx = (aimfov*5.0f) * (ScreenCX / 100.0f);
			float radiusy = (aimfov*5.0f) * (ScreenCY / 100.0f);

			//get crosshairdistance
			ModelInfo[i].CrosshairDistance = GetDistance(ModelInfo[i].pOutX, ModelInfo[i].pOutY, ScreenCX, ScreenCY);

			//if in fov
			if (ModelInfo[i].pOutX >= ScreenCX - radiusx && ModelInfo[i].pOutX <= ScreenCX + radiusx && ModelInfo[i].pOutY >= ScreenCY - radiusy && ModelInfo[i].pOutY <= ScreenCY + radiusy)

				//get closest/nearest target to crosshair
				if (ModelInfo[i].CrosshairDistance < fClosestPos)
				{
					fClosestPos = ModelInfo[i].CrosshairDistance;
					BestTarget = i;
				}
		}

		//if nearest target to crosshair
		if (BestTarget != -1)
		{
			//do not aim at team members (only works 50% of the time, worldtoscreen issue?)
			if (outline_x != (int)ModelInfo[BestTarget].pOutX && outline_y != (int)ModelInfo[BestTarget].pOutY)//for PVP: if outline coordinate is not model coordinate then it is an enemy model
			//if (outline_x == (int)ModelInfo[BestTarget].pOutX && outline_y == (int)ModelInfo[BestTarget].pOutY)//PVE: if outline coordinate is model coordinate then it is not a corpse
			{
				//draw pic on main target
				DrawPic(pDevice, SpriteTexture1, ModelInfo[BestTarget].pOutX, ModelInfo[BestTarget].pOutY);

				double DistX = ModelInfo[BestTarget].pOutX - ScreenCX;
				double DistY = ModelInfo[BestTarget].pOutY - ScreenCY;

				DistX /= (float)aimsens;
				DistY /= (float)aimsens;

				//aim here
				if (GetKeyState(Daimkey) & 0x8000)
					mouse_event(MOUSEEVENTF_MOVE, (float)DistX, (float)DistY, 0, NULL);
			}
		}
	}
	ModelInfo.clear();


	return EndScene_orig(pDevice);
}

//==========================================================================================================================

HRESULT APIENTRY DrawIndexedPrimitive_hook(IDirect3DDevice9* pDevice, D3DPRIMITIVETYPE Type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
	//wallhack
	if(wallhack == 1)
	if(numElements == 7)//models
	{
		//chams
		//pDevice->SetTexture(0, Red);
		//pDevice->SetTexture(1, Red);
		//pDevice->SetPixelShader(shadRed);
		pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
		DrawIndexedPrimitive_orig(pDevice, Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
		pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
		//chams
		//pDevice->SetTexture(0, Green);
		//pDevice->SetTexture(1, Green);
		//pDevice->SetPixelShader(shadGreen);
	}


	//outline
	//Stride == 24 && NumVertices == 1196 && primCount == 2250 //worldtoscreen will work with this
	//?Stride == 16 && NumVertices == 1198 && primCount == 626 //unk/no

	//idea: if outline and model coordinates are the same then it is a friendly model in pvp, do not aim
	//1. get outline coordinates of nearest target to crosshair
	//2. compare outline_xy coordinate to model_xy
	//3. if both are not the same then it is an enemy team member, do aim

	//check outline 
	if (aimbot == 1 || esp == 1)
	if (NumVertices == 1196 && primCount == 2250)//outline
		AddOutlines(pDevice);


	//check models wf & bw
	if(aimbot == 1 || esp == 1)
	if(numElements == 7)//models
		AddModels(pDevice);


	//erase outlines
	//if (NumVertices == 1196 && primCount == 2250)//outline
		//return D3D_OK;//delete

	return DrawIndexedPrimitive_orig(pDevice, Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
}

//==========================================================================================================================

HRESULT APIENTRY Reset_hook(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	DeleteSprite();

	if (Sprite1)
		Sprite1->OnLostDevice();

	HRESULT ResetReturn = Reset_orig(pDevice, pPresentationParameters);

	//if (SUCCEEDED(ResetReturn))
	//{
		if (Sprite1)
			Sprite1->OnResetDevice();
	//}

	return ResetReturn;
}

//==========================================================================================================================

HRESULT APIENTRY SetStreamSource_hook(LPDIRECT3DDEVICE9 pDevice, UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT sStride)
{
	if (StreamNumber == 0)
		Stride = sStride;

	return SetStreamSource_orig(pDevice, StreamNumber, pStreamData, OffsetInBytes, sStride);
}

//==========================================================================================================================

HRESULT APIENTRY SetVertexDeclaration_hook(LPDIRECT3DDEVICE9 pDevice, IDirect3DVertexDeclaration9* pdecl)
{
	if (pdecl)
	{
		pdecl->GetDeclaration(decl, &numElements);
	}

	return SetVertexDeclaration_orig(pDevice, pdecl);
}

//==========================================================================================================================

HRESULT APIENTRY SetVertexShaderConstantF_hook(LPDIRECT3DDEVICE9 pDevice, UINT StartRegister, const float *pConstantData, UINT Vector4fCount)
{
	if (pConstantData)
	{
		mStartregister = StartRegister;
		mVectorCount = Vector4fCount;
	}

	return SetVertexShaderConstantF_orig(pDevice, StartRegister, pConstantData, Vector4fCount);
}

//==========================================================================================================================

HRESULT APIENTRY SetVertexShader_hook(LPDIRECT3DDEVICE9 pDevice, IDirect3DVertexShader9 *veShader)
{
	if (veShader)
	{
		vShader = veShader;
		vShader->GetFunction(NULL, &vSize);
	}
	return SetVertexShader_orig(pDevice, veShader);
}

//==========================================================================================================================

HRESULT APIENTRY SetPixelShader_hook(LPDIRECT3DDEVICE9 pDevice, IDirect3DPixelShader9 *piShader)
{
	if (piShader)
	{
		pShader = piShader;
		pShader->GetFunction(NULL, &pSize);
	}
	return SetPixelShader_orig(pDevice, piShader);
}

//==========================================================================================================================

HRESULT APIENTRY DrawPrimitive_hook(IDirect3DDevice9* pDevice, D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
	return DrawPrimitive_orig(pDevice, PrimitiveType, StartVertex, PrimitiveCount);
}

//==========================================================================================================================

HRESULT APIENTRY SetTexture_hook(LPDIRECT3DDEVICE9 pDevice, DWORD Sampler, IDirect3DBaseTexture9 *pTexture)
{
	return SetTexture_orig(pDevice, Sampler, pTexture);
}

//==========================================================================================================================

HRESULT APIENTRY Present_hook(IDirect3DDevice9* pDevice, const RECT *pSourceRect, const RECT *pDestRect, HWND hDestWindowOverride, const RGNDATA *pDirtyRegion)
{
	return Present_orig(pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

//=====================================================================================================================

DWORD WINAPI Lazyhack(LPVOID lpParameter)
{
	while (!GetModuleHandleA("d3d9.dll")) {
		Sleep(200);
	}

	IDirect3D9* d3d = NULL;
	IDirect3DDevice9* d3ddev = NULL;

	HWND tmpWnd = CreateWindowA("BUTTON", "O", WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 300, 300, NULL, NULL, Hand, NULL);
	if (tmpWnd == NULL)
	{
		//Log("[DirectX] Failed to create temp window");
		return 0;
	}

	d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (d3d == NULL)
	{
		DestroyWindow(tmpWnd);
		//Log("[DirectX] Failed to create temp Direct3D interface");
		return 0;
	}

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = tmpWnd;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

	HRESULT result = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, tmpWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3ddev);
	if (result != D3D_OK)
	{
		d3d->Release();
		DestroyWindow(tmpWnd);
		Log("[DirectX] Failed to create temp Direct3D device. Run the game first and inject dll later"); 
		return 0;
	}

	// We have the device, so walk the vtable to get the address of all the dx functions in d3d9.dll
#if defined _M_X64
	DWORD64* dVtable = (DWORD64*)d3ddev;
	dVtable = (DWORD64*)dVtable[0];
#elif defined _M_IX86
	DWORD* dVtable = (DWORD*)d3ddev;
	dVtable = (DWORD*)dVtable[0]; // == *d3ddev
#endif
	//Log("[DirectX] dVtable: %x", dVtable);

	//for(int i = 0; i < 95; i++)
	//{
			//Log("[DirectX] vtable[%i]: %x, pointer at %x", i, dVtable[i], &dVtable[i]);
	//}

	// Detour functions x86 & x64
	EndScene_orig = (EndScene)dVtable[42];
	DrawIndexedPrimitive_orig = (DrawIndexedPrimitive)dVtable[82];
	SetVertexDeclaration_orig = (SetVertexDeclaration)dVtable[87];
	SetStreamSource_orig = (SetStreamSource)dVtable[100];
	Reset_orig = (Reset)dVtable[16];
	SetVertexShaderConstantF_orig = (SetVertexShaderConstantF)dVtable[94];
	//SetVertexShader_orig = (SetVertexShader)dVtable[92];
	//SetPixelShader_orig = (SetPixelShader)dVtable[107];
	//DrawPrimitive_orig = (DrawPrimitive)dVtable[81];
	//SetTexture_orig = (SetTexture)dVtable[65];
	//Present_orig = (Present)dVtable[17];
	
	Sleep(2000);//required in a few games

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(LPVOID&)EndScene_orig, (PBYTE)EndScene_hook);
	DetourAttach(&(LPVOID&)DrawIndexedPrimitive_orig, (PBYTE)DrawIndexedPrimitive_hook);
	DetourAttach(&(LPVOID&)SetVertexDeclaration_orig, (PBYTE)SetVertexDeclaration_hook);
	DetourAttach(&(LPVOID&)SetStreamSource_orig, (PBYTE)SetStreamSource_hook);
	DetourAttach(&(LPVOID&)Reset_orig, (PBYTE)Reset_hook);
	DetourAttach(&(LPVOID&)SetVertexShaderConstantF_orig, (PBYTE)SetVertexShaderConstantF_hook);
	//DetourAttach(&(LPVOID&)SetVertexShader_orig, (PBYTE)SetVertexShader_hook);
	//DetourAttach(&(LPVOID&)SetPixelShader_orig, (PBYTE)SetPixelShader_hook);
	//DetourAttach(&(LPVOID&)DrawPrimitive_orig, (PBYTE)DrawPrimitive_hook);
	//DetourAttach(&(LPVOID&)SetTexture_orig, (PBYTE)SetTexture_hook);
	//DetourAttach(&(LPVOID&)Present_orig, (PBYTE)Present_hook);
	DetourTransactionCommit();
	
	//Log("[Detours] Detours attached\n");

	d3ddev->Release();
	d3d->Release();
	DestroyWindow(tmpWnd);

	return 1;
}

//==========================================================================================================================

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		Hand = hModule;
		DisableThreadLibraryCalls(hModule); //disable unwanted thread notifications to reduce overhead
		GetModuleFileNameA(hModule, dlldir, 512);
		for (int i = (int)strlen(dlldir); i > 0; i--)
		{
			if (dlldir[i] == '\\')
			{
				dlldir[i + 1] = 0;
				break;
			}
		}
		CreateThread(0, 0, Lazyhack, 0, 0, 0); //init our hooks

		break;
	case DLL_PROCESS_DETACH:
		//Uninitialize 
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}
