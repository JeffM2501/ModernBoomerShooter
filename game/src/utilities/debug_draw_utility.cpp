#include "utilities/debug_draw_utility.h"

#include "component.h"

#include "services/global_vars.h"

#include <vector>
#include <unordered_map>

namespace DebugDrawUtility
{
#if defined(_DEBUG)
    struct DebugDrawItem
    {
        size_t ComponentType = 0;
        DebugDraw* DebugItem;
    };

    static std::vector<DebugDrawItem> DebugItems;

    static std::unordered_map<size_t, bool> DebugItemStates;
#endif

    DebugDraw::DebugDraw(Component* owner)
    {
#if defined(_DEBUG)

        DebugItems.emplace_back(DebugDrawItem{ owner->GetTypeID(), this });

        if (!DebugItemStates.contains(owner->GetTypeID()))
            DebugItemStates.insert_or_assign(owner->GetTypeID(), true);
#endif
    }

    DebugDraw::~DebugDraw()
    {
#if defined(_DEBUG)

        for (auto itr = DebugItems.begin(); itr != DebugItems.end(); itr++)
        {
            if (itr->DebugItem == this)
            {
                DebugItems.erase(itr);
                break;
            }
        }
#endif
    }

    void DebugDraw::SetDrawFunctions(std::function<void(const Camera3D&)> draw3d, std::function<void()> draw2d)
    {
#if defined(_DEBUG)
        Draw3D = draw3d;
        Draw2D = draw2d;
#endif
    }


    void Draw2D()
    {
        if (!GlobalVars::ShowDebugDraw)
            return;

#if defined(_DEBUG)
        for (auto& item : DebugItems)
        {
            if (IsComponentEnabled(item.ComponentType) && item.DebugItem->Draw2D)
                item.DebugItem->Draw2D();
        }
#endif
    }
    void Draw3D(const Camera3D& camera)
    {
        if (!GlobalVars::ShowDebugDraw)
            return;

#if defined(_DEBUG)
        for (auto& item : DebugItems)
        {
            if (IsComponentEnabled(item.ComponentType) && item.DebugItem->Draw3D)
                item.DebugItem->Draw3D(camera);
        }
#endif
    }

    void Cleanup()
    {
#if defined(_DEBUG)
        DebugItems.clear();
#endif
    }

    void DisableComponent(size_t componentHash)
    {
#if defined(_DEBUG)
        DebugItemStates.insert_or_assign(componentHash, false);
#endif
    }

    void EnableComponent(size_t componentHash)
    {
#if defined(_DEBUG)
        DebugItemStates.insert_or_assign(componentHash, true);
#endif
    }

    bool IsComponentEnabled(size_t componentHash)
    {
#if defined(_DEBUG)
        auto itr = DebugItemStates.find(componentHash);
        return itr == DebugItemStates.end() || itr->second;
#endif
        return false;
    }

}