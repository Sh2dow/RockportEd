#pragma once
// Win32 targeting
#define NTDDI_VERSION 0x06010000 // NTDDI_WIN7
#define _WIN32_WINNT  0x0601     // _WIN32_WINNT_WIN7
#define WINVER        0x0601     // _WIN32_WINNT_WIN7
#include <SDKDDKVer.h>
// Win32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
// Things?
#define _CRT_NO_SECURE_WARNINGS

// Commonly used headers
#include <map>
#include <string>
#include <vector>
#include "Helpers\Memory\Memory.h"
// MirrorHook Definitions
#define MIRRORHOOK_DEFINITIONS_PATH "../Modules/MirrorHook/MirrorHook/inc/Definitions.hpp"