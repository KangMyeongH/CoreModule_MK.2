#pragma once
#include "ComponentFactory.h"
#include "core_defines.h"

namespace engine
{
	class Component;
}

class COREMODULE_API ComponentRegistrar
{
public:
	ComponentRegistrar(const engine::_string& typeName, std::function<std::shared_ptr<engine::Component>()> creator)
	{
		ComponentFactory::GetInstance().componentFactory[typeName] = std::move(creator);
	}
};
