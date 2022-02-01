// D3D9Hook
// This D3D9 hook is mostly universal across games that use DirectX 9.
// It does not need to be updated or maintained whatsoever.

// Currently does not support x64 architecture.
// Does not evade *all* anti-cheats, there are still various methods game developers could use to detect this.

#include "D3D9Hook.hpp"

IDirect3DDevice9* D3D9Device = nullptr;

bool WireFrameEnabled = false;
bool hs = false;

void FixCallsAndJumps(uintptr_t CopiedFunction, uintptr_t OriginalFunction, size_t FuncSize)
{
	uint8_t* byte = reinterpret_cast<uint8_t*>(CopiedFunction);
	for (size_t i = 0; i < FuncSize; ++i)
	{
		if (byte[i] == 0xE8) // call
		{
			uintptr_t addr = OriginalFunction + ((CopiedFunction + i) - CopiedFunction);
			uintptr_t origcalladdr = (addr + *reinterpret_cast<uintptr_t*>(addr + 1)) + 5;
			uintptr_t reladdr = origcalladdr - (CopiedFunction + i) - 5;
			*reinterpret_cast<uintptr_t*>((CopiedFunction + i) + 1) = reladdr;
			i = i + 0x4;
		}
		else if (byte[i] == 0xE9) // unconditional jump
		{
			uintptr_t addr = OriginalFunction + ((CopiedFunction + i) - CopiedFunction);
			uintptr_t origcalladdr = (addr + *reinterpret_cast<uintptr_t*>(addr + 1)) + 5;
			uintptr_t reladdr = origcalladdr - (CopiedFunction + i) - 5;
			*reinterpret_cast<uintptr_t*>((CopiedFunction + i) + 1) = reladdr;
			i = i + 0x4;
		}
		else if (byte[i] == 0xFF && (byte[i] + 1) == 0x15) // call dword ptr
		{
			uintptr_t addr = OriginalFunction + ((CopiedFunction + i + 1) - CopiedFunction);
			uintptr_t origcalladdr = (addr + *reinterpret_cast<uintptr_t*>(addr + 1)) + 5;
			uintptr_t reladdr = origcalladdr - (CopiedFunction + i + 1) - 5;
			*reinterpret_cast<uintptr_t*>((CopiedFunction + i) + 2) = reladdr;
			i = i + 0x4;
		}
		else if (byte[i] == 0x0F) // far jumps
		{
			uintptr_t addr = OriginalFunction + ((CopiedFunction + i + 1) - CopiedFunction);
			uintptr_t origcalladdr = (addr + *reinterpret_cast<uintptr_t*>(addr + 1)) + 5;
			uintptr_t reladdr = origcalladdr - (CopiedFunction + i + 1) - 5;
			*reinterpret_cast<uintptr_t*>((CopiedFunction + i) + 2) = reladdr;
			i = i + 0x4;
		}
	}
}

size_t CalculateFunctionSize(uintptr_t VA)
{
	uint8_t* byte = reinterpret_cast<uint8_t*>(VA);

	while (!(byte[0] == 0xC3 && byte[1] != 0xCC))
		byte += 0x10;

	return reinterpret_cast<size_t>(byte) - VA;
}

