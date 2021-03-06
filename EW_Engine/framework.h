﻿// header.h: 표준 시스템 포함 파일
// 또는 프로젝트 특정 포함 파일이 들어 있는 포함 파일입니다.
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
// Windows 헤더 파일
#include <windows.h>
// C 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <DirectXMath.h>
#include <DirectXMathConvert.inl>
#include <DirectXMathMatrix.inl>
#include <DirectXMathMisc.inl>
#include <DirectXMathVector.inl>
#include <DirectXCollision.h>
#include <DirectXCollision.inl>
#include <DirectXColors.h>
#include <d3dcompiler.h>
#include <d3dcompiler.inl>

#include <queue>
#include <unordered_map>
#include <concurrent_queue.h>

#include <agents.h>
#include <string>
#include <iostream>
#include <algorithm>