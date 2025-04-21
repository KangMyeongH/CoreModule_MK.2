#pragma once

#define PI 3.14159265359f

namespace engine
{
	template <typename T>
	T Clamp(const T& value, const T& min, const T& max)
	{
		if (value < min)
		{
			return min;
		}

		if (value > max)
		{
			return max;
		}

		return value;
	}

	inline _float RadianToDegree(const _float radian)
	{
		return radian * (180.f / PI);
	}

	inline _float DegreeToRadian(const _float degree)
	{
		return degree * (PI / 180.f);
	}

	struct Vector3
	{
	public:
		//======================================//
		//				constructor				//
		//======================================//

		Vector3() : Value(0, 0, 0) {}
		Vector3(const _float x, const _float y, const _float z) : Value(x, y, z) {}
		explicit Vector3(const _float3& v) : Value(v) {}
		
		Vector3& operator=(const Vector3& rhs) = default;
		Vector3& operator=(const _vector& rhs)
		{
			XMStoreFloat3(&Value, rhs);
			return *this;
		}

		//======================================//
		//				operators				//
		//======================================//

		Vector3 operator+(const Vector3& rhs) const
		{
			return FromVector(DirectX::XMVectorAdd(ToVector(), rhs.ToVector()));
		}

		Vector3& operator+=(const Vector3& rhs)
		{
			const _vector v = DirectX::XMVectorAdd(ToVector(), rhs.ToVector());
			XMStoreFloat3(&Value, v);

			return *this;
		}

		Vector3 operator-(const Vector3& rhs) const
		{
			return FromVector(DirectX::XMVectorSubtract(ToVector(), rhs.ToVector()));
		}

		Vector3& operator-=(const Vector3& rhs)
		{
			const _vector v = DirectX::XMVectorSubtract(ToVector(), rhs.ToVector());
			XMStoreFloat3(&Value, v);

			return *this;
		}

		Vector3 operator*(const _float scalar) const
		{
			return FromVector(DirectX::XMVectorScale(ToVector(), scalar));
		}

		Vector3& operator*=(const _float scalar)
		{
			const _vector v = DirectX::XMVectorScale(ToVector(), scalar);
			XMStoreFloat3(&Value, v);

			return *this;
		}

		Vector3 operator*(const Vector3& rhs) const
		{
			return FromVector(DirectX::XMVectorMultiply(ToVector(), rhs.ToVector()));
		}

		Vector3& operator*=(const Vector3& rhs)
		{
			const _vector v = DirectX::XMVectorMultiply(ToVector(), rhs.ToVector());
			XMStoreFloat3(&Value, v);

			return *this;
		}

		Vector3 operator/(const _float scalar) const
		{
			if (scalar == 0.0f)
			{
				return Vector3{ 0.0f, 0.0f, 0.0f };
			}

			const _vector v = DirectX::XMVectorScale(ToVector(), 1.0f / scalar);
			return FromVector(v);
		}

		Vector3& operator/=(const _float scalar)
		{
			if (scalar == 0.0f)
			{
				Value = _float3(0.0f, 0.0f, 0.0f);
				return *this;
			}

			const _vector v = DirectX::XMVectorScale(ToVector(), 1.0f / scalar);
			XMStoreFloat3(&Value, v);
			return *this;
		}

		Vector3 operator/(const Vector3& rhs) const
		{
			const _vector v1 = ToVector();
			const _vector v2 = rhs.ToVector();
			const _vector zero = DirectX::XMVectorZero();

			const _vector mask = DirectX::XMVectorNotEqual(v2, zero);
			const _vector divResult = DirectX::XMVectorDivide(v1, v2);
			const _vector finalResult = DirectX::XMVectorSelect(zero, divResult, mask);

			return FromVector(finalResult);
		}

		_bool operator==(const Vector3& rhs) const
		{
			const _vector v1 = ToVector();
			const _vector v2 = rhs.ToVector();
			const _vector epsilon = DirectX::XMVectorReplicate(1e-6f);

			return DirectX::XMVector3NearEqual(v1, v2, epsilon);
		}

		_bool operator!=(const Vector3& rhs) const
		{
			return !(*this == rhs);
		}

