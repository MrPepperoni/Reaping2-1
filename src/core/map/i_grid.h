#ifndef INCLUDED_CORE_MAP_I_GRID_H
#define INCLUDED_CORE_MAP_I_GRID_H

#include "platform/i_platform.h"
namespace map {

class IGrid
{
public:
    IGrid( int32_t Id );
    virtual void Update( double DeltaTime )=0;
    virtual glm::vec2 GetProcessedPosition();
    virtual void SetCursorPosition(double x, double y);
protected:
    glm::vec2 mCursorPosition;
    glm::vec2 mProcessedPosition;
    int32_t mId;
};



} // namespace map
#endif//INCLUDED_CORE_MAP_I_GRID_H