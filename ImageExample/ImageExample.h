#pragma once

#include <wincodec.h>
#include "D2DFramework.h"

class ImageExample : public D2DFramework
{
	Microsoft::WRL::ComPtr<IWICImagingFactory> mspWICFactory;
	Microsoft::WRL::ComPtr<ID2D1Bitmap> mspBitmap;// 그림을 그리는 인터페이스

public:
	virtual void Initialize(HINSTANCE hInstance,
		LPCWSTR title = L"BMP File Example",
		UINT width = 1024,
		UINT height = 768) override;
	void Render() override;
	void Release() override;

public:
	HRESULT LoadBMP(LPCWSTR filename); // 그림을 불러온다.
	HRESULT LoadWICImage(LPCWSTR filename);
};

