#pragma once

#include <memory>

class ObjectLifetimeToken
{
private:
    void* Owner = nullptr;

public:
    ObjectLifetimeToken(void* owner) : Owner(owner) {}

    using Ptr = std::shared_ptr<ObjectLifetimeToken>;
    static Ptr Create(void* owner)
    {
        return std::make_shared<ObjectLifetimeToken>(owner);
    }

    bool IsValid() const { return Owner != nullptr; }

    void Invalidate() { Owner = nullptr; }

    void* GetOwner() { return Owner; }

    template<class T>
    T* GetOwner() { return static_cast<T*>(Owner); }
};

