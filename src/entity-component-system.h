#pragma once

#include <cstdint>
#include <bitset>
#include <queue>
#include <cassert>

#ifdef _WIN32
#define ECS_EXPORT __declspec(dllexport)
#else
#define ECS_EXPORT
#endif

ECS_EXPORT using Entity = std::uint32_t;
ECS_EXPORT const Entity MAX_ENTITIES = 5000;

ECS_EXPORT using ComponentType = std::uint8_t;
ECS_EXPORT const ComponentType MAX_COMPONENTS = 32;

ECS_EXPORT using Signature = std::bitset<MAX_COMPONENTS>;

class ECS_EXPORT EntityManager
{
private:
    std::queue<Entity> mAvailableEntities{};
    std::array<Signature, MAX_ENTITIES> mSignatures{};
    std::uint32_t mLivingEntityCount{};

public:
    EntityManager()
    {
        for (Entity entity = 0; entity < MAX_ENTITIES; ++entity)
        {
            mAvailableEntities.push(entity);
        }
    }

    Entity CreateEntity()
    {
        assert(mLivingEntityCount < MAX_ENTITIES && "Too many entities in existence.");

        Entity entity = mAvailableEntities.front();
        mAvailableEntities.pop();
        ++mLivingEntityCount;

        return entity;
    }

    void DestroyEntity(Entity entity)
    {
        assert(entity < MAX_ENTITIES && "Entity out of range.");

        // Invalidate the destroyed entity's signature
        mSignatures[entity].reset();

        mAvailableEntities.push(entity);
        --mLivingEntityCount;
    }

    void SetSignature(Entity entity, Signature signature)
    {
        assert(entity < MAX_ENTITIES && "Entity is out of range.");

        mSignatures[entity] = signature;
    }

    Signature GetSignature(Entity entity)
    {
        assert(entity < MAX_ENTITIES && "Entity is out of range.");

        return mSignatures[entity];
    }
};
