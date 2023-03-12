#pragma once
#include <array>

namespace SE {

    /**
     * An Entity Component that has a Spatial context.
     * AKA, a Component with a position, a rotation, and a size.
     *
     * Spatial Components may exist in a hierarchy; where each subsequent object inherits the spatial positions of its' parents.
     * This allows for things like complex articulation of large machines, etc.
     */
    class EntitySpatialComponent : public EntityComponent {
    public:

        // Whether this is the root spatial component for this Entity.
        inline bool IsSpatialRoot() const { return parent == nullptr; }

        // Fetch the transforms and bounds.
        inline Math::Transform const& GetLocalTransform() const { return transform; }
        inline Math::OrientedBB const& GetLocalBounds() const { return bounds; }

        inline Math::Transform const& GetWorldTransform() const { return worldTransform; }
        inline Math::OrientedBB const& GetWorldBounds() const { return worldBounds; }

        // Get the position in world-space.
        inline Math::Vector const& GetPosition() const { return worldTransform.GetTranslation(); }

        // Get the rotation in world-space.
        inline Math::Quaternion const& GetRotation() const { return worldTransform.GetRotation(); }

        // Get vectors relative to this spatial component.
        inline Math::Vector GetForwardVector() const { return worldTransform.GetForwardVector(); }
        inline Math::Vector GetUpVector() const { return worldTransform.GetUpVector(); }
        inline Math::Vector GetRightVector() const { return worldTransform.GetRightVector(); }

        // Update the local and world transforms
        inline void SetLocalTransform(Math::Transform const& newTransform);
        inline void SetWorldTransform(Math::Transform const& newTransform);

        // Move this component by the specified amount of transform, rotation
        inline void MoveBy(Math::Transform const& delta);


        // Is there a child spatial component
        inline bool HasChildren() const { return !children.empty(); }

        // Get the ID of the parent of this Spatial Component
        inline ComponentID const& GetParentID() const { return parent->GetID(); }

        // Get the transform of the parent component
        inline Math::Transform const& GetParentTransform() const { return parent->worldTransform; }

        // Fetch how deep in the spatial hierarchy this Component is.
        size_t GetHierarchyDepth(bool singleEntity = true) const;

        // Check if we are a child of a given Component
        bool IsChildOf(EntitySpatialComponent const* parent) const;

        // Get the ID of the socket we are attached to
        inline std::string const& GetAttachmentSocketID() const { return attachmentSocket; }

        // Set the name of the socket we wish to be attached to.
        inline void SetAttachmentSocket(std::string& id) { attachmentSocket = id;}

        // Fetch the transform of a specific attachment.
        Math::Transform const& GetAttachmentTransform(std::string& id) const;

        // Whether this Component supports local scaling
        virtual bool LocalScale() const { return false; }
        // Fetch the local scale, if supported. { 1, 1, 1 } otherwise.
        virtual std::array<float, 3> const& GetLocalScale() const { static auto ones = std::array<float, 3> { 1, 1, 1 }; return ones; }

        // Convert a given world-space transform to a model-space transform.
        inline Math::Transform const& ConvertWorldToLocalTransform(Math::Transform const& world);


    protected:

        virtual void Initialize() override;

        // Set the local scale, if supported
        virtual void SetLocalScale(std::array<float, 3> const& newScale) {}

        // Calculate the local bounding box for the Component; the position should always be close to origin.
        virtual Math::OrientedBB CalculateBounds() const;

        // Update the local and world bounds for the Component.
        void UpdateBounds();

        // Attempt to find the given Socket in this Component, and return its' Transform if present.
        bool GetAttachmentTransform(std::string& socketId, Math::Transform& out) const;
        virtual bool FindAttachmentTransform(std::string& socketId, Math::Transform& out) const;

        // Whether this Component has the given Socket.
        // Default: No sockets.
        virtual bool HasSocket(std::string& socket) const { return false;}

        // Process the creation/destruction/movement of a Socket.
        void UpdateSockets();

        // Called whenever the World Transform is updated.
        virtual void OnMoved() {}

        // Set the World Transform for a component.
        // Skips a lot of internal processing.
        inline void SetWorldTransform(Math::Transform& newTransform, bool callback = true);

    private:

        inline void CalculateWorldTransform(bool callback = true);

        // The location of this Component, relative to the world.
        Math::Transform transform;
        // The collision boundary of this Component.
        Math::OrientedBB bounds;

        // The world transform of this component; how to get to the position from the world origin.
        Math::Transform worldTransform;
        Math::OrientedBB worldBounds;

        // The parent of this Spatial Component
        EntitySpatialComponent* parent = nullptr;
        // The socket (attachment point) of the parent entity that this Component is attached to.
        std::string attachmentSocket;
        // The Components that are attached to this Component.
        std::vector<EntitySpatialComponent*> children;

    };
}