#pragma once
#include <memory>

#include "core_defines.h"
#include "core_typedef.h"

namespace engine
{
	class Component;
}

class ComponentFactory
{
private:
	//======================================//
	//				constructor				//
	//======================================//

	ComponentFactory() = default;
	~ComponentFactory() = default;
public:
	ComponentFactory(const ComponentFactory&) = delete;
	ComponentFactory& operator=(const ComponentFactory&) = delete;
	ComponentFactory(ComponentFactory&&) = delete;
	ComponentFactory& operator=(ComponentFactory&&) = delete;

public:
	//======================================//
	//				  method				//
	//======================================//

	static ComponentFactory& GetInstance()
	{
		static ComponentFactory s_Instance;
		return s_Instance;
	}

	std::shared_ptr<engine::Component> CreateComponent(const engine::_string& typeName);

	//======================================//
	//				  fields				//
	//======================================//

	std::unordered_map<engine::_string, std::function<std::shared_ptr<engine::Component>()>> componentFactory;
};
