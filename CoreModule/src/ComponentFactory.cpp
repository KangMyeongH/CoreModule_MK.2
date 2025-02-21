#include "ComponentFactory.h"
#include "Component.h"

std::shared_ptr<engine::Component> ComponentFactory::CreateComponent(const engine::_string& typeName)
{
	if (componentFactory.find(typeName) != componentFactory.end())
	{
		auto component = componentFactory[typeName]();
		const engine::Component& ref = *component;

		return ref.Clone();
	}

	return nullptr;
}
