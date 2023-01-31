#pragma once
#include <exception>
#include <sstream>

class ComException : public std::exception
{
	HRESULT mResult;

public:
	ComException(HRESULT hr) : mResult(hr) {}

	virtual const char* what() const override
	{
		/*std::ostringstream oss;

		oss << "Failure with HRESULt : " << mResult << std::endl;

		return oss.str().c_str();*/

		static char buf[512]{};

		sprintf_s(buf, 512, "Failure with HRESULT : %X", mResult);
		return buf;

	
	}
};

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw ComException(hr);
	}
}

