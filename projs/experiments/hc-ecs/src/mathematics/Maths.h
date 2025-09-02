/**********************/
/**    SONIC ECS     **/
/**  GOTTA GO FAST.  **/
/**********************/

#include <bits/valarray_after.h>

#include "DirectXMath.h"
using namespace DirectX;

#define X(v) XMVectorGetX(v)

namespace sonic {
    namespace maths {

        /**
         * Encodes a rotation, translation and scale.
         * A wrapper around a Quaternion and a 4-dimensional Vector.
         * Inlines most operations, providing nothing more than a useful interface for the underlying DXMath classes.
         */
        struct Transformation {

            static Transformation const Identity;
            static XMVECTOR const Xv0001;

            Transformation() = default;

            explicit inline Transformation(XMMATRIX const& matrix) {
                XMVECTOR translate, scale;
                XMMatrixDecompose(&scale, &Rotation, &translate, matrix);
                TranslationScale = XMVectorSelect(translate, scale, Xv0001);
            }

            explicit inline Transformation(XMVECTOR const& rot, XMFLOAT3 const& translate = XMFLOAT3(0,0,0), float scale = 1.0f) : Rotation(rot), TranslationScale(XMVectorSet(translate.x, translate.y, translate.z, scale)) {}

            inline static Transformation FromRotation(XMVECTOR const& rotation) {
                return Transformation(rotation);
            }

            inline static Transformation FromTranslation(XMFLOAT3 const& translate) {
                return Transformation(XMQuaternionIdentity(), translate);
            }

            inline static Transformation FromScale(float scale) {
                return Transformation(XMQuaternionIdentity(), XMFLOAT3(0,0,0), scale);
            }

            inline static Transformation FromTranslateScale(XMFLOAT3 const& translate, float scale) {
                return Transformation(XMQuaternionIdentity(), translate, scale);
            }

            inline static Transformation FromTwoVectors(XMVECTOR const& start, XMVECTOR const& end) {
                XMVECTOR p = XMVector3Cross(start, end);
                XMVECTOR w = XMVectorSet(0,0,0,
                    std::sqrt( X(XMQuaternionLengthSq(start)) * X(XMQuaternionLengthSq(end))) + X(XMVector3Dot(start, end))
                );
                return Transformation(XMVectorSelect(p, w, Xv0001));
            }

            inline static Transformation Delta(Transformation const& from, Transformation const& to);
            inline static Transformation DeltaNoScale(Transformation const& from, Transformation const& to);

            inline XMMATRIX ToMatrix() const {
                XMFLOAT4 tValues(0,0,0,0);
                XMStoreFloat4(&tValues, TranslationScale);
                return XMMatrixTransformation(
                    XMVectorZero(),
                    XMVectorZero(),
                    XMVectorSet(tValues.w, tValues.w, tValues.w, tValues.w),
                    XMVectorZero(),
                    Rotation,
                    XMVectorSelect(TranslationScale, Xv0001, Xv0001)
                );
            }

            inline XMMATRIX ToMatrixWithoutScale() const {
                return XMMatrixTransformation(
                    XMVectorZero(),
                    XMVectorZero(),
                    XMVectorSet(1, 1, 1, 1),
                    XMVectorZero(),
                    Rotation,
                    XMVectorSelect(TranslationScale, Xv0001, Xv0001)
                );
            }

            inline XMVECTOR GetAxisX() const {
                return (XMVectorSet(1, 0,0, 0) * XMQuaternionConjugate(Rotation)) * Rotation;
            }
            inline XMVECTOR GetAxisY() const {
                return (XMVectorSet(0, 1,0, 0) * XMQuaternionConjugate(Rotation)) * Rotation;
            }
            inline XMVECTOR GetAxisZ() const {
                return (XMVectorSet(0, 0,1, 0) * XMQuaternionConjugate(Rotation)) * Rotation;
            }

            inline XMVECTOR GetRight() const {
                return (XMVectorSet(-1, 0,0, 0) * XMQuaternionConjugate(Rotation)) * Rotation;
            }

            inline XMVECTOR GetUp() const {
                return (XMVectorSet(0, 0,1, 0) * XMQuaternionConjugate(Rotation)) * Rotation;
            }

            inline XMVECTOR GetForward() const {
                return (XMVectorSet(0, -1,0, 0) * XMQuaternionConjugate(Rotation)) * Rotation;
            }

