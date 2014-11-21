#ifndef INCLUDED_CORE_I_HEALTH_COMPONENT_H
#define INCLUDED_CORE_I_HEALTH_COMPONENT_H
#include "component.h"

class Actor;
class IHealthComponent : public Component
{
public:
    DEFINE_COMPONENT_BASE(IHealthComponent)
    virtual void Update( double Seconds )=0;
    virtual int32_t const& GetHP()const=0;
    virtual void SetHP( int32_t Hp )=0;
    virtual bool IsAlive()const=0;
    virtual double GetTimeOfDeath()const=0;
    virtual void SetActor( Actor* Obj )=0;
    virtual bool NeedDelete()const=0;
protected:
    friend class ComponentFactory;
};

#endif//INCLUDED_CORE_I_HEALTH_COMPONENT_H