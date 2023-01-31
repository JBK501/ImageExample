#pragma once
#include <Windows.h>
#include <d2d1.h>
#include <wrl/client.h>		//Windows Runtime Library
#include "ComException.h"

class D2DFramework
{
	const LPCWSTR gClassName = { L"MyWindowClass" };

//ID2D1Factory* gpD2DFactory{};
//ID2D1HwndRenderTarget* gpRenderTarget{};
protected:	
	HWND mHwnd;

	// shared_ptr<>
	Microsoft::WRL::ComPtr<ID2D1Factory> mspD2DFactory{};
	Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> mspRenderTarget{};

protected:
	HRESULT InitWindow(HINSTANCE hInstance, LPCWSTR title, UINT width, UINT height);
	virtual HRESULT InitD2D(HWND hwnd);
	virtual HRESULT CreateDeviceResources();

public:
	virtual void Initialize(HINSTANCE hInstance,
		LPCWSTR title = L"Direct2D Example",
		UINT width = 1024, UINT height = 768);
	virtual void Release();
	virtual void Render();
	virtual int GameLoop();

	void ShowErrorMsg(
		LPCWSTR msg, 
		HRESULT error = E_FAIL,
		LPCWSTR title = L"Error"
	);

};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
