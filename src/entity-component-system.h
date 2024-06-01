#pragma once

#include <cstdint>
#include <bitset>
#include <queue>
#include <cassert>
#include <set>

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
    // Queue of unused entity IDs
    std::queue<Entity> mAvailableEntities{};
    // Array of signatures where the index corresponds to the entity ID
    std::array<Signature, MAX_ENTITIES> mSignatures{};
    // Total living entities - used to keep limits on how many exist
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

class ECS_EXPORT IComponentArray
{
public:
    virtual ~IComponentArray() = default;
    virtual void EntityDestroyed(Entity entity) = 0;
};

template <typename T>
class ECS_EXPORT ComponentArray : public IComponentArray
{
private:
    // The dense array of components, it is also called packed array
    // because it is a contiguous block of memory
    // meaning that the components are stored next to each other in memory
    // and there are no gaps between them.
    std::array<T, MAX_ENTITIES> mComponentArray;
    // Map from an entity ID to an array index.
    std::unordered_map<Entity, std::size_t> mEntityToIndexMap;
    // Map from an array index to an entity ID.
    std::unordered_map<std::size_t, Entity> mIndexToEntityMap;

    // Total size of valid entries in the array.
    std::size_t mSize{};

public:
    /**
     * @brief Inserts the component at the end of the array, and updates the maps.
     *
     * @param entity The entity ID to associate with the component.
     * @param component The component to add to the array.
     *
     * @note This method will assert if the entity already has a component in the array.
     */
    void InsertData(Entity entity, T component)
    {
        assert(mEntityToIndexMap.find(entity) == mEntityToIndexMap.end() && "Component added to same entity more than once.");

        std::size_t newIndex = mSize;
        mEntityToIndexMap[entity] = newIndex;
        mIndexToEntityMap[newIndex] = entity;
        ++mSize;
    }

    /**
     * @brief Removes a component from the array, and updates the maps.
     *
     * @param entity The entity ID to remove from the array.
     *
     * The remove process will swap the last element with the element to remove,
     * and update the maps to point to the new location of the moved element.
     *
     * @note This method will assert if the entity does not have a component in the array.
     */
    void RemoveData(Entity entity)
    {
        assert(mEntityToIndexMap.find(entity) != mEntityToIndexMap.end() && "Removing non-existent component.");

        // First find the index of the component that will be removed
        std::size_t indexOfRemovedEntity = mEntityToIndexMap[entity];
        std::size_t indexOfLastElement = mSize - 1;
        // Now swap the last element with the element to remove
        mComponentArray[indexOfRemovedEntity] = mComponentArray[indexOfLastElement];

        // Next update the maps to point to the new locations
        Entity entityOfLastElement = mIndexToEntityMap[indexOfLastElement];
        mEntityToIndexMap[entityOfLastElement] = indexOfRemovedEntity;
        mIndexToEntityMap[indexOfRemovedEntity] = entityOfLastElement;

        // Finally remove the entity, remove the index map, and reduce the size
        mEntityToIndexMap.erase(entity);
        mIndexToEntityMap.erase(indexOfLastElement);
        --mSize;
    }

    T &GetData(Entity entity)
    {
        assert(mEntityToIndexMap.find(entity) != mEntityToIndexMap.end() && "Retrieving non-existent component.");
        std::size_t indexOfEntity = mEntityToIndexMap[entity];

        return mComponentArray[indexOfEntity];
    }

    void EntityDestroyed(Entity entity) override
    {
        if (mEntityToIndexMap.find(entity) != mEntityToIndexMap.end())
        {
            RemoveData(entity);
        }
    }
};

class ECS_EXPORT ComponentManager
{
private:
    std::unordered_map<const char *, ComponentType> mComponentTypes{};
    std::unordered_map<const char *, std::shared_ptr<IComponentArray>> mComponentArrays{};
    ComponentType mNextComponentType{};

    template <typename T>
    std::shared_ptr<ComponentArray<T>> GetComponentArray()
    {
        const char *typeName = typeid(T).name();

        assert(mComponentTypes.find(typeName) != mComponentTypes.end() && "Component not registered before use.");

        return std::static_pointer_cast<ComponentArray<T>>(mComponentArrays[typeName]);
    }

public:
    template <typename T>
    void RegisterComponent()
    {
        const char *typeName = typeid(T).name();

        assert(mComponentTypes.find(typeName) == mComponentTypes.end() && "Registering component type more than once.");

        // Add this component type to the component type map
        mComponentTypes.insert({typeName, mNextComponentType});

        // Create a ComponentArray pointer and add it to the component arrays map
        mComponentArrays.insert({typeName, std::make_shared<ComponentArray<T>>()});

        // Increment the value so that the next component registered will be different
        ++mNextComponentType;
    }

