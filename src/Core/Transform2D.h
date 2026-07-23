#ifndef TRANSFORM2D_H_DEFINED
#define TRANSFORM2D_H_DEFINED

#include "define.h"

#include "Math/Matrix/Matrix.h"
#include "Math/Vector/Vector.h"
#include "Math/Quaternions/Quaternion.h"

class Transform2D
{
public:
    enum LocalDirty : uint8
    {
        None            = 0,
        Position        = 1 << 0,
        RotationScale   = 1 << 1,
        World           = Position | RotationScale
    };
    
    Transform2D();
    ~Transform2D() = default;

    Transform2D(const Transform2D& other);
    Transform2D(Transform2D&& other) noexcept;
    Transform2D& operator=(const Transform2D& other);
    Transform2D& operator=(Transform2D&& other) noexcept;

    void SetIdentity();
    void UpdateMatrix();
    bool IsWorldDirty();
    Mat4f32 const& GetMatrix();
    
    void UpdateFromParent(Transform2D const& parent);
    
    void AddFlag(uint8 _flag);
    void RemoveFlag(uint8 _flag);
    bool GetDirtyState(uint8 _flag) const;
    
    //////////////////// Pos //////////////////////////

    Vect2f32 const& GetPosition();
    
    void SetPosition(Vect2f32 const& _position);
    void Move(Vect2f32 const& _delta);
    void Move(Vect2f32 const& _dir, float _distance);

    ////////////////////// Scale //////////////////////
    
    Vect2f32 const& GetScale();
    
    void SetScale(Vect2f32 const& _scale);
    void SetScale(float _scale);
    void Scale(Vect2f32 const& _scale);
    void Scale(float _scale);
    
    ///////////////////// Rotation /////////////////////

    float GetRotation() const;
    Vect2f32 GetRight()   const;
    Vect2f32 GetUp()      const;

    void SetRotation(float _angle);
    void Rotate(float _delta);
    void ResetRotation();
    
private:
    Vect2f32   m_pos   = { 0.0f, 0.0f };
    Vect2f32   m_scale = { 1.0f, 1.0f };
    float      m_angle = 0.0f;
    
    Mat4f32 m_matrix;

    uint32 m_dirty = 0;
};

#endif
