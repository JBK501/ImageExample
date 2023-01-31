#include <fstream>
#include <vector>
#include "ImageExample.h"

#pragma comment(lib, "WindowsCodecs.lib")

void ImageExample::Initialize(HINSTANCE hInstance, LPCWSTR title, 
	UINT width, UINT height)
{
	HRESULT hr = CoInitialize(nullptr);
	ThrowIfFailed(hr);

	hr = CoCreateInstance(
		CLSID_WICImagingFactory, 
		nullptr, 
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(mspWICFactory.GetAddressOf())
	);
	ThrowIfFailed(hr);

	D2DFramework::Initialize(hInstance, title, width, height);

	//LoadBMP(L"Data/32.bmp");
	LoadWICImage(L"Data/bug.png");
}

void ImageExample::Render()
{
	mspRenderTarget->BeginDraw();

	mspRenderTarget->Clear(D2D1::ColorF(0.0f,0.2f,0.4f,1.0f));
	mspRenderTarget->DrawBitmap(mspBitmap.Get());


	mspRenderTarget->EndDraw();
}

void ImageExample::Release()
{
	D2DFramework::Release();
	D2DFramework::Release();

	mspWICFactory.Reset();
	CoUninitialize();
}

// POD데이터 : Plain Old Data : 구조체 or 생성자를 제공하지 않는 클래스
// 장정 : 데이터를 한방에 전송가능하다.
HRESULT ImageExample::LoadBMP(LPCWSTR filename)
{
	// 1. 파일 열기
	std::ifstream file;
	file.open(filename, std::ios::binary);

	// 2. BITMAPFILEHEADER 구조체읽기
	BITMAPFILEHEADER bfh;
	file.read(reinterpret_cast<char*>(&bfh), sizeof(BITMAPFILEHEADER));
	if (bfh.bfType != 0x4D42)
	{
		return E_FAIL;
	}

	// 3. BITMAPINFOHEADER 구조체읽기
	BITMAPINFOHEADER bih;
	file.read(reinterpret_cast<char*>(&bih), sizeof(BITMAPINFOHEADER));
	if (bih.biBitCount != 32)
	{
		return E_FAIL;
	}

	// 4. 실제 픽셀 배열을 읽기
	file.seekg(bfh.bfOffBits);

	std::vector<char> pixels(bih.biSizeImage); // 이미지의 바이트 크기

	// 그림 파일 읽기

	// <한 번에 읽기>
	// 거꾸로 읽어짐
	//file.read(&pixels[0], bih.biSizeImage); 

	// <한 줄씩 읽기>
	// 한줄(pitch) -> h 줄
	// 한줄 -> h-1 줄
	// 한줄 -> h-2 줄
	// ...
	//			0줄

	int pitch = bih.biWidth * (bih.biBitCount / 8);

	//1. 높이만큼 반복한다.
	// 1.1 이미지 높이의 피치만큼 읽는다.
	/*for (int y = bih.biHeight - 1; y >= 0; y--)
	{
		file.read(&pixels[pitch * y], pitch);
	}*/


	// <한 픽셀씩 읽기>
	// 배경색 - RGB(30, 199, 250)은 투명하게 읽는다.

	int index{};
	char r{}, g{}, b{}, a{};

	// 1. 높이 만큼 반복한다.
	for (int y = bih.biHeight - 1; y >= 0; --y)
	{
		// 1.1. 인덱스를 구한다.
		index = y * pitch;
		// 1.2. 너비만큼 반복한다.
		for (int x = 0; x < bih.biWidth; ++x)
		{
			// 1.2.1. 1픽셀을 읽는다.
			file.read(&b, 1);
			file.read(&g, 1);
			file.read(&r, 1);
			file.read(&a, 1);

			// 1.2.2. 배경색(r == 30, g == 199, b == 250)과 같으면 
			if (r == (char)30 && g == (char)199 && b == (char)250)
			{
				r = g = b = a = 0;	// 1.2.2.1. 투명하게 처리한다.
			}

			// 1.2.3. 색상을 저장한다.
			pixels[index++] = b;
			pixels[index++] = g;
			pixels[index++] = r;
			pixels[index++] = a;
		}
	}

	file.close();

	// 5. 비트맵 인터페이스 만들기 
	HRESULT hr;

	hr = mspRenderTarget->CreateBitmap(
		D2D1::SizeU(bih.biWidth, bih.biHeight),
		D2D1::BitmapProperties(
			D2D1::PixelFormat(
				DXGI_FORMAT_B8G8R8A8_UNORM,
				D2D1_ALPHA_MODE_PREMULTIPLIED)
		),
		mspBitmap.GetAddressOf()
	);
	ThrowIfFailed(hr);

	// 6. 픽셀 복사
	hr = mspBitmap->CopyFromMemory(nullptr, &pixels[0], pitch);
	ThrowIfFailed(hr);

	return S_OK;
}

HRESULT ImageExample::LoadWICImage(LPCWSTR filename)
{
	HRESULT hr;
	// 1. 디코더 생성 (Decoder)
	Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
	hr = mspWICFactory->CreateDecoderFromFilename(
		filename,
		nullptr,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		decoder.GetAddressOf()
	);
	ThrowIfFailed(hr);
	
	// 2. 디코더에서 프레임 획득 (BMP파일 - 0번 프레임)
	Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
	hr = decoder->GetFrame(0, frame.GetAddressOf());
	ThrowIfFailed(hr);
	
	// 3. 컨버터로 프레임에서 픽셀 배열을 구해온다.
	Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
	hr = mspWICFactory->CreateFormatConverter(converter.GetAddressOf());
	ThrowIfFailed(hr);

	hr = converter->Initialize(
		frame.Get(),
		GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone,
		nullptr,
		0,
		WICBitmapPaletteTypeCustom
	);
	ThrowIfFailed(hr);

	// 4. 픽셀배열로 비트맵을 생성한다.
	hr = mspRenderTarget->CreateBitmapFromWicBitmap(
		converter.Get(),
		mspBitmap.GetAddressOf()
	);
	ThrowIfFailed(hr);

	return S_OK;
}
