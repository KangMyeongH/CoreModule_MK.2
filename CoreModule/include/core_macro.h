#pragma once

#ifndef MSG_BOX
#define MSG_BOX(_message) MessageBox(nullptr, TEXT(message), L"System Message", MB_OK)
#endif

#define CLONE_SHARED_PTR(className) \
	new className(*this), [](const className* ptr) { delete ptr; }


#define DECLARE_REGISTER_COMPONENT(className) 																	\
public: 																										\
	SharedPtr<Component> Clone() const override                                     							\
	{                                                                               							\
		SharedPtr<className> clone(CLONE_SHARED_PTR(className));                    							\
        return clone;                                                               							\
	}																											\
																												\
private: 																										\
	static std::shared_ptr<className> create()																	\
	{																											\
		return std::shared_ptr<className>(new className(nullptr), [](const className* ptr) { delete ptr; });	\
	}																											\
																												\
	static ComponentRegistrar registrar_##className;

//=====================================================================================================================

#define DECLARE_REGISTER_SCRIPTBEHAVIOUR(className) 															\
public: 																										\
	engine::SharedPtr<engine::Component> Clone() const override                                     			\
	{                                                                               							\
		engine::SharedPtr<className> clone(CLONE_SHARED_PTR(className));                    					\
        return clone;                                                               							\
	}																											\
																												\
private: 																										\
	static std::shared_ptr<className> create()																	\
	{																											\
		return std::shared_ptr<className>(new className(nullptr), [](const className* ptr) { delete ptr; });	\
	}																											\
																												\
	static ComponentRegistrar registrar_##className;

#define DEFINE_REGISTER_COMPONENT(className) 																	\
	ComponentRegistrar engine::className::registrar_##className(#className, &engine::className::create);

#define DEFINE_REGISTER_SCRIPTBEHAVIOUR(className)																\
	ComponentRegistrar className::registrar_##className(#className, &className::create);

#define REGISTER_COMPONENT(className) \
	static ComponentRegistrar registrar_##className(#className, []() { return std::make_shared<className>(nullptr); });

#define REGISTER_SCRIPTBEHAVIOUR(className) \
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

#define LOG_FAIL(msg)  do{ std::cout << "VALIDATE ERROR: " << msg << '\n'; }while(0)