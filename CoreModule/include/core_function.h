#pragma once

namespace engine
{
	inline _string StripMsvcClassName(const char* msvcName)
	{
		_string result(msvcName);

		static const char* classPrefix = "class ";
		if (result.compare(0, std::strlen(classPrefix), classPrefix) == 0)
		{
			result.erase(0, std::strlen(classPrefix));
		}

		static const char* enginePrefix = "engine::";
		if (result.compare(0, std::strlen(enginePrefix), enginePrefix) == 0)
		{
			result.erase(0, std::strlen(enginePrefix));
		}

		return result;
	}

	template<typename T>
	void SafeDelete(T& pointer)
	{
		if (nullptr != pointer)
		{
			delete pointer;
			pointer = nullptr;
		}
	}

	template<typename T>
	void SafeDeleteArray(T& pointer)
	{
		if (nullptr != pointer)
		{
			delete[] pointer;
			pointer = nullptr;
		}
	}

	template<typename T>
	_uint SafeRelease(T& instance)
	{
		_uint refCnt = 0;

		if (nullptr != instance)
		{
			refCnt = instance->Release();

			if (0 == refCnt)
			{
				instance = nullptr;
			}
		}

		return refCnt;
	}
}
