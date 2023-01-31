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

// POD������ : Plain Old Data : ����ü or �����ڸ� �������� �ʴ� Ŭ����
// ���� : �����͸� �ѹ濡 ���۰����ϴ�.
HRESULT ImageExample::LoadBMP(LPCWSTR filename)
{
	// 1. ���� ����
	std::ifstream file;
	file.open(filename, std::ios::binary);

	// 2. BITMAPFILEHEADER ����ü�б�
	BITMAPFILEHEADER bfh;
	file.read(reinterpret_cast<char*>(&bfh), sizeof(BITMAPFILEHEADER));
	if (bfh.bfType != 0x4D42)
	{
		return E_FAIL;
	}

	// 3. BITMAPINFOHEADER ����ü�б�
	BITMAPINFOHEADER bih;
	file.read(reinterpret_cast<char*>(&bih), sizeof(BITMAPINFOHEADER));
	if (bih.biBitCount != 32)
	{
		return E_FAIL;
	}

	// 4. ���� �ȼ� �迭�� �б�
	file.seekg(bfh.bfOffBits);

	std::vector<char> pixels(bih.biSizeImage); // �̹����� ����Ʈ ũ��

	// �׸� ���� �б�

	// <�� ���� �б�>
	// �Ųٷ� �о���
	//file.read(&pixels[0], bih.biSizeImage); 

	// <�� �پ� �б�>
	// ����(pitch) -> h ��
	// ���� -> h-1 ��
	// ���� -> h-2 ��
	// ...
	//			0��

	int pitch = bih.biWidth * (bih.biBitCount / 8);

	//1. ���̸�ŭ �ݺ��Ѵ�.
	// 1.1 �̹��� ������ ��ġ��ŭ �д´�.
	/*for (int y = bih.biHeight - 1; y >= 0; y--)
	{
		file.read(&pixels[pitch * y], pitch);
	}*/


	// <�� �ȼ��� �б�>
	// ���� - RGB(30, 199, 250)�� �����ϰ� �д´�.

	int index{};
	char r{}, g{}, b{}, a{};

	// 1. ���� ��ŭ �ݺ��Ѵ�.
	for (int y = bih.biHeight - 1; y >= 0; --y)
	{
		// 1.1. �ε����� ���Ѵ�.
		index = y * pitch;
		// 1.2. �ʺ�ŭ �ݺ��Ѵ�.
		for (int x = 0; x < bih.biWidth; ++x)
		{
			// 1.2.1. 1�ȼ��� �д´�.
			file.read(&b, 1);
			file.read(&g, 1);
			file.read(&r, 1);
			file.read(&a, 1);

			// 1.2.2. ����(r == 30, g == 199, b == 250)�� ������ 
			if (r == (char)30 && g == (char)199 && b == (char)250)
			{
				r = g = b = a = 0;	// 1.2.2.1. �����ϰ� ó���Ѵ�.
			}

			// 1.2.3. ������ �����Ѵ�.
			pixels[index++] = b;
			pixels[index++] = g;
			pixels[index++] = r;
			pixels[index++] = a;
		}
	}

	file.close();

	// 5. ��Ʈ�� �������̽� ����� 
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

	// 6. �ȼ� ����
	hr = mspBitmap->CopyFromMemory(nullptr, &pixels[0], pitch);
	ThrowIfFailed(hr);

	return S_OK;
}

HRESULT ImageExample::LoadWICImage(LPCWSTR filename)
{
	HRESULT hr;
	// 1. ���ڴ� ���� (Decoder)
	Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
	hr = mspWICFactory->CreateDecoderFromFilename(
		filename,
		nullptr,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		decoder.GetAddressOf()
	);
	ThrowIfFailed(hr);
	
	// 2. ���ڴ����� ������ ȹ�� (BMP���� - 0�� ������)
	Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
	hr = decoder->GetFrame(0, frame.GetAddressOf());
	ThrowIfFailed(hr);
	
	// 3. �����ͷ� �����ӿ��� �ȼ� �迭�� ���ؿ´�.
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

	// 4. �ȼ��迭�� ��Ʈ���� �����Ѵ�.
	hr = mspRenderTarget->CreateBitmapFromWicBitmap(
		converter.Get(),
		mspBitmap.GetAddressOf()
	);
	ThrowIfFailed(hr);

	return S_OK;
}
