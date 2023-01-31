#include <sstream>
#include "D2DFramework.h"

#pragma comment (lib,"d2d1.lib")

HRESULT D2DFramework::InitWindow(HINSTANCE hInstance, LPCWSTR title, UINT width, UINT height)
{
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpszClassName = gClassName;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpfnWndProc = WindowProc;
	wc.cbSize = sizeof(WNDCLASSEX);
	if (!RegisterClassEx(&wc))
	{
		MessageBox(nullptr, L"Failed to register window class!", L"Error", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	RECT wr = { 0,0,
		static_cast<LONG>(width),
		static_cast<LONG>(height) };

	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
	mHwnd = CreateWindowEx(
		NULL,
		gClassName,
		title,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wr.right - wr.left,
		wr.bottom - wr.top,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (mHwnd == nullptr)
	{
		MessageBox(nullptr, L"Failed to create window class!", L"Error", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	SetWindowLongPtr(mHwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	return S_OK;
}

HRESULT D2DFramework::InitD2D(HWND hwnd)
{
	// 1. Direct2D Factory 생성
	HRESULT hr = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED, mspD2DFactory.GetAddressOf()
	);

	ThrowIfFailed(hr);

	CreateDeviceResources();

	return S_OK;
}

HRESULT D2DFramework::CreateDeviceResources()
{
	// 2. 렌더타겟(그릴 곳) 생성
	RECT wr{};
	GetClientRect(mHwnd, &wr); // 윈도우 핸들로 부터 윈도우 클라이언트 영역을 구해 온다.
	HRESULT hr = mspD2DFactory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(
			mHwnd,
			D2D1::SizeU(wr.right - wr.left, wr.bottom - wr.top)
		),
		mspRenderTarget.GetAddressOf()
	);

	ThrowIfFailed(hr);

	return hr;
}

void D2DFramework::Initialize(HINSTANCE hInstance, 
	LPCWSTR title, 
	UINT width, UINT height)
{
	InitWindow(hInstance, title, width, height);
	InitD2D(mHwnd);

	ShowWindow(mHwnd, SW_SHOW);
	UpdateWindow(mHwnd);
}

void D2DFramework::Release()
{
	// 4. 해제
	/*if (gpRenderTarget != nullptr)
	{
		gpRenderTarget->Release();
		gpRenderTarget = nullptr;
	}

	if (gpD2DFactory != nullptr)
	{
		gpD2DFactory->Release();
		gpD2DFactory = nullptr;
	}*/
	mspRenderTarget.Reset();
	mspD2DFactory.Reset();
}

void D2DFramework::Render()
{
	// 3. 그리기
	mspRenderTarget->BeginDraw();
	mspRenderTarget->Clear(D2D1::ColorF(0.0f, 0.2f, 0.4f, 1.0f));	// 렌더타켓(여기선 윈도우)화면을 특정색상으로 설정한다.

	HRESULT hr = mspRenderTarget->EndDraw();

	// 디바이스 로스트 발생 시 해야하는 후속처리
	if (hr == D2DERR_RECREATE_TARGET)
	{
		CreateDeviceResources();
	}
}

int D2DFramework::GameLoop()
{
	MSG msg;

	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				break;
		}
		else
		{
			Render();
		}
	}
	return static_cast<int>(msg.wParam);
}

void D2DFramework::ShowErrorMsg(LPCWSTR msg, HRESULT error, LPCWSTR title)
{
	std::wostringstream oss;
	oss << "Error Code : " << error << std::endl << msg;

	MessageBox(NULL, oss.str().c_str(), title, MB_OK);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	D2DFramework* pFramework = reinterpret_cast<D2DFramework*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

	switch (message)
	{
		case WM_CLOSE:
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}