#include "i_brush.h"
#include "../i_position_component.h"
#include "../i_collision_component.h"
#include "editor_target_system.h"
#include "editor_system.h"
#include "editor_layer_system.h"

namespace map {

IBrush::IBrush( int32_t Id )
    : mId( Id )
    , mScene( Scene::Get() )
{

}

std::vector<int32_t> IBrush::GetActorsToRemove()
{
    std::vector<int32_t> r;
    auto cur = EditorTargetSystem::Get()->GetCursor();
    if( !cur.IsValid() )
    {
        return r;
    }
    EditorLayer::Type editorLayer = EditorLayerSystem::Get()->GetEditorLayerType();
    glm::vec2 mousePos = EditorTargetSystem::Get()->GetCursorPosition();
    int32_t curGUID = cur->GetGUID();
    Opt<IRenderableComponent> cursorRenderableC( cur->Get<IRenderableComponent>() );
    for( ActorList_t::iterator it = mScene.GetActors().begin(), e = mScene.GetActors().end(); it != e; ++it )
    {
        Actor& actor = **it;
        Opt<IPositionComponent> positionC = actor.Get<IPositionComponent>();
        Opt<ICollisionComponent> collisionC = actor.Get<ICollisionComponent>();
        if ( !positionC.IsValid() || !collisionC.IsValid() )
        {
            continue;
        }
        if ( curGUID != actor.GetGUID()
             && std::abs( positionC->GetX() - mousePos.x ) < collisionC->GetRadius()
             && std::abs( positionC->GetY() - mousePos.y ) < collisionC->GetRadius() )
        {
            Opt<IRenderableComponent> renderableC( actor.Get<IRenderableComponent>() );
            if (renderableC.IsValid()
                && ( editorLayer == EditorLayer::Any
                   || ( editorLayer == EditorLayer::Target
                      && cursorRenderableC.IsValid()
                      && renderableC->GetLayerPriority() == cursorRenderableC->GetLayerPriority() ) ) )
            {
                r.push_back( actor.GetGUID() );
            }
        }
    }
    return r;
}

IBrush::~IBrush()
{

}

} // namespace map

