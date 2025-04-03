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

	inline _string WStringToString(const _wstring& wstring)
	{
		if (wstring.empty())
		{
			return "";
		}

		int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, &wstring[0], static_cast<int>(wstring.size()), nullptr, 0, nullptr, nullptr);
		std::string str(sizeNeeded, 0);
		WideCharToMultiByte(CP_UTF8, 0, &wstring[0], static_cast<int>(wstring.size()), &str[0], sizeNeeded, nullptr, nullptr);
		return str;
	}

	inline _wstring StringToWString(const _string& string)
	{
		if (string.empty())
		{
			return L"";
		}

		int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, &string[0], static_cast<int>(string.size()), nullptr, 0);
		std::wstring wstr(sizeNeeded, 0);
		MultiByteToWideChar(CP_UTF8, 0, &string[0], static_cast<int>(string.size()), &wstr[0], sizeNeeded);
		return wstr;
	}

	inline bool FileExists(const std::wstring& filename)
	{
		std::ifstream File(filename);
		return File.good();
	}

	inline std::wstring GetFileExtensionW(const std::wstring& fullPath)
	{
		size_t pos = fullPath.find_last_of(L"\\/");

		if (pos == std::wstring::npos)
			return fullPath;

		std::wstring fileNameWithExt = fullPath.substr(pos + 1);

		size_t dotPos = fileNameWithExt.find_last_of(L'.');

		std::wstring extension;

		if (dotPos != std::wstring::npos)
		{
			extension = fileNameWithExt.substr(dotPos + 1);
		}

		else
		{
			extension.clear();
		}

		return extension;
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

	inline void to_json(nlohmann::ordered_json& j, const _float2& v)
	{
		j = nlohmann::ordered_json{ {"x", v.x}, {"y", v.y} };
	}

	inline void from_json(const nlohmann::ordered_json& j, _float2& v)
	{
		j.at("x").get_to(v.x);
		j.at("y").get_to(v.y);
	}

	inline void to_json(nlohmann::ordered_json& j, const _float3& v)
	{
		j = nlohmann::ordered_json{ {"x", v.x}, {"y", v.y}, {"z", v.z} };
	}

	inline void from_json(const nlohmann::ordered_json& j, _float3& v)
	{
		j.at("x").get_to(v.x);
		j.at("y").get_to(v.y);
		j.at("z").get_to(v.z);
	}

	inline void to_json(nlohmann::ordered_json& j, const _float4& v)
	{
		j = nlohmann::ordered_json{ {"x", v.x}, {"y", v.y}, {"z", v.z}, {"w", v.w} };
	}

	inline void from_json(const nlohmann::ordered_json& j, _float4& v)
	{
		j.at("x").get_to(v.x);
		j.at("y").get_to(v.y);
		j.at("z").get_to(v.z);
		j.at("z").get_to(v.w);
	}

}
