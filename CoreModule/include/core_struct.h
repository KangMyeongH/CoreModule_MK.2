#pragma once

namespace engine
{
	struct Ray
	{
		Vector3 Origin;			// Ray의 시작점 		(월드 좌표)
		Vector3 Direction;		// Ray의 방향 벡터	(정규화된 월드 벡터)

		Ray() : Origin(0, 0, 0), Direction(0, 0, 1) {}
		Ray(const Vector3& origin, const Vector3& direction) : Origin(origin), Direction(direction) {}

		Vector3 GetPoint(const _float distance) const
		{
			return Origin + Direction * distance;
		}
	};

	struct RayHit
	{
		Vector3 Point;			// 충돌 지점
		Vector3 Normal;		// 충돌 법선 ( 지금은 미 구현 )
		float Distance;		// ray origin으로부터 거리. 즉, ray 시작점으로 부터 거리
		//Collider* Collider; // 충돌된 collider

		RayHit() : Point(0,0,0), Normal(0,0,0), Distance(0.f) {}
	};

	struct VS_ConstantBuffer
	{
		_float4X4 WorldMat;
		_float4X4 ViewMat;
		_float4X4 ProjMat;
	};

	// 기본적으로 Vertex랑 Pixel Shader만을 들고 있는 Shader
	struct Shader
	{
		ComPtr<ID3D11VertexShader>		VertexShader;
		ComPtr<ID3D11PixelShader>		PixelShader;
		ComPtr<ID3D11DomainShader>		DomainShader;
		ComPtr<ID3D11ComputeShader>		ComputeShader;
		ComPtr<ID3D11GeometryShader>	GeometryShader;
		ComPtr<ID3D11HullShader>		HullShader;
		ComPtr<ID3D11InputLayout>		InputLayout;

		Shader() = default;
		~Shader() = default;

		void Bind(ID3D11DeviceContext* context)
		{
			context->IASetInputLayout(InputLayout);
			context->VSSetShader(VertexShader, nullptr, 0);
			context->HSSetShader(HullShader, nullptr, 0);
			context->DSSetShader(DomainShader, nullptr, 0);
			context->GSSetShader(GeometryShader, nullptr,0);
			context->PSSetShader(PixelShader, nullptr, 0);
			context->CSSetShader(ComputeShader, nullptr, 0);
		}
	};

}
