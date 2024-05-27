#pragma once
#include "Entity.h"

#define OBJECT_PHYSICAL_SIZE    14
#define OBJECT_FRAME_SIZE        16

#define POINTS_HEART    0

enum class ObjectType { CHAIN, SHIELD, KEY_CHEST, KEY_DOOR, CHEST_CHAIN, CHEST_SHIELD };

class Object : public Entity
{
public:
    Object(const Point& p, ObjectType t);
    ~Object();

    void DrawDebug(const Color& col) const;
    int Points() const;

    ObjectType GetType()const;

private:
    ObjectType type;
};