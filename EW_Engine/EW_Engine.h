#pragma once

#include "resource.h"

#ifndef ENGINEHEADER_H__
#define ENGINEHEADER_H__
inline std::wstring AnsiToWString(const std::string& str)
{
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
	return std::wstring(buffer);
}

class EW_Exception
{
public:
	inline EW_Exception() = default;
	inline EW_Exception(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

	inline std::wstring ToString()const;

	HRESULT ErrorCode = S_OK;
	std::wstring FunctionName;
	std::wstring Filename;
	int LineNumber = -1;
};

EW_Exception::EW_Exception(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber)
{
	ErrorCode = hr;
	FunctionName = functionName;
	Filename = filename;
	LineNumber = lineNumber;
}
std::wstring EW_Exception::ToString() const
{
	return std::wstring();
}

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = AnsiToWString(__FILE__);                       \
    if(FAILED(hr__)) { throw EW_Exception(hr__, L#x, wfn, __LINE__); } \
}
#endif

#ifndef ReleaseCom
#define ReleaseCom(x) { if(x){ x->Release(); x = 0; } }
#endif

#ifndef ReleaseUnique
#define ReleaseUnique(ptr) {if(ptr){ delete ptr.release();}}
#endif

static DirectX::XMFLOAT4X4 Identity4x4()
{
	static DirectX::XMFLOAT4X4 I(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	return I;
}

#endif

#include "EW_Base.h"
#include "EW_Window.h"
#include "EW_Buffer.h"

#include "EW_Obj.h"
#include "EW_Sprite.h"

#include "EW_Device.h"