// https://github.com/glfw/glfw/blob/master/deps/mingw/dinput.h
// https://github.com/8BitPimp/fo1_rev/blob/master/dinput.cpp
// These are __thiscalls and I'm too lazy to implement that, just showing them on stack is easier.
/*
DECLARE_INTERFACE_(IDirectInputDeviceA, IUnknown)
{
   //IUnknown methods, index 0
   STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
   STDMETHOD_(ULONG, AddRef)(THIS) PURE;
   STDMETHOD_(ULONG, Release)(THIS) PURE;

   //IDirectInputDeviceA methods, index 3
   STDMETHOD(GetCapabilities)(THIS_ LPDIDEVCAPS) PURE;
   STDMETHOD(EnumObjects)(THIS_ LPDIENUMDEVICEOBJECTSCALLBACKA, LPVOID, DWORD) PURE;
   STDMETHOD(GetProperty)(THIS_ REFGUID, LPDIPROPHEADER) PURE;
   STDMETHOD(SetProperty)(THIS_ REFGUID, LPCDIPROPHEADER) PURE;
   STDMETHOD(Acquire)(THIS) PURE;
   STDMETHOD(Unacquire)(THIS) PURE;
   STDMETHOD(GetDeviceState)(THIS_ DWORD, LPVOID) PURE;
   STDMETHOD(GetDeviceData)(THIS_ DWORD, LPDIDEVICEOBJECTDATA, LPDWORD, DWORD) PURE;
   STDMETHOD(SetDataFormat)(THIS_ LPCDIDATAFORMAT) PURE;
   STDMETHOD(SetEventNotification)(THIS_ HANDLE) PURE;
   STDMETHOD(SetCooperativeLevel)(THIS_ HWND, DWORD) PURE;
   STDMETHOD(GetObjectInfo)(THIS_ LPDIDEVICEOBJECTINSTANCEA, DWORD, DWORD) PURE;
   STDMETHOD(GetDeviceInfo)(THIS_ LPDIDEVICEINSTANCEA) PURE;
   STDMETHOD(RunControlPanel)(THIS_ HWND, DWORD) PURE;
   STDMETHOD(Initialize)(THIS_ HINSTANCE, DWORD, REFGUID) PURE;
};
*/

#include "stdafx.h"
#include "DInput8Hook.h"
#include "Memory.h"
#include "D3D9Hook_Settings.h"
#include "Mods.h"
#include <dinput.h>

#pragma comment (lib, "dxguid.lib")

typedef LPDIRECTINPUTDEVICEA* LPPDIRECTINPUTDEVICEA;
typedef HRESULT(WINAPI* GetDeviceState_t)(HINSTANCE, DWORD, LPVOID);
typedef HRESULT(WINAPI* CreateDevice_t)(HINSTANCE, REFGUID, LPPDIRECTINPUTDEVICEA, LPUNKNOWN);
typedef HRESULT(WINAPI* DirectInput8Create_t)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);

namespace DInput8Hook {
   GetDeviceState_t     origGetDeviceState_Keyboard = nullptr;
   GetDeviceState_t     origGetDeviceState_Mouse    = nullptr;
   CreateDevice_t       origCreateDevice            = nullptr;
   DirectInput8Create_t origDirectInput8Create      = nullptr;

   HRESULT WINAPI hkGetDeviceState_Keyboard(HINSTANCE hInstance, DWORD cbData, LPVOID lpvData) {
      HRESULT retOrigGetDeviceState = origGetDeviceState_Keyboard(hInstance, cbData, lpvData);

      if (cbData == 256) {
         if (D3D9HookSettings::blockKeyboard || *Mods::GameInfo::isGameWindowInactive) {
            ZeroMemory(lpvData, 256);
         }
         else {
            BYTE* keys = (BYTE*)lpvData;
            if (D3D9HookSettings::putIntoReverse) {
               keys[*Mods::GameInfo::key_Brake] = TRUE;
               D3D9HookSettings::putIntoReverse = false;
            }
            else if (D3D9HookSettings::reversePedals) {
               BYTE brake                            = keys[*Mods::GameInfo::key_Brake];
               BYTE accel                            = keys[*Mods::GameInfo::key_Accelerate];
               keys[*Mods::GameInfo::key_Brake]      = accel;
               keys[*Mods::GameInfo::key_Accelerate] = brake;
            }
         }
      }
      return retOrigGetDeviceState;
   }
   HRESULT WINAPI hkGetDeviceState_Mouse(HINSTANCE hInstance, DWORD cbData, LPVOID lpvData) {
      HRESULT retOrigGetDeviceState = origGetDeviceState_Mouse(hInstance, cbData, lpvData);

      DIMOUSESTATE* mouseState = (DIMOUSESTATE*)lpvData;
      if (D3D9HookSettings::blockMouse) {
         ZeroMemory(mouseState->rgbButtons, 4);
      }
      else if (*Mods::GameInfo::isGameWindowInactive) {
         ZeroMemory(lpvData, sizeof(DIMOUSESTATE));
      }
      return retOrigGetDeviceState;
   }

