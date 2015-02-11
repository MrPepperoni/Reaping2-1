#include "network/create_actor_message_sender_system.h"
#include "create_actor_message.h"
#include "core/shot_collision_component.h"
#include "set_ownership_message.h"
#include "position_message_sender_system.h"
#include "move_message_sender_system.h"
namespace network {


    CreateActorMessageSenderSystem::CreateActorMessageSenderSystem()
        : MessageSenderSystem()
    {

    }

    void CreateActorMessageSenderSystem::Init()
    {
        MessageSenderSystem::Init();
        mOnActorEvent = EventServer<ActorEvent>::Get().Subscribe( boost::bind( &CreateActorMessageSenderSystem::OnActorEvent, this, _1 ) );
    }

    void CreateActorMessageSenderSystem::Update(double DeltaTime)
    {
        MessageSenderSystem::Update(DeltaTime);
    }

    void CreateActorMessageSenderSystem::OnActorEvent(ActorEvent const& Evt)
    {
        std::auto_ptr<CreateActorMessage> createActorMsg(new CreateActorMessage);
        createActorMsg->mActorGUID=Evt.mActor->GetGUID();
        createActorMsg->mActorId=Evt.mActor->GetId();
        createActorMsg->mState=Evt.mState;
        Opt<ShotCollisionComponent> shotCollisionC=Evt.mActor->Get<ShotCollisionComponent>();
        if (shotCollisionC.IsValid())
        {
            createActorMsg->mParentGUID=shotCollisionC->GetParentGuid();
        }
        mMessageHolder.AddOutgoingMessage(createActorMsg);
        if(Evt.mState==ActorEvent::Added)
        {
            mMessageHolder.AddOutgoingMessage(PositionMessageSenderSystem::GeneratePositionMessage(*Evt.mActor));
            mMessageHolder.AddOutgoingMessage(MoveMessageSenderSystem::GenerateMoveMessage(*Evt.mActor));
        }
    }

} // namespace engine
