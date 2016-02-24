#include "platform/i_platform.h"
#include "engine/collisions/shot_collision_sub_system.h"
#include "engine/engine.h"
#include "platform/auto_id.h"
#include "core/i_health_component.h"
#include "core/shot_collision_component.h"
#include "core/i_position_component.h"
#include "core/i_notify_parent_on_death_component.h"
#include "core/i_owner_component.h"

namespace engine {

ShotCollisionSubSystem::ShotCollisionSubSystem()
    : CollisionSubSystem()
{

}

void ShotCollisionSubSystem::Init()
{
}

void ShotCollisionSubSystem::Update(Actor& actor, double DeltaTime)
{
    if (mActorsCollided.empty())
    {
        return;
    }
    Opt<ShotCollisionComponent> shotCC=actor.Get<ShotCollisionComponent>();
    if (shotCC->IsHitClosest())
    {
        Opt<Actor> closestActor;
        double lowestDist=std::numeric_limits<double>::max();
        for (ActorsCollided_t::iterator it=mActorsCollided.begin(),e=mActorsCollided.end();it!=e;++it)
        {
            Actor& other=**it;
            Opt<IPositionComponent> positionC = actor.Get<IPositionComponent>();
            Opt<IPositionComponent> otherPositionC = other.Get<IPositionComponent>();
            double distPow=std::pow(positionC->GetX()-otherPositionC->GetX(),2)
                +std::pow(positionC->GetY()-otherPositionC->GetY(),2);
            if (distPow<lowestDist)
            {
                lowestDist=distPow;
                closestActor=*it;
            }
        }
        TakeDamage(actor,*closestActor,shotCC);
    }
    else
    {
        for (ActorsCollided_t::iterator it=mActorsCollided.begin(),e=mActorsCollided.end();it!=e;++it)
        {
            Actor& other=**it;
            TakeDamage(actor, other, shotCC);

        }
    }
    mActorsCollided.clear();
}

void ShotCollisionSubSystem::ClipScene(Actor& actor)
{
    Opt<ShotCollisionComponent> shotCC=actor.Get<ShotCollisionComponent>();
    Opt<IPositionComponent> positionC = actor.Get<IPositionComponent>();
    Opt<IHealthComponent> healthC = actor.Get<IHealthComponent>();

    double radius=shotCC->GetRadius();
    glm::vec4 const& AllowedDimensions = mScene.GetDimensions();
    if( positionC->GetX() + radius < AllowedDimensions.x * 2 ||
        positionC->GetX() - radius > AllowedDimensions.z * 2 ||
        positionC->GetY() + radius < AllowedDimensions.y * 2 ||
        positionC->GetY() - radius > AllowedDimensions.w * 2 )
    {
        healthC->SetHP(0);
    }
}

void ShotCollisionSubSystem::Collide(Actor& actor, Actor& other)
{
    Opt<ShotCollisionComponent> shotCC=actor.Get<ShotCollisionComponent>();
    if( other.GetGUID() == shotCC->GetParentGuid() )
    {
        return;
    }

    Opt<IHealthComponent> healthC = actor.Get<IHealthComponent>();

    mActorsCollided.push_back(Opt<Actor>(&other));

    if (healthC.IsValid())
    {
        Opt<ICollisionComponent> otherCollC = other.Get<ICollisionComponent>();
        if( otherCollC.IsValid() && !shotCC->CanPassThrough( otherCollC->GetCollisionClass() ) )
        {
            healthC->SetHP(0);
        }
    }
    Opt<INotifyParentOnDeathComponent> notifyParentC=actor.Get<INotifyParentOnDeathComponent>();
    if(notifyParentC.IsValid())
    {
        notifyParentC->SetKillerGUID(other.GetGUID());
    }
}

void ShotCollisionSubSystem::TakeDamage(Actor &actor, Actor &target, Opt<ShotCollisionComponent> shotCC)
{
    Opt<IHealthComponent> otherHealthC=target.Get<IHealthComponent>();
    Opt<IOwnerComponent> ownerC=actor.Get<IOwnerComponent>();

    if((!shotCC->IsDamageOnce()||shotCC->GetDamagedActorIds().find(target.GetGUID())==shotCC->GetDamagedActorIds().end())
        &&otherHealthC.IsValid()&&otherHealthC->IsAlive())
    {
        otherHealthC->TakeDamage(shotCC->GetDamage());
        shotCC->AddDamagedActorId(target.GetGUID());
        if (ownerC.IsValid())
        {
            otherHealthC->SetLastDamageOwnerGUID(ownerC->GetOwnerGUID());
        }
    }
}

} // namespace engine

