#include "systems/map_object_system.h"

#include "world.h"
#include "systems/scene_render_system.h"
#include "components/map_object_component.h"
#include "components/transform_component.h"
#include "components/trigger_component.h"
#include "components/door_controller_component.h"
#include "services/model_manager.h"
#include "services/global_vars.h"

#include "utilities/collision_utils.h"

#include "raymath.h"


void MapObjectSystem::OnSetup()
{
    SceneRenderer = WorldPtr->GetSystem<SceneRenderSystem>();

    Audio = WorldPtr->GetSystem<AudioSystem>();

    OpenDoorSound = Audio->GetSound("door_open");
    CloseDoorSound = Audio->GetSound("door_close");

    WorldPtr->AddEventHandler(DoorControllerComponent::DoorOpening, [this](size_t, GameObject*, GameObject*)
        { 
            if (OpenDoorSound)
                OpenDoorSound->Play();
        }, Token);

    WorldPtr->AddEventHandler(DoorControllerComponent::DoorClosing, [this](size_t, GameObject*, GameObject*)
        { 
            if (CloseDoorSound)
                CloseDoorSound->Play();
        }, Token);

}

void MapObjectSystem::OnUpdate()
{
    for (auto* door : Doors.Components)
        door->Update();
}

void MapObjectSystem::OnAddObject(GameObject* object)
{
    auto* mapObject = MapObjects.Add(object);

    if (mapObject)
    {
        mapObject->Instance = ModelManager::GetModel(mapObject->ModelName);

        if (SceneRenderer)
            SceneRenderer->MapObjectAdded(mapObject);
    }

    Triggers.Add(object);
    Doors.Add(object);
}

template<class T>
bool FindAndErase(GameObject* object, std::vector<T*>& container)
{
    T* comp = object->GetComponent<T>();
    if (comp)
    {
        auto itr = std::find(container.begin(), container.end(), comp);
        if (itr != container.end())
        {
            container.erase(itr);
            return true;
        }
    };

    return false;
}

void MapObjectSystem::OnRemoveObject(GameObject* object)
{
    MapObjects.Remove(object);
    Triggers.Remove(object);
    Doors.Remove(object);
}

void MapObjectSystem::CheckTriggers(GameObject* entity, float radius, bool hitSomething)
{
    if (!entity)
        return;

    auto* transform = entity->GetComponent<TransformComponent>();
    if (!transform)
        return;

    Vector2 pos = { transform->Position.x, transform->Position.y };

    for (auto& trigger : Triggers.Components)
    {
        if (CheckCollisionCircleRec(pos, radius, trigger->Bounds))
        {
            if (!entity->HasFlag(trigger))
            {
                entity->AddFlag(trigger);
                trigger->AddObject(entity->GetToken());
                trigger->GetOwner()->CallEvent(TriggerComponent::TriggerEnter, entity);
            }
        }
        else if (entity->HasFlag(trigger))
        {
            trigger->RemovObject(entity->GetToken());
            entity->ClearFlag(trigger);
            trigger->GetOwner()->CallEvent(TriggerComponent::TriggerExit, entity);
        }
    }
}

bool MapObjectSystem::MoveEntity(Vector3& position, Vector3& desiredMotion, float radius, GameObject* entity)
{
    bool hitSomething = false;

    if (Vector3Equals(desiredMotion, Vector3Zeros))
        return false;

    Vector3 newPos = position + desiredMotion;

    for (auto* object : MapObjects.Components)
    {
        if (!object->Solid)
            continue;

        float motionVecLen = Vector3Length(desiredMotion);
        if (motionVecLen < FLT_MIN)
            break;

        BoundingBox bbox = object->Instance->Geometry->GetBounds();

        Vector3 targetVec = desiredMotion / motionVecLen;

        TransformComponent& transform = object->GetOwner()->MustGetComponent<TransformComponent>();

        // check bounds

        if (false)
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

