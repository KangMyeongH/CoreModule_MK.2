#pragma once

#ifndef MSG_BOX
#define MSG_BOX(_message) MessageBox(nullptr, TEXT(message), L"System Message", MB_OK)
#endif

#define CLONE_SHARED_PTR(className) \
	new className(*this), [](const className* ptr) { delete ptr; }

#define DECLARE_REGISTER_COMPONENT(className)										\
    /* friend 구조체: protected 생성자 접근 우회용 */									\
    friend struct __FactoryHelper_##className;										\
private:																			\
    /* 헬퍼 구조체: 클래스 내부에서만 유효하므로 protected 생성자 new/delete 가능 */		\
    struct __FactoryHelper_##className												\
    {                                                                             	\
        static std::shared_ptr<className> Create()                                	\
        {                                                                         	\
            /* 커스텀 deleter로 'delete ptr' 수행 -> protected 소멸자도 접근 가능 */	\
            return std::shared_ptr<className>(                                    	\
                new className(nullptr),                                          	\
                [](className* ptr){ delete ptr; }                                  	\
            );                                                                    	\
        }                                                                         	\
    };                                                                            	\
    /* 실제 등록 작업을 수행할 정적 함수/멤버(선언) */                            		\
    static ComponentRegistrar& getRegistrar();                                    	\
    static int s_registrar;                                                         \
public: 																			\
	SharedPtr<Component> Clone() const override                                     \
	{                                                                               \
		SharedPtr<className> clone(CLONE_SHARED_PTR(className));                    \
        return clone;                                                               \
	}

#define DEFINE_REGISTER_COMPONENT(className)                                      	\
    /* 정적 함수(팩토리 등록자) 정의 */                                            	\
    ComponentRegistrar& engine::className::getRegistrar()                           \
    {                                                                              	\
        static ComponentRegistrar s_reg(                                          	\
            #className,                                                           	\
            []() -> std::shared_ptr<engine::Component> {                          	\
                /* friend 헬퍼 구조체를 통해 Create 호출 */                        	\
                return __FactoryHelper_##className::Create();                     	\
            }                                                                      	\
        );                                                                         	\
        return s_reg;                                                             	\
    }                                                                              	\
    /* 정적 int 멤버 정의: 여기서 getRegistrar()를 '강제 호출'하여 등록 유발 */     	\
    int engine::className::s_registrar = [](){                                      \
        getRegistrar(); /* 호출해서 static ComponentRegistrar를 초기화 */          	\
        return 0;                                                                 	\
    }();

#define REGISTER_COMPONENT(className) \
	static ComponentRegistrar registrar_##className(#className, []() { return std::make_shared<className>(nullptr); });

#pragma region null_or_failed_check

#define NULL_CHECK( _ptr)	\
	{if( _ptr == 0){ return;}}

#define NULL_CHECK_RETURN( _ptr, _return)	\
	{if( _ptr == 0){return _return;}}

#define NULL_CHECK_MSG( _ptr, _message )		\
	{if( _ptr == 0){MessageBox(NULL, _message, L"System Message",MB_OK);}}

#define NULL_CHECK_RETURN_MSG( _ptr, _return, _message )	\
	{if( _ptr == 0){MessageBox(NULL, _message, L"System Message",MB_OK);return _return;}}

#define FAILED_CHECK(_hr)	if( ((HRESULT)(_hr)) < 0 )	\
	{ MessageBoxW(NULL, L"Failed", L"System Error",MB_OK);  return E_FAIL;}

#define FAILED_CHECK_RETURN(_hr, _return)	if( ((HRESULT)(_hr)) < 0 )		\
	{ MessageBoxW(NULL, L"Failed", L"System Error",MB_OK);  return _return;}

#define FAILED_CHECK_MSG( _hr, _message)	if( ((HRESULT)(_hr)) < 0 )	\
	{ MessageBoxW(NULL, _message, L"System Message",MB_OK); return E_FAIL;}

#define FAILED_CHECK_RETURN_MSG( _hr, _return, _message)	if( ((HRESULT)(_hr)) < 0 )	\
	{ MessageBoxW(NULL, _message, L"System Message",MB_OK); return _return;}

#pragma endregion null_or_failed_check

#pragma region Singleton

#define NO_COPY(ClassName)									\
		ClassName(const ClassName&) = delete; 				\
		ClassName(ClassName&&) = delete;					\
		ClassName& operator=(const ClassName&) = delete; 	\
		ClassName& operator=(ClassName&&) = delete; 		

#define DECLARE_SINGLETON(ClassName)						\
		NO_COPY(ClassName)									\
		public:												\
		static ClassName& GetInstance();


#define IMPLEMENT_SINGLETON(ClassName)						\
		ClassName& ClassName::GetInstance()					\
		{													\
			static ClassName sClass;						\
			return sClass;									\
		}

#pragma endregion Singleton