#include "platform/i_platform.h"
#include "core/grid.h"
#include "core/i_position_component.h"
#include "core/i_move_component.h"
#include "core/i_collision_component.h"
#include "collision_model.h"

const uint32_t Grid::Collisions[] =
{
    0,                                                                                                              // no collision
    1 << CollisionClass::Creep | 1 << CollisionClass::Mine | 1 << CollisionClass::Player | 1 << CollisionClass::Wall, // projectile
    1 << CollisionClass::Creep | 1 << CollisionClass::Player, // AoeDamage
    1 << CollisionClass::Projectile | 1 << CollisionClass::AoeDamage | 1 << CollisionClass::Mine | 1 << CollisionClass::Player | 1 << CollisionClass::Wall | 1 << CollisionClass::Water, // creep
    1 << CollisionClass::Projectile | 1 << CollisionClass::Creep | 1 << CollisionClass::Wall,                       // mine
    1 << CollisionClass::Projectile | 1 << CollisionClass::AoeDamage | 1 << CollisionClass::Creep | 1 << CollisionClass::Player | 1 << CollisionClass::Wall | 1 << CollisionClass::Pickup | 1 << CollisionClass::Water | 1 << CollisionClass::Flag, // player
    1 << CollisionClass::Projectile | 1 << CollisionClass::Creep | 1 << CollisionClass::Mine | 1 << CollisionClass::Player, // wall
    1 << CollisionClass::Creep | 1 << CollisionClass::Player, // water
    1 << CollisionClass::Player,                                                                                    // pickup
    1 << CollisionClass::Player | 1 << CollisionClass::Flag, // flag
};

PossibleCollisions_t Grid::GetPossibleCollisions()const
{
    PossibleCollisions_t PossibleCollisions;
    for( Cells_t::const_iterator i = mCells.begin(), e = mCells.end(); i != e; ++i )
    {
        Cell const& C = *i;
        if (C.mActors[CollisionClass::Player].empty() &&
            C.mActors[CollisionClass::Projectile].empty() &&
            C.mActors[CollisionClass::Creep].empty())
        {
            continue;
        }
        for (size_t k = 0; k < CollisionClass::Num_Classes; ++k)
        {
            Actors_t const& Actors = C.mActors[k];
            for( size_t j = k; j < CollisionClass::Num_Classes; ++j )
            {
                if( !( Collisions[k] & ( 1 << j ) ) )
                {
                    continue;
                }
                Actors_t const& Actors2 = C.mActors[j];
                for( Actors_t::const_iterator a = Actors.begin(), ea = Actors.end(); a != ea; ++a )
                    for( Actors_t::const_iterator b = Actors2.begin(), eb = Actors2.end(); b != eb; ++b )
                    {
                        if( *a == *b )
                        {
                            continue;
                        }
                        PossibleCollisions.insert( CollPair( *a, *b ) );
                    }
            }
        }
    }
    return PossibleCollisions;
}

void Grid::Build( glm::vec4 const& Dimensions, float CellSize )
{
    mActorInCell.clear();
    mCells.clear();
    mCellSize = CellSize;
    mMin = glm::vec2( Dimensions.x, Dimensions.y );
    if( mCellSize < std::numeric_limits<float>::epsilon() )
    {
        mDimX = mDimY = 0;
        assert( false );
        return;
    }
    float const Width = Dimensions.z - Dimensions.x;
    float const Height = Dimensions.w - Dimensions.y;
    mDimX = ( size_t )glm::ceil( Width / mCellSize );
    mDimY = ( size_t )glm::ceil( Height / mCellSize );
    mCells.resize( mDimX * mDimY );
}

