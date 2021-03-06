#ifndef INCLUDED_COMPONENT_REPO_H
#define INCLUDED_COMPONENT_REPO_H

#include "platform/factory.h"
#include "platform/singleton.h"
#include "core/component.h"

#include <memory>

class ComponentFactory : public platform::Factory<Component>, public platform::Singleton<ComponentFactory>
{
    friend class platform::Singleton<ComponentFactory>;
    ComponentFactory();
protected:
    template<typename Elem_T>
    static std::auto_ptr<Component> CreateComponent( int32_t Id );
};
template<typename Elem_T>
std::auto_ptr<Component> ComponentFactory::CreateComponent( int32_t Id )
{
    std::auto_ptr<Component> component( new Elem_T() );
    component->SetId( Id );
    return component;
}

#endif//INCLUDED_COMPONENT_REPO_H