#pragma once

struct Rectangle;
struct Vector2;
struct Vector3;
struct BoundingBox;

namespace CollisionUtils
{
    void PointNearestRect(const Rectangle& rect, const Vector2& point, Vector2* nearest, Vector2* normal);
    void PointNearestBoundsXY(const BoundingBox& rect, const Vector3& position, const Vector3& point, Vector3* nearest, Vector3* normal);

    void SetUnitAngleDeg(float& angle);
}