uintptr_t Trampoline(uintptr_t OriginalFunction, uintptr_t HookFunction)
{
	size_t fSize = CalculateFunctionSize(OriginalFunction);
	uintptr_t tAddr = reinterpret_cast<uintptr_t>(VirtualAlloc(nullptr, fSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
	memcpy_s(reinterpret_cast<void*>(tAddr), fSize, reinterpret_cast<void*>(OriginalFunction), fSize);
	FixCallsAndJumps(tAddr, OriginalFunction, fSize);
	DWORD OP{};
	dVirtualProtect(reinterpret_cast<void*>(OriginalFunction), 5, PAGE_EXECUTE_READWRITE, &OP);
	*reinterpret_cast<BYTE*>(OriginalFunction) = 0xE9;
	*reinterpret_cast<uintptr_t*>(OriginalFunction + 1) = HookFunction - OriginalFunction - 5;
	dVirtualProtect(reinterpret_cast<void*>(OriginalFunction), 5, OP, &OP);
	return tAddr;
}

HRESULT __stdcall D3D9EndSceneHook(IDirect3DDevice9* pDevice)
{
	switch (hs)
	{
	case false:
		D3D9Device = pDevice;
		std::cout << "D3D9 Device: 0x" << std::hex << D3D9Device << std::endl;
		hs = true;
	}
#if D3D9DEBUG
	std::cout << "EndScene hooked: 0x" << std::hex << D3D9EndSceneHook << std::endl;
#endif

	return oD3D9EndScene(pDevice);
}

HRESULT __stdcall D3D9DrawPrimitiveHook(IDirect3DDevice9* pDevice, D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
#if D3D9DEBUG
	std::cout << "DrawPrimitive hooked: 0x" << std::hex << D3D9DrawPrimitive << std::endl;
#endif
	return oD3D9DrawPrimitive(pDevice, PrimitiveType, StartVertex, PrimitiveCount);
}

HRESULT __stdcall D3D9DrawIndexedPrimitiveHook(IDirect3DDevice9* pDevice, D3DPRIMITIVETYPE nn, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
	if (WireFrameEnabled)
	{
		pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	}
	else
	{
		pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		//pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	}
#if D3D9DEBUG
	std::cout << "DrawIndexedPrimitive hooked: 0x" << std::hex << D3D9DrawIndexedPrimitiveHook << std::endl;
#endif

	return oD3D9DrawIndexedPrimitive(pDevice, nn, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
}

bool CreateDummyDevice()
{
	std::cout << "Creating DummyDevice" << std::endl;
	IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION); // create a d3d instance for us to work with
	std::cout << "pD3D created" << std::endl;

	IDirect3DDevice9* DummyDevice = nullptr;

	D3DPRESENT_PARAMETERS DeviceParameters{};
	ZeroMemory(&DeviceParameters, sizeof(D3DPRESENT_PARAMETERS)); // clear that shit

	// populate deviceparameters struct since we need it
	DeviceParameters.Windowed = TRUE;
	DeviceParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	DeviceParameters.hDeviceWindow = dGetForegroundWindow();
	DeviceParameters.BackBufferFormat = D3DFMT_UNKNOWN;

	// finally create the device
	if (pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, dGetForegroundWindow(), D3DCREATE_HARDWARE_VERTEXPROCESSING, &DeviceParameters, &DummyDevice) != D3D_OK)
	{
		return false;
	}

	std::cout << "DummyDevice created" << std::endl;

	// go into the dummydevice's vftable and start us off at the first element
	uintptr_t* DummyDeviceVFTable = reinterpret_cast<uintptr_t*>(DummyDevice);
	DummyDeviceVFTable = reinterpret_cast<uintptr_t*>(DummyDeviceVFTable[0]);

	// give the endscene and drawindexedprimitive functions their proper address
	oD3D9EndScene = reinterpret_cast<OriginalD3D9EndScene>(DummyDeviceVFTable[static_cast<int>(D3D9DeviceVFTable::EndScene)]);
	oD3D9DrawPrimitive = reinterpret_cast<OriginalD3D9DrawPrimitive>(DummyDeviceVFTable[static_cast<int>(D3D9DeviceVFTable::DrawPrimitive)]);
	oD3D9DrawIndexedPrimitive = reinterpret_cast<OriginalD3D9DrawIndexedPrimitive>(DummyDeviceVFTable[static_cast<int>(D3D9DeviceVFTable::DrawIndexedPrimitive)]);

	// release that shit
	DummyDevice->Release();
	std::cout << "DummyDevice released" << std::endl;
	pD3D->Release();
	std::cout << "pD3D released" << std::endl;

	return true;
}

void Init()
{
	FILE* cons{};

	dAllocConsole();
	freopen_s(&cons, "CONOUT$", "w", stdout);
	freopen_s(&cons, "CONOUT$", "w", stderr);
	freopen_s(&cons, "CONIN$", "r", stdin);

	dSetConsoleTitleA("D3D9 Hook");
	dSetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0xC);

	std::cout << "Universal D3D9 Hook" << std::endl;
	std::cout << "Press F7 to enable/disable wireframe.\n" << std::endl;

	if (!CreateDummyDevice())
	{
		std::cout << "Failed to create dummy device!" << std::endl;
	}

	oD3D9EndScene = reinterpret_cast<OriginalD3D9EndScene>(Trampoline(reinterpret_cast<uintptr_t>(oD3D9EndScene), reinterpret_cast<uintptr_t>(D3D9EndSceneHook)));
	std::cout << "EndScene hooked: 0x" << std::hex << D3D9EndSceneHook << std::endl;
	oD3D9DrawPrimitive = reinterpret_cast<OriginalD3D9DrawPrimitive>(Trampoline(reinterpret_cast<uintptr_t>(oD3D9DrawPrimitive), reinterpret_cast<uintptr_t>(D3D9DrawPrimitiveHook)));
	std::cout << "DrawPrimitive hooked: 0x" << std::hex << D3D9DrawPrimitiveHook << std::endl;
	oD3D9DrawIndexedPrimitive = reinterpret_cast<OriginalD3D9DrawIndexedPrimitive>(Trampoline(reinterpret_cast<uintptr_t>(oD3D9DrawIndexedPrimitive), reinterpret_cast<uintptr_t>(D3D9DrawIndexedPrimitiveHook)));
	std::cout << "DrawIndexedPrimitive hooked: 0x" << std::hex << oD3D9DrawIndexedPrimitive << " 0x" << D3D9DrawIndexedPrimitiveHook << std::endl;

	// register wireframe hotkey
	dRegisterHotKey(nullptr, 1, MOD_NOREPEAT, VK_F7); // set our hotkey id to 1
	MSG HotkeyMsg{};
	while (GetMessage(&HotkeyMsg, nullptr, 0, 0))
	{
		PeekMessage(&HotkeyMsg, nullptr, 0, 0, 0);
		switch (HotkeyMsg.message)
		{
		case WM_HOTKEY:
		{
			switch (HotkeyMsg.wParam)
			{
			case 1: // if it was our hotkey id
			{
				if (WireFrameEnabled)
					WireFrameEnabled = false;
				else
					WireFrameEnabled = true;
				std::cout << WireFrameEnabled << std::endl;
				break;
			}
			}
			break;
		}
		}
	}
}

int __stdcall DllMain(HMODULE hModule, DWORD ulReason, void*)
{
	dDisableThreadLibraryCalls(hModule);

	switch (ulReason)
	{
	case DLL_PROCESS_ATTACH:
	{
		std::thread InitThread(Init);
		InitThread.detach();
		break;
	}
	}
	return 1;
}