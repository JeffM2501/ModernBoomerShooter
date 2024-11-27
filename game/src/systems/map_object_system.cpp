#include "systems/map_object_system.h"

#include "world.h"
#include "systems/scene_render_system.h"
#include "components/map_object_component.h"
#include "components/transform_component.h"
#include "services/model_manager.h"
#include "services/global_vars.h"

#include "utilities/collision_utils.h"

#include "raymath.h"


void MapObjectSystem::OnSetup()
{
    SceneRenderer = WorldPtr->GetSystem<SceneRenderSystem>();
}

void MapObjectSystem::OnUpdate()
{
    // AI updates?
}

void MapObjectSystem::OnAddObject(GameObject* object)
{
    if (object->HasComponent<MapObjectComponent>())
    {
        MapObjects.push_back(object->GetComponent<MapObjectComponent>());

        MapObjectComponent* mapObject = MapObjects.back();

        mapObject->Instance = ModelManager::GetModel(mapObject->ModelName);

        if (SceneRenderer)
            SceneRenderer->MapObjectAdded(mapObject);
    }
}

bool MapObjectSystem::MoveEntity(Vector3& position, Vector3& desiredMotion, float radius)
{
    bool hitSomething = false;

    if (Vector3Equals(desiredMotion, Vector3Zeros))
        return false;

    Vector3 newPos = position + desiredMotion;

    for (auto* object : MapObjects)
    {
        if (!object->Solid)
            continue;
        
        // TODO check if the object is near the entity

        float motionVecLen = Vector3Length(desiredMotion);
        if (motionVecLen < FLT_MIN)
            break;

        BoundingBox bbox = object->Instance->Geometry->GetBounds();

        Vector3 targetVec = desiredMotion / motionVecLen;

        TransformComponent& transform = object->GetOwner()->MustGetComponent<TransformComponent>();

        // check bounds

        if (GlobalVars::ShowCollisionVolumes)
        {
            Rectangle rect = { 0,0,0,0 };
            rect.x = transform.Position.x + bbox.min.x;
            rect.y = transform.Position.y + bbox.min.y;
            rect.width = bbox.max.x - bbox.min.x;
            rect.height = bbox.max.y - bbox.min.y;

            Vector2 hitPoint2D = { -10000,-100000 };
            Vector2 hitNormal2D = { 0, 0 };

            Vector2 newPos2D = { newPos.x, newPos.y };

            CollisionUtils::PointNearestRect(rect, newPos2D, &hitPoint2D, &hitNormal2D);

            Vector2 vectorToHit = hitPoint2D - newPos2D;

            bool inside = Vector2LengthSqr(vectorToHit) < radius * radius;

            if (inside)
            {
                hitSomething = true;
                // normalize the vector along the point to where we are nearest
                vectorToHit = Vector2Normalize(vectorToHit);

                // project that out to the radius to find the point that should be 'deepest' into the rectangle.
                Vector2 projectedPoint = newPos2D + (vectorToHit * radius);

                // compute the shift to take the deepest point out to the edge of our nearest hit, based on the vector direction
                Vector3 delta = { 0, 0, 0 };

                if (hitNormal2D.x != 0)
                    delta.x = hitPoint2D.x - projectedPoint.x;
                else
                    delta.y = hitPoint2D.y - projectedPoint.y;

                // shift the new point by the delta to push us outside of the rectangle
                newPos += delta;
            }
        }
        else
        {
            Vector3 hitPoint = { -10000,-100000, 0 };
            Vector3 hitNormal = { 0, 0, 0 };

            CollisionUtils::PointNearestBoundsXY(bbox, transform.Position, newPos, &hitPoint, &hitNormal);

            Vector3 vectorToHit = hitPoint - newPos;
 
            bool inside = Vector3LengthSqr(vectorToHit) < radius * radius;
 
            if (inside)
            {
                hitSomething = true;
                // normalize the vector along the point to where we are nearest
                vectorToHit = Vector3Normalize(vectorToHit);
 
                // project that out to the radius to find the point that should be 'deepest' into the rectangle.
                Vector3 projectedPoint = newPos + (vectorToHit * radius);
 
                // compute the shift to take the deepest point out to the edge of our nearest hit, based on the vector direction
                Vector3 delta = { 0,0 };
 
                if (hitNormal.x != 0)
                    delta.x = hitPoint.x - projectedPoint.x;
                else
                    delta.y = hitPoint.y - projectedPoint.y;
 
                // shift the new point by the delta to push us outside of the rectangle
                newPos += delta;
            }
        }
    }

    desiredMotion = newPos - position;

    return hitSomething;
}

