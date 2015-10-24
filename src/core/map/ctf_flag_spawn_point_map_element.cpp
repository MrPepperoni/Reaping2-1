#include "core/map/ctf_flag_spawn_point_map_element.h"

namespace map {
namespace ctf {

CtfFlagSpawnPointMapElement::CtfFlagSpawnPointMapElement()
    : MapElement()
    , mX(0.0)
    , mY(0.0)
    , mTeam(Team::Blue)
{
}

void CtfFlagSpawnPointMapElement::Load(Json::Value& setters)
{
    MapElement::Load(setters);
    Json::Value const& position=setters["position"];
    if(!position.isObject())
    {
        return;
    }
    int32_t x;
    if (!Json::GetInt(position["x"],x))
    {
        return;
    }
    SetX(x);
    int32_t y;
    if (!Json::GetInt(position["y"],y))
    {
        return;
    }
    SetY(y);
    std::string teamStr;
    if (!Json::GetStr(setters["team"],teamStr))
    {
        return;
    }
    SetTeam(Team::Get()(AutoId(teamStr)));
}

void CtfFlagSpawnPointMapElement::SetX(int32_t x)
{
    mX=x;
}

int32_t CtfFlagSpawnPointMapElement::GetX()const
{
    return mX;
}

void CtfFlagSpawnPointMapElement::SetY(int32_t y)
{
    mY=y;
}

int32_t CtfFlagSpawnPointMapElement::GetY()const
{
    return mY;
}

void CtfFlagSpawnPointMapElement::SetTeam(Team::Type team)
{
    mTeam=team;
}

Team::Type CtfFlagSpawnPointMapElement::GetTeam()const
{
    return mTeam;
}

} // namespace ctf
} // namespace map