		Vector3 operator-() const
		{
			return FromVector(DirectX::XMVectorNegate(ToVector()));
		}

		_float& operator[](const int index)
		{
			switch (index)
			{
			case 0:
				return Value.x;
			case 1:
				return Value.y;
			case 2:
				return Value.z;
			default:
				throw std::out_of_range("Index out of range");
			}
		}

		//======================================//
		//				  method				//
		//======================================//

		_vector	ToVector() const
		{
			return XMLoadFloat3(&Value);
		}

		static Vector3 FromVector(_fvector v)
		{
			Vector3 result;
			XMStoreFloat3(&result.Value, v);
			return result;
		}

		// 정규화 시킨 벡터를 반환 (this를 정규화 하지않음)
		Vector3 Normalized() const
		{
			return FromVector(DirectX::XMVector3Normalize(ToVector()));
		}

		// 벡터를 정규화 시킴 (this를 정규화 함)
		void Normalize()
		{
			const _vector v = DirectX::XMVector3Normalize(ToVector());
			XMStoreFloat3(&Value, v);
		}

		// 벡터의 크기를 반환 (Length)
		_float Magnitude() const
		{
			const _vector v = ToVector();
			return DirectX::XMVectorGetX(DirectX::XMVector3Length(v));
		}

		// 제곱근을 하지않은 벡터의 크기를 반환 (LengthSq)
		_float SqrMagnitude() const
		{
			const _vector v = ToVector();
			return DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(v));
		}

		//======================================//
		//			   static method			//
		//======================================//

		static Vector3 Forward()
		{
			return Vector3{ 0.f, 0.f, 1.f };
		}

		static Vector3 Back()
		{
			return Vector3{ 0.f, 0.f, -1.f };
		}

		static Vector3 Up()
		{
			return Vector3{ 0.f, 1.f, 0.f };
		}

		static Vector3 Down()
		{
			return Vector3{ 0.f, -1.f, 0.f };
		}

		static Vector3 Right()
		{
			return Vector3{ 1.f, 0.f, 0.f };
		}

		static Vector3 Left()
		{
			return Vector3{ -1.f, 0.f, 0.f };
		}

		static Vector3 Zero()
		{
			return Vector3{ 0.f, 0.f, 0.f };
		}

		static Vector3 One()
		{
			return Vector3{ 1.f, 1.f, 1.f };
		}

		// 정규화 된 벡터를 반환
		static Vector3 Normalize(const Vector3& value)
		{
			return FromVector(DirectX::XMVector3Normalize(value.ToVector()));
		}

		// 두 벡터의 내적을 반환
		static _float Dot(const Vector3& lhs, const Vector3& rhs)
		{
			return DirectX::XMVectorGetX(DirectX::XMVector3Dot(lhs.ToVector(), rhs.ToVector()));
		}

		// 두 벡터의 외적을 반환
		static Vector3 Cross(const Vector3& lhs, const Vector3 rhs)
		{
			return FromVector(DirectX::XMVector3Cross(lhs.ToVector(), rhs.ToVector()));
		}

		// 두 벡터의 거리를 구함
		static _float Distance(const Vector3& startVec, const Vector3& endVec)
		{
			const Vector3 diff = endVec - startVec;

			return diff.Magnitude();
		}

		// 두 벡터의 박향 벡터를 반환
		static Vector3 Direction(const Vector3& startVec, const Vector3& endVec)
		{
			const Vector3 diff = endVec - startVec;

			return diff.Normalized();
		}

		// 선형 보간
		static Vector3 Lerp(const Vector3& start, const Vector3& end, float t)
		{
			t = Clamp(t, 0.0f, 1.0f);
			return start + (end - start) * t;
		}

		// 두 벡터의 가장 큰 요소로만 이루어진 벡터를 반환
		static Vector3 Max(const Vector3& lhs, const Vector3& rhs)
		{
			const _vector vLhs = lhs.ToVector();
			const _vector vRhs = rhs.ToVector();
			const _vector max = DirectX::XMVectorMax(vLhs , vRhs);

			return FromVector(max);
		}

