#include "utilities/collision_utils.h"

#include "raylib.h"
#include "raymath.h"

namespace CollisionUtils
{
    void PointNearestRect(const Rectangle& rect, const Vector2& point, Vector2* nearest, Vector2* normal)
    {
        // get the closest point on the vertical sides
        float hValue = rect.x;
        float hNormal = -1;
        if (point.x > rect.x + rect.width)
        {
            hValue = rect.x + rect.width;
            hNormal = 1;
        }

        Vector2 vecToPoint = Vector2Subtract(Vector2 { hValue, rect.y }, point);
        // get the dot product between the ray and the vector to the point
        float dotForPoint = Vector2DotProduct(Vector2 { 0, -1 }, vecToPoint);
        Vector2 nearestPoint = { hValue,0 };

        if (dotForPoint < 0)
            nearestPoint.y = rect.y;
        else if (dotForPoint >= rect.height)
            nearestPoint.y = rect.y + rect.height;
        else
            nearestPoint.y = rect.y + dotForPoint;

        // get the closest point on the horizontal sides
        float vValue = rect.y;
        float vNormal = -1;
        if (point.y > rect.y + rect.height)
        {
            vValue = rect.y + rect.height;
            vNormal = 1;
        }

        vecToPoint = Vector2Subtract(Vector2 { rect.x, vValue }, point);
        // get the dot product between the ray and the vector to the point
        dotForPoint = Vector2DotProduct(Vector2 { -1, 0 }, vecToPoint);
        *nearest = Vector2{ 0, vValue };

        if (dotForPoint < 0)
            nearest->x = rect.x;
        else if (dotForPoint >= rect.width)
            nearest->x = rect.x + rect.width;
        else
            nearest->x = rect.x + dotForPoint;

        if (Vector2LengthSqr(Vector2Subtract(point, nearestPoint)) < Vector2LengthSqr(Vector2Subtract(point, *nearest)))
        {
            *nearest = nearestPoint;

            if (normal)
            {
                normal->x = hNormal;
                normal->y = 0;
            }
        }
        else
        {
            if (normal)
            {
                normal->y = vNormal;
                normal->x = 0;
            }
        }
    }

    void PointNearestBoundsXY(const BoundingBox& bbox, const Vector3& position, const Vector3& point, Vector3* nearest, Vector3* normal)
    {
        Vector3 min = position + bbox.min;
        Vector3 max = position + bbox.max;
        Vector3 size = bbox.max - bbox.min;

        // get the closest point on the vertical sides
        float hValue = min.x;
        float hNormal = -1;
        if (point.x > max.x )
        {
            hValue = max.x;
            hNormal = 1;
        }

        Vector3 vecToPoint = Vector3Subtract(Vector3{ hValue, min.y, 0 }, point);
        // get the dot product between the ray and the vector to the point
        float dotForPoint = Vector3DotProduct(Vector3{ 0, -1, 0 }, vecToPoint);
        Vector3 nearestPoint = { hValue,0 };

        if (dotForPoint < 0)
            nearestPoint.y = min.y;
        else if (dotForPoint >= size.y)
            nearestPoint.y = max.y;
        else
            nearestPoint.y = min.y + dotForPoint;

        // get the closest point on the horizontal sides
        float vValue = min.y;
        float vNormal = -1;
        if (point.y > max.y)
        {
            vValue = max.y;
            vNormal = 1;
        }

        vecToPoint = Vector3Subtract(Vector3{ min.x, vValue, 0 }, point);
        // get the dot product between the ray and the vector to the point
        dotForPoint = Vector3DotProduct(Vector3{ -1, 0, 0 }, vecToPoint);
        *nearest = Vector3{ 0, vValue, 0 };

        if (dotForPoint < 0)
            nearest->x = min.x;
        else if (dotForPoint >= size.x)
            nearest->x = max.x;
        else
            nearest->x = min.x + dotForPoint;

        if (Vector3LengthSqr(Vector3Subtract(point, nearestPoint)) < Vector3LengthSqr(Vector3Subtract(point, *nearest)))
        {
            *nearest = nearestPoint;

            if (normal)
            {
                normal->x = hNormal;
                normal->y = 0;
            }
        }
        else
        {
            if (normal)
            {
                normal->y = vNormal;
                normal->x = 0;
            }
        }
    }
}