#include "ImageExample.h"

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd
)
{
	int ret{};

	try
	{
		ImageExample myFramework;

		myFramework.Initialize(hInstance);
		ret = myFramework.GameLoop();
		myFramework.Release();
		
		return ret;
	}
	catch (const ComException& e)
	{
		OutputDebugStringA(e.what());
	}
}