#include "utilities/collision_utils.h"

#include "raylib.h"
#include "raymath.h"

namespace CollisionUtils
{
    void PointNearestRect(const Rectangle& rect, const Vector3& point, Vector3* nearest, Vector3* normal)
    {
        // get the closest point on the vertical sides
        float hValue = float(rect.x);
        float hNormal = -1.0f;
        if (point.x > rect.x + rect.width)
        {
            hValue += 1;
            hNormal = 1;
        }

        Vector3 vecToPoint = Vector3{ hValue, rect.y, 0.0f } - point;

        // get the dot product between the ray and the vector to the point
        float dotForPoint = Vector3DotProduct(Vector3{ 0, -1, 0 }, vecToPoint);
        Vector3 nearestPoint = { hValue, 0, 0 };

        if (dotForPoint < 0)
            nearestPoint.y = float(rect.y);
        else if (dotForPoint >= 1)
            nearestPoint.y = float(rect.y + rect.height);
        else
            nearestPoint.y = rect.y + dotForPoint;

        // get the closest point on the horizontal sides
        float vValue = float(rect.y);
        float vNormal = -1;
        if (point.y > rect.y + rect.height)
        {
            vValue = rect.y + rect.height;
            vNormal = 1;
        }

        vecToPoint = Vector3{ rect.x, vValue, 0 } - point;
        // get the dot product between the ray and the vector to the point
        dotForPoint = Vector3DotProduct(Vector3{ -1, 0, 0 }, vecToPoint);
        *nearest = Vector3{ 0,vValue, 0 };

        if (dotForPoint < 0)
            nearest->x = rect.x;
        else if (dotForPoint >= 1)
            nearest->x = rect.x + rect.width;
        else
            nearest->x = rect.x + dotForPoint;

        if (Vector3LengthSqr(point - nearestPoint) < Vector3LengthSqr(point - *nearest))
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
        // get the closest point on the vertical sides
        float hValue = bbox.min.x + position.x;
        float hNormal = -1.0f;
        if (point.x > bbox.max.x + position.x)
        {
            hValue += 1;
            hNormal = 1;
        }

        Vector3 vecToPoint = Vector3{ hValue, bbox.min.y + position.y, 0.0f } - point;

        // get the dot product between the ray and the vector to the point
        float dotForPoint = Vector3DotProduct(Vector3{ 0, -1, 0 }, vecToPoint);
        Vector3 nearestPoint = { hValue, 0, 0 };

        if (dotForPoint < 0)
            nearestPoint.y = bbox.min.y + position.y;
        else if (dotForPoint >= 1)
            nearestPoint.y = bbox.max.y + position.y;
        else
            nearestPoint.y = bbox.min.y + position.y + dotForPoint;

        // get the closest point on the horizontal sides
        float vValue = bbox.min.y + position.y;
        float vNormal = -1;
        if (point.y > bbox.max.y + position.y)
        {
            vValue = bbox.max.y + position.y;
            vNormal = 1;
        }

        vecToPoint = Vector3{ bbox.min.x + position.x, vValue, 0 } - point;
        // get the dot product between the ray and the vector to the point
        dotForPoint = Vector3DotProduct(Vector3{ -1, 0, 0 }, vecToPoint);
        *nearest = Vector3{ 0,vValue, 0 };

        if (dotForPoint < 0)
            nearest->x = bbox.min.x + position.x;
        else if (dotForPoint >= 1)
            nearest->x = bbox.max.x + position.x;
        else
            nearest->x = bbox.min.x + position.x + dotForPoint;

        if (Vector3LengthSqr(point - nearestPoint) < Vector3LengthSqr(point - *nearest))
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