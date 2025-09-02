/**********************/
/**    SONIC ECS     **/
/**  GOTTA GO FAST.  **/
/**********************/

#pragma once
#include "Component.h"
#include <mathematics/Maths.h>


namespace sonic {

    /**
     * A Spatial Component is an ECS-innate marker that tells the system that this Entity has a physical placement in the Scene.
     * It is one of only two Components provided by the ECS itself.
     * Spatial means that it has a location and a size.
     * An Entity with a Spatial Component that has a parent Entity itself with a Spatial Component will have a hierarchy applied:
     *  The overall World transform of the Entity that this Component belongs to is the result of multiplying all Transformations of the Entity's parents until the top of the hierarchy is reached.
     * Spatial Entities can have dedicated attachment points, allowing for joints and motors and hinges, etc.
     * Any Spatial Component can represent an attachment to a socket, a loose (highest-tier) Entity, or any level of node in its' applicable tree.
     *
     */
    class SpatialComponent : public Component {
    public:
        inline bool IsRootComponent() const { return Parent == nullptr || Parent->Entity != Entity; }

        inline maths::Transformation const& GetLocalTransform() const { return Transform;};
        inline maths::OBB const& GetBoundingBox() const { return BoundingBox; }
        inline maths::Transformation const& GetSceneTransform() const { return SceneTransform; }
        inline maths::OBB const& GetSceneBoundingBox() const { return SceneBoundingBox; }

        XMVECTOR const& GetPosition() const { return SceneTransform.GetTranslationScale(); };
        XMVECTOR const& GetOrientation() const { return SceneTransform.GetRotation(); };
        XMVECTOR GetForwardVector() const { return SceneTransform.GetForward(); };
        XMVECTOR GetUpVector() const { return SceneTransform.GetUp(); };
        XMVECTOR GetRightVector() const { return SceneTransform.GetRight(); };

        inline void SetLocalTransform(maths::Transformation const& newTransform) {
            this->Transform = newTransform;
            UpdateSceneTransform();
        }

        inline void MoveByDelta(maths::Transformation const& delta) {
            maths::Transformation const newTransform = delta * GetSceneTransform();
            SetSceneTransform(newTransform);
        }

        inline bool HasParent() const { return Parent != nullptr; }

        inline bool HasChildren() const { return !Children.empty(); }

        inline ComponentID GetParentID() const { return Parent->GetID(); }

        inline maths::Transformation const& GetParentTransform() const { return Parent->Transform; }

        int32_t GetHierarchyDepth(bool includeSiblings = false) const;

        bool IsChildOf(SpatialComponent const* parent) const;

        inline StringID GetAttachmentSocketID() const { return ParentAttachmentSocket; }

        inline void SetAttachmentSocketID(StringID socket) { ParentAttachmentSocket = socket; }

        maths::Transformation GetAttachmentSocketTransformation(StringID socket) const;

        void ApplyOffsetToChildren(XMVECTOR const& offset);

        virtual bool SupportsLocalScale() const { return false; }

        virtual XMFLOAT3 const& GetLocalScale() const { return XMFLOAT3(1, 1, 1); }

        inline maths::Transformation ConvertSceneToLocalTransformation(maths::Transformation const& scene) const { return maths::Transformation::Delta(SceneTransform, scene); }

        inline XMVECTOR ConvertSceneToLocalPoint(XMVECTOR const& scene) const { return SceneTransform.GetInverse().TransformPoint(scene); }

        inline XMVECTOR ConvertSceneToLocalVector(XMVECTOR const& vec) const { return SceneTransform.GetInverse().Rotate(vec); }

        inline void SetSceneTransform(maths::Transformation newTransform, bool callback = true) {
            if (Parent != nullptr) {
                auto parent = Parent->GetAttachmentSocketTransformation(ParentAttachmentSocket);
                SceneTransform = newTransform;
                Transform = maths::Transformation::Delta(parent, SceneTransform);
            } else {
                SceneTransform = newTransform;
                Transform = newTransform;
            }

            SceneBoundingBox = BoundingBox.GetTransformed(SceneTransform);

            for (auto child : Children)
                child->UpdateSceneTransform();

            if (callback)
                OnSceneTransformUpdated();
        }
    protected:

        virtual void Initialize() override;

        virtual void OnLocalScaleChanged(XMFLOAT3 const& newScale) {}

        virtual maths::OBB CalculateLocalBounds() const {
            return maths::OBB(XMVectorZero(), XMVectorSet(0.5, 0.5, 0.5, 0.5));
        }

        void UpdateBounds() {
         // TODO
        }

        bool TryGetAttachmentSocketTransformation(StringID socket, maths::Transformation& out) const;

        virtual bool TryFindAttachmentSocketTransformation(StringID socket, maths::Transformation& out) const;

        virtual bool HasSocket(StringID socket) const { return false; }

        void NotifySocketsUpdated();

        virtual void OnSceneTransformUpdated() {}



    private:

        inline void UpdateSceneTransform(bool callback = true) {
            if (Parent != nullptr) {
                auto parent = Parent->GetAttachmentSocketTransformation(ParentAttachmentSocket);
                SceneTransform = Transform * parent;
            } else {
                SceneTransform = Transform;
            }

            SceneBoundingBox = BoundingBox.GetTransformed(SceneTransform);

            for (auto child : Children)
                child->UpdateSceneTransform(false);

            if (callback)
                OnSceneTransformUpdated();
        }

        SONIC_REFLECTION_FIELD()
        maths::Transformation                   Transform;                  // Local transformation - relative to the parent entity, or to the Entity's base coordinate if this has no parents.
        maths::OBB                              BoundingBox;                // The smallest box required to enclose this Entity.

        maths::Transformation                   SceneTransform;             // Global transformation - relative to the 0,0 coordinate in the center of the Scene.
        maths::OBB                              SceneBoundingBox;           // The smallest axis-aligned bounding box required to enclose the Entity.

        SpatialComponent*                       Parent = nullptr;           // If non-null, the Entity Spatial Component that this Component is a child of.
        SONIC_REFLECTION_FIELD()
        StringID                                ParentAttachmentSocket;     // If the Parent is non-null, the Attachment Socket that this Component is attached to.
        std::vector<SpatialComponent*>          Children;                   // The Spatial Components attached to this Entity.
    };
}
