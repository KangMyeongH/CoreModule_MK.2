#pragma once

namespace engine
{
	typedef 	bool						_bool;
	typedef 	signed char 				_byte;
	typedef		unsigned char				_ubyte;
	typedef		char						_char;

	typedef		wchar_t						_wchar;

	typedef		std::string					_string;
	typedef 	std::wstring				_wstring;

	typedef		signed short				_short;
	typedef		unsigned short				_ushort;

	typedef		signed int					_int;
	typedef		unsigned int				_uint;

	typedef		signed long					_long;
	typedef		unsigned long				_ulong;

	typedef		float						_float;
	typedef		double						_double;

	typedef		DirectX::XMFLOAT2			_float2;
	typedef		DirectX::XMFLOAT3			_float3;
	typedef		DirectX::XMFLOAT4			_float4;

	typedef		DirectX::XMVECTOR			_vector;
	typedef		DirectX::FXMVECTOR			_fvector;
	typedef		DirectX::GXMVECTOR			_gvector;
	typedef		DirectX::HXMVECTOR			_hvector;
	typedef		DirectX::CXMVECTOR			_cvector;

	typedef		DirectX::XMFLOAT4X4			_float4X4;

	typedef		DirectX::XMMATRIX			_matrix;
	typedef		DirectX::FXMMATRIX			_fmatrix;
	typedef		DirectX::CXMMATRIX			_cmatrix;

	template <typename T>
	using 		SharedPtr = std::shared_ptr<T>;

	template <typename T>
	using		WeakPtr = std::weak_ptr<T>;
}