            inline bool IsIdentity() const { return XMQuaternionIsIdentity(Rotation) && XMVector4Equal(TranslationScale, XMVectorSet(0,0,0,1)); }

            inline Transformation& Inverse();

            inline Transformation GetInverse() const;

            inline Transformation DeltaToOther(Transformation const& target) const { return Transformation::Delta(*this, target); }

            inline Transformation DeltaFromOther(Transformation const& target) const { return Transformation::Delta(target, *this); }


            inline XMVECTOR const& GetRotation() const { return Rotation; }

            inline void SetRotation(XMVECTOR const& rotation) { Rotation = rotation; }

            inline void AddRotation(XMVECTOR const& delta) { Rotation = XMQuaternionMultiply(delta, Rotation); }

            inline XMVECTOR const& GetTranslationScale() const { return TranslationScale; }

            inline void SetTranslation(XMVECTOR const& newTranslate) { TranslationScale = XMVectorSelect(newTranslate, TranslationScale, Xv0001); }

            inline void SetTranslationScale(XMVECTOR const& newTranslateScale) { TranslationScale = newTranslateScale; }

            inline void AddTranslation(XMVECTOR const& delta) { TranslationScale = XMVectorAdd(TranslationScale, delta); }

            inline float GetScale() const { return XMVectorGetW(TranslationScale); }

            inline XMVECTOR GetScaleVector() const { return XMVectorSplatW(TranslationScale); }

            inline XMVECTOR GetScaleVectorInverse() const { return XMVectorMultiply(XMVectorSplatW(TranslationScale), XMVectorSet(-1, -1, -1, -1)); }

            inline void SetScale(float scale) { TranslationScale = XMVectorSetW(TranslationScale, scale); }

            inline bool HasScale() const { return XMVectorGetW(TranslationScale) != 1.0f; }

            inline bool HasNegativeScale() const { return XMVectorGetW(TranslationScale) < 0.0f; }

            inline XMVECTOR Translate(XMVECTOR const& delta) const { return XMVectorAdd(TranslationScale, delta); }

            inline XMVECTOR Scale(XMVECTOR const& vec) const { return XMVectorMultiply(vec, GetScaleVector()); }

            inline XMVECTOR TransformPoint(XMVECTOR const& vec) const;

            inline XMVECTOR TransformPointNoScale(XMVECTOR const& vec) const;

            inline XMVECTOR Rotate(XMVECTOR const& vec) const { return XMVector3Rotate(vec, Rotation); }

            inline XMVECTOR InverseRotate(XMVECTOR const& vec) const { return XMVector3InverseRotate(vec, Rotation); }

            inline XMVECTOR InverseTransformPoint(XMVECTOR const& vec) const;

            inline XMVECTOR InverseTransformPointNoScale(XMVECTOR const& vec) const;

            inline XMVECTOR Transform(XMVECTOR const& vec) const;

            inline XMVECTOR InverseTransform(XMVECTOR const& vec) const;

            inline XMVECTOR InverseTransformNoScale(XMVECTOR const& vec) const;

            inline Transformation operator*(Transformation const& other) const;

            inline Transformation operator*=(Transformation const& other) const;

            inline bool operator==(Transformation const& other) const {
                if (!XMVector4Equal(TranslationScale, other.TranslationScale)) return false;
                if (!XMQuaternionEqual(Rotation, other.Rotation)) return false;
                return true;
            }

            inline bool operator!=(Transformation const& other) const { return !operator==(other); }

        private:
            XMVECTOR Rotation = XMQuaternionIdentity();
            XMVECTOR TranslationScale = XMVectorZero();
        };

        /**
         * Axis-Aligned Bounding Box.
         * The straight lines are always aligned with the world x-y-z grid.
         */
        struct AABB {

            XMVECTOR Center = XMVectorSplatOne();
            XMVECTOR HalfExtents = XMVectorNegate(XMVectorSplatOne());
        };

        /**
         * Oriented Bounding Box.
         * The straight lines are in arbitrary axes.
         */
        struct OBB {

            void ApplyTransformation(Transformation const& transform);

            inline OBB GetTransformed(Transformation const& transform) const {
                OBB result = *this;
                result.ApplyTransformation(transform);
                return result;
            }

            XMVECTOR Center = XMVectorSplatOne();
            XMVECTOR HalfExtents = XMVectorNegate(XMVectorSplatOne());
            XMVECTOR Rotation = XMQuaternionIdentity();
        };
    }
}