    template <typename T>
    ComponentType GetComponentType()
    {
        const char *typeName = typeid(T).name();

        assert(mComponentTypes.find(typeName) != mComponentTypes.end() && "Component not registered before use.");

        return mComponentTypes[typeName];
    }

    template <typename T>
    void AddComponent(Entity entity, T component)
    {
        GetComponentArray<T>()->InsertData(entity, component);
    }

    template <typename T>
    void RemoveComponent(Entity entity)
    {
        GetComponentArray<T>()->RemoveData(entity);
    }

    template <typename T>
    T &GetComponent(Entity entity)
    {
        return GetComponentArray<T>()->GetData(entity);
    }

    void EntityDestroyed(Entity entity)
    {
        for (auto const &pair : mComponentArrays)
        {
            auto const &componentArray = pair.second;

            componentArray->EntityDestroyed(entity);
        }
    }
};

class ECS_EXPORT System
{
public:
    std::set<Entity> mEntities{};
};

class ECS_EXPORT SystemManager
{
private:
    std::unordered_map<const char *, std::shared_ptr<System>> mSystems{};
    std::unordered_map<const char *, Signature> mSignatures{};

public:
    template <typename T>
    std::shared_ptr<T> RegisterSystem()
    {
        const char *typeName = typeid(T).name();

        assert(mSystems.find(typeName) == mSystems.end() && "Registering system more than once.");

        auto system = std::make_shared<T>();
        mSystems.insert({typeName, system});

        return system;
    }

    template <typename T>
    void SetSignature(Signature signature)
    {
        const char *typeName = typeid(T).name();

        assert(mSystems.find(typeName) != mSystems.end() && "System used before registered.");

        mSignatures.insert({typeName, signature});
    }

    void EntityDestroyed(Entity entity)
    {
        for (auto pair : mSystems)
        {
            auto const &system = pair.second;

            system->mEntities.erase(entity);
        }
    }

    void EntitySignatureChanged(Entity entity, Signature newSignature)
    {
        for (auto pair : mSystems)
        {
            auto const &type = pair.first;
            auto const &system = pair.second;
            auto const &systemSignature = mSignatures[type];

            if ((newSignature & systemSignature) == systemSignature)
            {
                system->mEntities.insert(entity);
            }
            else
            {
                system->mEntities.erase(entity);
            }
        }
    }
};

class Coordinator
{
private:
    std::unique_ptr<ComponentManager> mComponentManager;
    std::unique_ptr<EntityManager> mEntityManager;
    std::unique_ptr<SystemManager> mSystemManager;

public:
    void Init()
    {
        // Create pointers to each manager
        mComponentManager = std::make_unique<ComponentManager>();
        mEntityManager = std::make_unique<EntityManager>();
        mSystemManager = std::make_unique<SystemManager>();
    }

    // Entity methods
    Entity CreateEntity()
    {
        return mEntityManager->CreateEntity();
    }

    void DestroyEntity(Entity entity)
    {
        mEntityManager->DestroyEntity(entity);

        mComponentManager->EntityDestroyed(entity);

        mSystemManager->EntityDestroyed(entity);
    }

    // Component methods
    template <typename T>
    void RegisterComponent()
    {
        mComponentManager->RegisterComponent<T>();
    }

    template <typename T>
    void AddComponent(Entity entity, T component)
    {
        mComponentManager->AddComponent<T>(entity, component);

        auto signature = mEntityManager->GetSignature(entity);
        signature.set(mComponentManager->GetComponentType<T>(), true);
        mEntityManager->SetSignature(entity, signature);

        mSystemManager->EntitySignatureChanged(entity, signature);
    }

    template <typename T>
    void RemoveComponent(Entity entity)
    {
        mComponentManager->RemoveComponent<T>(entity);

        auto signature = mEntityManager->GetSignature(entity);
        signature.set(mComponentManager->GetComponentType<T>(), false);
        mEntityManager->SetSignature(entity, signature);

        mSystemManager->EntitySignatureChanged(entity, signature);
    }

    template <typename T>
    T &GetComponent(Entity entity)
    {
        return mComponentManager->GetComponent<T>(entity);
    }

    template <typename T>
    ComponentType GetComponentType()
    {
        return mComponentManager->GetComponentType<T>();
    }

    // System methods
    template <typename T>
    std::shared_ptr<T> RegisterSystem()
    {
        return mSystemManager->RegisterSystem<T>();
    }

    template <typename T>
    void SetSystemSignature(Signature signature)
    {
        mSystemManager->SetSignature<T>(signature);
    }
};