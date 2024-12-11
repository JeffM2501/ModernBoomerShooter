#pragma once

#include <functional>
#include <string_view>

#include "raylib.h"

class Component;

namespace DebugDrawUtility
{
    class DebugDraw
    {
    public:
        DebugDraw(Component* owner);
        ~DebugDraw();

        DebugDraw(const DebugDraw&) = delete;
        DebugDraw* operator = (const DebugDraw&) = delete;

        void SetDrawFunctions(std::function<void(const Camera3D&)> draw3d, std::function<void()> draw2d = nullptr);

#if defined(_DEBUG)
        std::function<void()> Draw2D = nullptr;
        std::function<void(const Camera3D&)> Draw3D = nullptr;
#endif
    };

    void Draw2D();
    void Draw3D(const Camera3D& camera);

    void Cleanup();

    void DisableComponent(size_t componentHash);
    template<class T>
    inline void DisableComponent() { DisableComponent(T::GetTypeID()); }

    void EnableComponent(size_t componentHash);
    template<class T>
    inline void EnableComponent() { EnableComponent(T::GetTypeID()); }

    bool IsComponentEnabled(size_t componentHash);
    template<class T>
    inline bool IsComponentEnabled() { IsComponentEnabled(T::GetTypeID()); }
}