		// 두 벡터의 가장 작은 요소로만 이루어진 벡터를 반환
		static Vector3 Min(const Vector3& lhs, const Vector3& rhs)
		{
			const _vector vLhs = lhs.ToVector();
			const _vector vRhs = rhs.ToVector();
			const _vector min = DirectX::XMVectorMin(vLhs, vRhs);

			return FromVector(min);
		}

		//======================================//
		//		   serialization field			//
		//======================================//

		friend void to_json(nlohmann::ordered_json& j, const Vector3& v)
		{
			j = nlohmann::ordered_json{
				{ "x", v.Value.x },
				{ "y", v.Value.y },
				{ "z", v.Value.z }
			};
		}

		friend void from_json(const nlohmann::ordered_json& j, Vector3& v)
		{
			j.at("x").get_to(v.Value.x);
			j.at("y").get_to(v.Value.y);
			j.at("z").get_to(v.Value.z);
		}

	public:
		_float3 Value;
	};
	
	struct Quaternion
	{
	public:
		//======================================//
		//				constructor				//
		//======================================//

		Quaternion() : Value(0.0f, 0.0f, 0.0f, 1.0f) {}
		Quaternion(const _float x, const _float y, const _float z, const _float w) : Value(x, y, z ,w) {}
		explicit Quaternion(const _float4& q) : Value(q) {}
		Quaternion& operator=(const Quaternion& rhs) = default;
		Quaternion& operator=(const _float4& rhs)
		{
			Value = rhs;
			return *this;
		}
		Quaternion& operator=(const _vector& rhs)
		{
			XMStoreFloat4(&Value, rhs);
			return *this;
		}

		//======================================//
		//				operators				//
		//======================================//

		Quaternion operator+(const Quaternion& rhs) const
		{
			const _vector q1 = ToVector();
			const _vector q2 = rhs.ToVector();
			const _vector qSum = DirectX::XMVectorAdd(q1, q2);

			return FromVector(qSum);
		}

		Quaternion& operator+=(const Quaternion& rhs)
		{
			*this = *this + rhs;

			return *this;
		}

		Quaternion operator-(const Quaternion& rhs) const
		{
			const _vector q1 = ToVector();
			const _vector q2 = rhs.ToVector();
			const _vector qSub = DirectX::XMVectorSubtract(q1, q2);

			return FromVector(qSub);
		}

		Quaternion& operator-=(const Quaternion& rhs)
		{
			*this = *this - rhs;

			return *this;
		}

		Quaternion operator*(const _float scalar) const
		{
			const _vector q = ToVector();
			const _vector scaled = DirectX::XMVectorScale(q, scalar);

			return FromVector(scaled);
		}

		Quaternion operator*=(const _float scalar)
		{
			*this = *this * scalar;

			return *this;
		}

		Quaternion operator*(const Quaternion& rhs) const
		{
			return FromVector(DirectX::XMQuaternionMultiply(ToVector(), rhs.ToVector()));
		}

		Quaternion operator*=(const Quaternion& rhs)
		{
			*this = *this * rhs;

			return *this;
		}

		Vector3 operator*(const Vector3& point) const
		{
			const _vector q = ToVector();
			const _vector v = point.ToVector();
			const _vector rotated = DirectX::XMVector3Rotate(v, q);

			return Vector3::FromVector(rotated);
		}

		Quaternion operator-() const
		{
			const _vector neg = DirectX::XMVectorNegate(ToVector());

			return FromVector(neg);
		}

		_bool operator==(const Quaternion& rhs) const
		{
			const _vector q1 = ToVector();
			const _vector q2 = rhs.ToVector();
			const _vector epsilon = DirectX::XMVectorReplicate(1e-6f);

			return DirectX::XMVector4NearEqual(q1, q2, epsilon);
		}

		_bool operator!=(const Quaternion& rhs) const
		{
			return !(*this == rhs);
		}

		_float& operator[](const int index)
		{
			switch (index)
			{
			case 0:
				return Value.x;
			case 1:
				return Value.y;
			case 2:
				return Value.z;
			case 3:
				return Value.w;
			default: 
				throw std::out_of_range("Index out of range");
			}
		}

		//======================================//
		//				  method				//
		//======================================//

		_vector ToVector() const
		{
			return XMLoadFloat4(&Value);
		}

		Vector3 Rotate(const Vector3& point) const
		{
			const _vector p = point.ToVector();
			const _vector r = DirectX::XMVector3Rotate(p, ToVector());

			return Vector3::FromVector(r);
		}

		Vector3 EulerAngles() const
		{
			_matrix rot = DirectX::XMMatrixRotationQuaternion(ToVector());

			_float yaw, pitch, roll;

			Vector3 angles;

			pitch = std::asin(-rot.r[2].m128_f32[1]);

			if (std::cos(pitch) > 0.0001f)
			{
				yaw = std::atan2(rot.r[2].m128_f32[0], rot.r[2].m128_f32[2]); // m31, m33
				roll = std::atan2(rot.r[0].m128_f32[1], rot.r[1].m128_f32[1]); // m12, m22
			}

			else
			{
				// 짐벌락 발생 시 yaw와 roll은 서로 얽힘
				yaw = std::atan2(-rot.r[1].m128_f32[0], rot.r[0].m128_f32[0]); // -m21, m11
				roll = 0;
			}


			_float sinP = 2.0f * (Value.w * Value.x - Value.y * Value.z);

			if (sinP > 1.0f)
			{
				sinP = 1.0f;
			}

			if (sinP < -1.f)
			{
				sinP = -1.f;
			}



			// 결과를 라디안에서 도 단위로 변환
			angles.Value.x = DirectX::XMConvertToDegrees(pitch);
			angles.Value.y = DirectX::XMConvertToDegrees(yaw);
			angles.Value.z = DirectX::XMConvertToDegrees(roll);

			return angles;
		}

		//======================================//
		//			   static method			//
		//======================================//

		static Quaternion FromVector(const _vector v)
		{
			Quaternion quaternion;
			XMStoreFloat4(&quaternion.Value, v);

			return quaternion;
		}

		static Quaternion Identity()
		{
			return Quaternion{ 0.0f, 0.0f, 0.0f, 1.0f };
		}

		static _float Dot(const Quaternion& lhs, const Quaternion& rhs)
		{
			const _vector q1 = lhs.ToVector();
			const _vector q2 = rhs.ToVector();
			const _vector result = DirectX::XMVector4Dot(q1, q2);

			return DirectX::XMVectorGetX(result);
		}

		static _float Angle(const Quaternion& lhs, const Quaternion& rhs)
		{
			_float dot = Dot(lhs, rhs);
			
			dot = Clamp(dot, -1.f, 1.f);

			const _float rad = std::acos(dot) * 2.f;

			return RadianToDegree(rad);
		}

		static Quaternion Inverse(const Quaternion& rotation)
		{
			const _vector q 		= rotation.ToVector();
			const _vector result 	= DirectX::XMQuaternionInverse(q);

			return FromVector(result);
		}

		static Quaternion AngleAxis(const _float degrees, const Vector3& axis)
		{
			const _float lenSq = axis.SqrMagnitude();
			if (lenSq < 1e-6f)
			{
				return Identity();
			}

			const Vector3 normal = axis.Normalized();
			const _float halfRad = DegreeToRadian(degrees) * 0.5f;
			const _float sin = std::sinf(halfRad);
			const _float cos = std::cosf(halfRad);
			
			return Quaternion{ normal.Value.x * sin, normal.Value.y * sin, normal.Value.z * sin, cos };
		}

		static Quaternion Euler(const _float xDegree, const _float yDegree, const _float zDegree)
		{
			const _float pitch = DirectX::XMConvertToRadians(xDegree);
			const _float yaw = DirectX::XMConvertToRadians(yDegree);
			const _float roll = DirectX::XMConvertToRadians(zDegree);

			//_vector qPitch 	= DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), pitch);
			//_vector qYaw 	= DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), yaw);
			//_vector qRoll 	= DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), roll);

			
			_vector q = DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
			//_vector q = DirectX::XMQuaternionMultiply(qYaw, DirectX::XMQuaternionMultiply(qPitch, qRoll));

			return FromVector(q);
		}

		static Quaternion Euler(const Vector3& euler)
		{
			return Euler(euler.Value.x, euler.Value.y, euler.Value.z);
		}

		static Quaternion FromToRotation(const Vector3& from, const Vector3& to)
		{
			const Vector3 fromNormal = from.Normalized();
			const Vector3 toNormal = to.Normalized();

			const _float dot = Vector3::Dot(fromNormal, toNormal);

			if (dot > 0.999999f)
			{
				return Identity();
			}

			else if (dot < -0.999999f)
			{
				Vector3 ortho = Vector3::Cross(Vector3::Up(), fromNormal);

				if (ortho.SqrMagnitude() < 1e-6f)
				{
					ortho = Vector3::Cross(Vector3::Forward(), fromNormal);
				}

				ortho = ortho.Normalized();

				return AngleAxis(180.f, ortho);
			}

			else
			{
				const _float deg = RadianToDegree(std::acos(dot));
				const Vector3 axis = Vector3::Cross(fromNormal, toNormal).Normalized();
				return AngleAxis(deg, axis);
			}
		}

		static Quaternion LookRotation(const Vector3& forward, const Vector3& up = Vector3::Up())
		{
			if (forward.SqrMagnitude() < 1e-6f)
			{
				return Identity();
			}

			Vector3 zAxis = forward.Normalized();
			Vector3 xAxis = Vector3::Cross(up, zAxis).Normalized();
			Vector3 yAxis = Vector3::Cross(zAxis, xAxis);

			_matrix rotationMatrix = {
				xAxis.Value.x, xAxis.Value.y, xAxis.Value.z, 0.f,
				yAxis.Value.x, yAxis.Value.y, yAxis.Value.z, 0.f,
				zAxis.Value.x, zAxis.Value.y, zAxis.Value.z, 0.f,
				0.f,      0.f,      0.f,    1.f
			};

			_vector quat = XMQuaternionRotationMatrix(rotationMatrix);
			return FromVector(quat);
		}

		static Quaternion Slerp(const Quaternion& a, const Quaternion& b, _float t)
		{
			if (t < 0.f)
			{
				t = 0.f;
			}

			if (t > 1.f)
			{
				t = 1.f;
			}

			return SlerpUnclamped(a, b, t);
		}

		static Quaternion SlerpUnclamped(const Quaternion& a, const Quaternion& b, _float t)
		{
			const _vector qa = a.ToVector();
			const _vector qb = b.ToVector();
			const _vector result = DirectX::XMQuaternionSlerp(qa, qb, t);

			return FromVector(result);
		}

		//======================================//
		//		   serialization field			//
		//======================================//

		friend void to_json(nlohmann::ordered_json& j, const Quaternion& q)
		{
			j = nlohmann::ordered_json{
				{ "x", q.Value.x },
				{ "y", q.Value.y },
				{ "z", q.Value.z },
				{ "w", q.Value.w }
			};
		}

		friend void from_json(const nlohmann::ordered_json& j, Quaternion& q)
		{
			j.at("x").get_to(q.Value.x);
			j.at("y").get_to(q.Value.y);
			j.at("z").get_to(q.Value.z);
			j.at("w").get_to(q.Value.w);
		}

	public:
		_float4 Value;
	};

	struct AnimationCurve
	{

		// Easing 함수는 시간 흐름에 따른 매개변수의 변화율을 지정합니다.
		// 대부분의 실제 사물들은 일정한 속도로 이동하지 않고, 즉시 시작하거나 즉시 멈추지도 않습니다.
		// 서랍을 예로 들자면, 처음에는 빠르게 열다가 거의 다 열었을 때쯤에는 천천히 엽니다.
		// 사물을 바닥에 떨어트렸을 때는 사물이 아래로 가속하다 사물이 바닥을 쳤을 때 튕겨 올라옵니다.
		//
		// 1. easeIn 계열 함수들은 점점 가속되는 효과를 제공합니다.
		// 2. easeOut 계열 함수들은 반대로 점점 감속되는 효과를 제공합니다.
		// 3. easeInOut 계열 함수들은 가속과 감속이 모두 적용되어 양쪽에서 부드럽게 전환됩니다.
		// 이 함수들은 t가 0에서 1까지 진행되며, 각각의 곡선을 정의합니다. 필요에 맞게 사용하시면 됩니다.
		//	참고 링크 : https://easings.net/ko

		// 사용법
		// a를 b까지 움직이는데 걸리는 시간(duration)이 있다고합니다.
		// POINT StartPosition = a; // 시작할 좌표
		// POINT EndPosition   = b; // 도착할 좌표
		//
		// float mCurrentTime = 0.f;
		// float mDurationTime = 2.f; // a를 b까지 움직이는데 2초가 걸린다.
		// void MoveAtoB()
		//{
		// 	 mCurrentTime += mDeltaTime;             // 프레임당 걸리는 시간 더해주기
		//   float t = mCurrentTime / mDurationTime;
		//   
		//   a = Lerp(StartPosition, EndPosition, EaseInSine(t));
		//}

		template<typename T>
		static T 		Lerp(T a, T b, _float t)
		{
			return a + t * (b - a);
		}

		static _float 	EaseInSine(const _float t)
		{
			return 1 - std::cos((t * PI) / 2);
		}
		static _float 	EaseOutSine(const _float t)
		{
			return std::sin((t * PI) / 2);
		}
		static _float 	EaseInOutSine(const _float t)
		{
			return -(std::cos(PI * t) - 1) / 2;
		}
		static _float 	EaseInQuad(const _float t)
		{
			return t * t;
		}
		static _float 	EaseOutQuad(const _float t)
		{
			return 1 - (1 - t) * (1 - t);
		}
		static _float 	EaseInOutQuad(const _float t)
		{
			return t < 0.5f ? 2.f * t * t : 1.f - std::pow(-2.f * t + 2.f, 2.f) * 0.5f;
		}
		static _float 	EaseInCubic(const _float t)
		{
			return t * t * t;
		}
		static _float 	EaseOutCubic(const _float t)
		{
			return 1.f - std::pow(1.f - t, 3.f);
		}
		static _float 	EaseInOutCubic(const _float t)
		{
			return t < 0.5 ? 4.f * t * t * t : 1.f - std::pow(-2.f * t + 2.f, 3.f) * 0.5f;
		}
		static _float 	EaseInQuart(const _float t)
		{
			return t * t * t * t;
		}
		static _float 	EaseOutQuart(const _float t)
		{
			return 1.f - std::pow(1.f - t, 4.f);
		}
		static _float 	EaseInOutQuart(const _float t)
		{
			return t < 0.5 ? 8.f * t * t * t * t : 1.f - std::pow(-2.f * t + 2.f, 4.f) * 0.5f;
		}
		static _float 	EaseInQuint(const _float t)
		{
			return t * t * t * t * t;
		}
		static _float 	EaseOutQuint(const _float t)
		{
			return 1.f - std::pow(1.f - t, 5.f);
		}
		static _float 	EaseInOutQuint(const _float t)
		{
			return t < 0.5 ? 16.f * t * t * t * t * t : 1.f - std::pow(-2.f * t + 2.f, 5.f) * 0.5f;
		}
		static _float 	EaseInExpo(const _float t)
		{
			return t == 0 ? 0 : std::pow(2.f, 10.f * t - 10.f);
		}
		static _float 	EaseOutExpo(const _float t)
		{
			return t == 1.f ? 1.f : 1.f - std::pow(2.f, -10.f * t);
		}
		static _float 	EaseInOutExpo(const _float t)
		{
			if (t == 0) return 0;
			if (t == 1) return 1;
			return t < 0.5 ? std::pow(2.f, 20.f * t - 10.f) / 2.f : (2.f - std::pow(2.f, -20.f * t + 10.f)) * 0.5f;
		}
		static _float 	EaseInCirc(const _float t)
		{
			return 1.f - std::sqrt(1.f - std::pow(t, 2.f));
		}
		static _float 	EaseOutCirc(const _float t)
		{
			return std::sqrt(1.f - std::pow(t - 1.f, 2.f));
		}
		static _float 	EaseInOutCirc(const _float t)
		{
			return t < 0.5f ? (1.f - std::sqrt(1.f - std::pow(2.f * t, 2.f))) / 2.f : (std::sqrt(1.f - std::pow(-2.f * t + 2.f, 2.f)) + 1.f) * 0.5f;
		}
		static _float 	EaseInBack(const _float t)
		{
			const _float c1 = 1.70158f;
			const _float c3 = c1 + 1.f;
			return c3 * t * t * t - c1 * t * t;
		}
		static _float 	EaseOutBack(const _float t)
		{
			const _float c1 = 1.70158f;
			const _float c3 = c1 + 1.f;
			return 1.f + c3 * std::pow(t - 1.f, 3.f) + c1 * std::pow(t - 1.f, 2.f);
		}
		static _float 	EaseInOutBack(const _float t)
		{
			const _float c1 = 1.70158f;
			const _float c2 = c1 * 1.525f;
			return t < 0.5f
				? (std::pow(2.f * t, 2.f) * ((c2 + 1.f) * 2.f * t - c2)) * 0.5f
				: (std::pow(2.f * t - 2.f, 2.f) * ((c2 + 1.f) * (t * 2.f - 2.f) + c2) + 2.f) * 0.5f;
		}
		static _float 	EaseInElastic(const _float t)
		{
			const _float c4 = (2.f * PI) / 3.f;
			return t == 0 ? 0 : t == 1.f ? 1.f : -std::pow(2.f, 10.f * t - 10.f) * std::sin((t * 10.f - 10.75f) * c4);
		}
		static _float 	EaseOutElastic(const _float t)
		{
			const _float c4 = (2.f * PI) / 3.f;
			return t == 0 ? 0 : t == 1.f ? 1.f : std::pow(2.f, -10.f * t) * std::sin((t * 10.f - 0.75f) * c4) + 1.f;
		}
		static _float 	EaseInOutElastic(const _float t)
		{
			const _float c5 = (2 * PI) / 4.5f;
			if (t == 0) return 0;
			if (t == 1.f) return 1.f;
			return t < 0.5f
				? -(std::pow(2.f, 20.f * t - 10.f) * std::sin((20.f * t - 11.125f) * c5)) * 0.5f
				: (std::pow(2.f, -20.f * t + 10.f) * std::sin((20.f * t - 11.125f) * c5)) * 0.5f + 1;
		}
		static _float 	EaseInBounce(const _float t)
		{
			return 1 - EaseOutBounce(1 - t);
		}
		static _float 	EaseOutBounce(_float t)
		{
			const _float n1 = 7.5625f;
			const _float d1 = 2.75f;

			if (t < 1 / d1) {
				return n1 * t * t;
			}
			else if (t < 2 / d1) {
				t -= 1.5f / d1;
				return n1 * t * t + 0.75f;
			}
			else if (t < 2.5 / d1) {
				t -= 2.25f / d1;
				return n1 * t * t + 0.9375f;
			}
			else {
				t -= 2.625f / d1;
				return n1 * t * t + 0.984375f;
			}
		}
		static _float 	EaseInOutBounce(const _float t)
		{
			return t < 0.5f
				? (1 - EaseOutBounce(1 - 2 * t)) * 0.5f
				: (1 + EaseOutBounce(2 * t - 1)) * 0.5f;
		}
	};

	inline Vector3 DecomposeEulerWithHistory(const Quaternion current, const Quaternion last, Vector3 euler)
	{
		const Quaternion deltaRotation = current * Quaternion::Inverse(last);

		Vector3 deltaEuler = deltaRotation.EulerAngles();

		deltaEuler.Value.x = (deltaEuler.Value.x > 180) ? deltaEuler.Value.x - 360 : deltaEuler.Value.x;
		deltaEuler.Value.y = (deltaEuler.Value.y > 180) ? deltaEuler.Value.y - 360 : deltaEuler.Value.y;
		deltaEuler.Value.z = (deltaEuler.Value.z > 180) ? deltaEuler.Value.z - 360 : deltaEuler.Value.z;

		return euler += deltaEuler;

		//Vector3 rawEuler = q.EulerAngles();

		//Vector3 newEuler;
		//for (int i = 0; i < 3; ++i)
		//{
		//	float diff = rawEuler[i] - oldEuler[i];

		//	while (diff < -180.0f) diff += 360.0f;
		//	while (diff > 180.0f) diff -= 360.0f;

		//	newEuler[i] = oldEuler[i] + diff;
		//}

		//return newEuler;
	}
}
