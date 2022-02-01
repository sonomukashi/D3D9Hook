#ifndef D3D9HOOKHPP
#define D3D9HOOKHPP
#include <Windows.h>
#include <iostream>
#include <thread>
#include <string>

#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

#define D3D9DEBUG false // enable this if you want to check if functions are being hooked properly

/* our dynamic functions, resolved in our own module so we don't rely on the game's import table to resolve functions for us */

using dynamicDisableThreadLibraryCalls = BOOL(__stdcall*)(HMODULE hLibModule);
extern dynamicDisableThreadLibraryCalls dDisableThreadLibraryCalls = reinterpret_cast<dynamicDisableThreadLibraryCalls>(GetProcAddress(GetModuleHandleA("Kernel32.dll"), "DisableThreadLibraryCalls"));

using dynamicAllocConsole = BOOL(__stdcall*)();
extern dynamicAllocConsole dAllocConsole = reinterpret_cast<dynamicAllocConsole>(GetProcAddress(GetModuleHandleA("Kernel32.dll"), "AllocConsole"));

using dynamicSetConsoleTitleA = BOOL(__stdcall*)(LPCSTR lpConsoleTitle);
extern dynamicSetConsoleTitleA dSetConsoleTitleA = reinterpret_cast<dynamicSetConsoleTitleA>(GetProcAddress(GetModuleHandleA("Kernel32.dll"), "SetConsoleTitleA"));

using dynamicVirtualProtect = BOOL(__stdcall*)(LPVOID lpAddress, 
	SIZE_T dwSize,
	DWORD  flNewProtect,
	PDWORD lpflOldProtect);
extern dynamicVirtualProtect dVirtualProtect = reinterpret_cast<dynamicVirtualProtect>(GetProcAddress(GetModuleHandleA("Kernel32.dll"), "VirtualProtect"));

using dynamicVirtualAlloc = LPVOID(__stdcall*)(LPVOID lpAddress,
	SIZE_T dwSize,
	DWORD  flAllocationType,
	DWORD  flProtect);
extern dynamicVirtualAlloc dVirtualAlloc = reinterpret_cast<dynamicVirtualAlloc>(GetProcAddress(GetModuleHandleA("Kernel32.dll"), "VirtualAlloc"));

using dynamicGetForegroundWindow = HWND(__stdcall*)();
extern dynamicGetForegroundWindow dGetForegroundWindow = reinterpret_cast<dynamicGetForegroundWindow>(GetProcAddress(GetModuleHandleA("User32.dll"), "GetForegroundWindow"));

using dynamicSetConsoleTextAttribute = BOOL(__stdcall*)(
	_In_ HANDLE hConsoleOutput,
	_In_ WORD wAttributes);
extern dynamicSetConsoleTextAttribute dSetConsoleTextAttribute = reinterpret_cast<dynamicSetConsoleTextAttribute>(GetProcAddress(GetModuleHandleA("Kernel32.dll"), "SetConsoleTextAttribute"));

using dynamicMessageBoxA = int(__stdcall*)(HWND hWnd,
	LPCTSTR lpText,
	LPCTSTR lpCaption,
	UINT uType);
extern dynamicMessageBoxA dMessageBoxA = reinterpret_cast<dynamicMessageBoxA>(GetProcAddress(GetModuleHandleA("User32.dll"), "MessageBoxA"));

using dynamicRegisterHotKey = BOOL(__stdcall*)(HWND hWnd,
	int  id,
	UINT fsModifiers,
	UINT vk);
extern dynamicRegisterHotKey dRegisterHotKey = reinterpret_cast<dynamicRegisterHotKey>(GetProcAddress(GetModuleHandleA("User32.dll"), "RegisterHotKey"));

/* our d3d function definitions */

using OriginalD3D9EndScene = HRESULT(__stdcall*)(IDirect3DDevice9* pDevice);
extern OriginalD3D9EndScene oD3D9EndScene = nullptr;

using OriginalD3D9DrawPrimitive = HRESULT(__stdcall*)(IDirect3DDevice9* pDevice,
    D3DPRIMITIVETYPE PrimitiveType,
	UINT StartVertex,
	UINT PrimitiveCount);
extern OriginalD3D9DrawPrimitive oD3D9DrawPrimitive = nullptr;

using OriginalD3D9DrawIndexedPrimitive = HRESULT(__stdcall*)(IDirect3DDevice9* pDevice, 
	D3DPRIMITIVETYPE, 
	INT BaseVertexIndex, 
	UINT MinVertexIndex, 
	UINT NumVertices, 
	UINT startIndex, 
	UINT primCount);
extern OriginalD3D9DrawIndexedPrimitive oD3D9DrawIndexedPrimitive = nullptr;

/* d3d9 device vftable enum */

enum class D3D9DeviceVFTable
{
	QueryInterface,
	AddRef,
	Release,
	TestCooperativeLevel,
	GetAvailableTextureMem,
	EvictManagedResources,
	GetDirect3D,
	GetDeviceCaps,
	GetDisplayMode,
	GetCreationParameters,
	SetCursorProperties,
	SetCursorPosition,
	ShowCursor,
	CreateAdditionalSwapChain,
	GetSwapChain,
	GetNumberOfSwapChains,
	Reset,
	Present,
	GetBackBuffer,
	GetRasterStatus,
	SetDialogBoxMode,
	SetGammaRamp,
	GetGammaRamp,
	CreateTexture,
	CreateVolumeTexture,
	CreateCubeTexture,
	CreateVertexBuffer,
	CreateIndexBuffer,
	CreateRenderTarget,
	CreateDepthStencilSurface,
	UpdateSurface,
	UpdateTexture,
	GetRenderTargetData,
	GetFrontBufferData,
	StretchRect,
	ColorFill,
	CreateOffscreenPlainSurface,
	SetRenderTarget,
	GetRenderTarget,
	SetDepthStencilSurface,
	GetDepthStencilSurface,
	BeginScene,
	EndScene,
	Clear,
	SetTransform,
	GetTransform,
	MultiplyTransform,
	SetViewport,
	GetViewport,
	SetMaterial,
	GetMaterial,
	SetLight,
	GetLight,
	LightEnable,
	GetLightEnabled,
	SetClipPlane,
	GetClipPlane,
	SetRenderState,
	GetRenderState,
	CreateStateBlock,
	BeginStateBlock,
	EndStateBlock,
	SetClipStatus,
	GetClipStatus,
	GetTexture,
	SetTexture,
	GetTextureStageState,
	SetTextureStageState,
	GetSamplerState,
	SetSamplerState,
	ValidateDevice,
	SetPaletteEntries,
	GetPaletteEntries,
	SetCurrentTexturePalette,
	GetCurrentTexturePalette,
	SetScissorRect,
	GetScissorRect,
	SetSoftwareVertexProcessing,
	KernelHandle,
	SetNPatchMode,
	GetNPatchMode,
	DrawPrimitive,
	DrawIndexedPrimitive,
	DrawPrimitiveUP,
	DrawIndexedPrimitiveUP,
	ProcessVertices
};
#endif