   HRESULT WINAPI hkCreateDevice(HINSTANCE hInstance, REFGUID refGUID, LPPDIRECTINPUTDEVICEA lppDirectInputDevice, LPUNKNOWN lpUnkOuter) {
      HRESULT retOrigCreateDevice = origCreateDevice(hInstance, refGUID, lppDirectInputDevice, lpUnkOuter);
      if (refGUID == GUID_SysKeyboard) {
         DWORD* inputTable = *(PDWORD*)(*lppDirectInputDevice);
         Memory::openMemoryAccess(inputTable[9], 4);

         origGetDeviceState_Keyboard = (GetDeviceState_t)(DWORD)inputTable[9];
         inputTable[9]               = (DWORD)hkGetDeviceState_Keyboard;

         Memory::restoreMemoryAccess();
      }
      else if (refGUID == GUID_SysMouse) {
         DWORD* inputTable = *(PDWORD*)(*lppDirectInputDevice);
         Memory::openMemoryAccess(inputTable[9], 4);

         origGetDeviceState_Mouse = (GetDeviceState_t)(DWORD)inputTable[9];
         inputTable[9]            = (DWORD)hkGetDeviceState_Mouse;

         Memory::restoreMemoryAccess();
      }
      return retOrigCreateDevice;
   }

   DWORD origDirectInput8CreateAddr;
   BYTE origCreateDeviceBeginBytes[6];
   BYTE detourCreateDeviceBeginBytes[6];

   HRESULT WINAPI hkDirectInput8Create(HINSTANCE hInstance, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN lpUnkOuter) {
      // Call original DirectInput8Create
      Memory::openMemoryAccess(origDirectInput8CreateAddr, 6);
      memcpy_s((LPVOID)origDirectInput8CreateAddr, 6, (LPVOID)origCreateDeviceBeginBytes, 6);

      HRESULT retOrigDirectInput8Create = origDirectInput8Create(hInstance, dwVersion, riidltf, ppvOut, lpUnkOuter);

      memcpy_s((LPVOID)origDirectInput8CreateAddr, 6, (LPVOID)detourCreateDeviceBeginBytes, 6);
      Memory::restoreMemoryAccess();


      DWORD* createDeviceAddr = (DWORD*)(
         (*(DWORD*)(*ppvOut) /* function table */)
         + 0x0C /* create device address offset */);

      Memory::openMemoryAccess(*createDeviceAddr, 4);
      origCreateDevice = (CreateDevice_t)*createDeviceAddr;

      *createDeviceAddr = (DWORD)hkCreateDevice;
      Memory::restoreMemoryAccess();
      return retOrigDirectInput8Create;
   }

   void Init() {
      origDirectInput8CreateAddr = (DWORD)GetProcAddress(LoadLibrary(L"dinput8.dll"), "DirectInput8Create");
      origDirectInput8Create     = (DirectInput8Create_t)origDirectInput8CreateAddr;

      memcpy_s((LPVOID)origCreateDeviceBeginBytes, 6, (LPVOID)origDirectInput8CreateAddr, 6);

      Memory::writeJMP(origDirectInput8CreateAddr, (DWORD)hkDirectInput8Create);
      Memory::writeRet(origDirectInput8CreateAddr + 5);

      memcpy_s((LPVOID)detourCreateDeviceBeginBytes, 6, (LPVOID)origDirectInput8CreateAddr, 6);
   }
}