std::set<Actor*> Grid::GetAllCollidingActors( glm::vec2 const& position, double radius, CollisionClass::Type collClass ) const
{
    glm::vec4 ActorDim( position.x - radius - mMin.x, position.y - radius - mMin.y,
            position.x + radius - mMin.x, position.y + radius - mMin.y );

    size_t const Ex = ( size_t )glm::floor( std::max<float>( 0.0f, ActorDim.z ) / mCellSize );
    size_t const Ey = ( size_t )glm::floor( std::max<float>( 0.0f, ActorDim.w ) / mCellSize );
    size_t const Sx = ( size_t )glm::floor( std::max<float>( 0.0f, ActorDim.x ) / mCellSize );
    size_t const Sy = ( size_t )glm::floor( std::max<float>( 0.0f, ActorDim.y ) / mCellSize );
    std::set<Actor*> rv;
    for( size_t y = Sy, ey = std::min<size_t>( Ey + 1, mDimY ); y < ey; ++y )
    {
        for( size_t x = Sx, ex = std::min<size_t>( Ex + 1, mDimX ); x < ex; ++x )
        {
            auto const& cell = mCells[y * mDimX + x];
            for( size_t j = 0; j < CollisionClass::Num_Classes; ++j )
            {
                if( !( Collisions[collClass] & ( 1 << j ) ) )
                {
                    continue;
                }
                rv.insert( cell.mActors[j].begin(), cell.mActors[j].end() );
            }
        }
    }
    return rv;
}

void Grid::AddActor( Actor* A, double Dt, Opt<ICollisionComponent> collisionC )
{
    int32_t const CC = collisionC->GetCollisionClass();
    if( !Collisions[CC] )
    {
        return;
    }
    auto& cells = mActorInCell[ A ];
    for( auto& cell : cells )
    {
        auto& actors = cell->mActors[ CC ];
        actors.erase( std::remove( actors.begin(), actors.end(), A ), actors.end() );
    }
    cells.clear();
    size_t const NumCells = mCells.size();
    if( NumCells == 0 )
    {
        assert( false );
        return;
    }
    else if( NumCells == 1 )
    {
        mCells[0].mActors[CC].push_back( A );
        cells.push_back( &mCells[0] );
        return;
    }
    glm::vec4 const& ActorDim = Box( *A, Dt );

    size_t const Ex = ( size_t )glm::floor( std::max<float>( 0.0f, ActorDim.z ) / mCellSize );
    size_t const Ey = ( size_t )glm::floor( std::max<float>( 0.0f, ActorDim.w ) / mCellSize );
    size_t const Sx = ( size_t )glm::floor( std::max<float>( 0.0f, ActorDim.x ) / mCellSize );
    size_t const Sy = ( size_t )glm::floor( std::max<float>( 0.0f, ActorDim.y ) / mCellSize );
    for( size_t y = Sy, ey = std::min<size_t>( Ey + 1, mDimY ); y < ey; ++y )
    {
        for( size_t x = Sx, ex = std::min<size_t>( Ex + 1, mDimX ); x < ex; ++x )
        {
            auto& cell = mCells[y * mDimX + x];
            cell.mActors[CC].push_back( A );
            cells.push_back( &cell );
        }
    }
}

glm::vec4 Grid::Box( Actor const& Obj, double Dt )const
{
    Opt<IMoveComponent> moveC = Obj.Get<IMoveComponent>();
    float const MvX = moveC.IsValid() ? Dt * moveC->GetSpeedX() : 0.0f;
    float const MvY = moveC.IsValid() ? Dt * moveC->GetSpeedY() : 0.0f;

    double const Radius = Obj.Get<ICollisionComponent>()->GetRadius();
    Opt<IPositionComponent> const objPositionC = Obj.Get<IPositionComponent>();
    double const Ox = objPositionC->GetX() - mMin.x;
    double const Oy = objPositionC->GetY() - mMin.y;
    glm::vec4 Ret( Ox - Radius,
                   Oy - Radius,
                   Ox + Radius,
                   Oy + Radius );
    if( MvX < 0.0 )
    {
        Ret.x += MvX;
    }
    else
    {
        Ret.z += MvX;
    }
    if( MvY < 0.0 )
    {
        Ret.y += MvY;
    }
    else
    {
        Ret.w += MvY;
    }
    return Ret;
}

void Grid::RemoveActor( Actor* A )
{
    auto& cells = mActorInCell[ A ];
    for( auto& cell : cells )
    {
        for( auto& actors : cell->mActors )
        {
            actors.erase( std::remove( actors.begin(), actors.end(), A ), actors.end() );
        }
    }
    mActorInCell.erase( A );
}

CollPair::CollPair( Actor* a1, Actor* a2 ) : A1( a1 < a2 ? a1 : a2 )
    , A2( a1 < a2 ? a2 : a1 )
{
    assert( a1 != a2 );
}

bool CollPairCmp::operator()( CollPair const& a, CollPair const& b ) const
{
    return a.A1 < b.A1 || ( a.A1 == b.A1 && a.A2 < b.A2 );
}
