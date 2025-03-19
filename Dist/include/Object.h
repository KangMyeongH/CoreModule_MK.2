#pragma once
#include <atomic>
#include <memory>

#include "core_defines.h"

namespace engine
{
	class COREMODULE_API Object : public std::enable_shared_from_this<Object>
	{
	protected:
		//======================================//
		//				constructor				//
		//======================================//

		explicit Object(_string name = "Unnamed Object")
			: m_bDestroyed(false), m_ID(s_IdGenerator++), m_Name(std::move(name))
		{
			
		}

		virtual ~Object() = default;

		Object(const Object& rhs)
			: m_bDestroyed(rhs.m_bDestroyed), m_ID(s_IdGenerator++), m_Name(rhs.m_Name)
		{

		}

		Object(Object&& rhs) noexcept : m_bDestroyed(rhs.m_bDestroyed), m_ID(s_IdGenerator++), m_Name(std::move(rhs.m_Name))
		{
			rhs.m_ID = 0;
			rhs.m_Name = {};
			rhs.m_bDestroyed = false;
		}

		Object& operator=(const Object& rhs)
		{
			m_ID = s_IdGenerator++;
			m_Name = rhs.m_Name;
			m_bDestroyed = rhs.m_bDestroyed;

			return *this;
		}

		Object& operator=(Object&& rhs) noexcept
		{
			m_ID = s_IdGenerator++;
			m_Name = rhs.m_Name;
			m_bDestroyed = rhs.m_bDestroyed;
			rhs.m_ID = 0;
			rhs.m_Name = {};
			rhs.m_bDestroyed = false;

			return *this;
		}

	public:
		//======================================//
        //				  method				//
        //======================================//
        
		void			SetName(const _string& name) { m_Name = name; }
		_string			GetName() const { return m_Name; }
		void			SetInstanceID(const _int id) { m_ID = id; }
		_int			GetInstanceID() const { return m_ID; }
		_bool			IsDestroyed() const { return m_bDestroyed; }

		virtual void 	Destroy() = 0;

		//======================================//
		//			   static method			//
		//======================================//

		static _int		GetMaxID()
		{
			return s_IdGenerator;
		}

	protected:
		//======================================//
		//				  fields				//
		//======================================//

		_bool						m_bDestroyed;	// 삭제 플래그

	private:
		static std::atomic<_int> 	s_IdGenerator; 	// 고유 ID 생성기
		_int						m_ID;			// 각 객체의 고유 ID
		_string						m_Name;			// 객체 이름
	};
}

