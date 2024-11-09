#pragma once

#include <memory>

class GameObject;

#define DEFINE_COMPONENT(T) \
    T(GameObject* owner) : Owner(owner){} \
    static size_t TypeID() { static std::hash<std::string_view> hasher; return hasher(#T);} \
    size_t GetTypeID() const override {return T::TypeID();} \
    static std::unique_ptr<Component> Create(GameObject* owner){ return std::make_unique<T>(owner);}

class Component
{
public:
    virtual ~Component() = default;

    virtual size_t GetTypeID() const = 0;

    template <class T>
    inline bool Is()
    {
        return GetTypeID() == T::TypeID();
    }

    template <class T>
    inline T* As()
    {
        if (GetTypeID() != T::TypeID())
            return nullptr;

        return reinterpret_cast<T*>(this);
    }

    void AddToSystem(size_t systemGUID);

    template<class T>
    inline void AddToSystem()
    {
        AddToSystem(T::GUID());
    }

protected:
    GameObject* Owner;
};