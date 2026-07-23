
#include "Transform2D.h"

Transform2D::Transform2D()
{
    m_dirty = 0;
    SetIdentity();
}

Transform2D::Transform2D(const Transform2D& other)
    : m_pos(other.m_pos),
      m_scale(other.m_scale),
      m_angle(other.m_angle),
      m_matrix(other.m_matrix),
      m_dirty(other.m_dirty)
{
}

Transform2D::Transform2D(Transform2D&& other) noexcept
    : m_pos(other.m_pos),
      m_scale(other.m_scale),
      m_angle(other.m_angle),
      m_matrix(other.m_matrix),
      m_dirty(other.m_dirty)
{
}

Transform2D& Transform2D::operator=(const Transform2D& other)
{
    if (this == &other) return *this;
    m_pos      = other.m_pos;
    m_scale    = other.m_scale;
    m_angle    = other.m_angle;
    m_matrix   = other.m_matrix;
    m_dirty    = other.m_dirty;
    return *this;
}

Transform2D& Transform2D::operator=(Transform2D&& other) noexcept
{
    if (this == &other) return *this;
    m_pos      = other.m_pos;
    m_scale    = other.m_scale;
    m_angle    = other.m_angle;
    m_matrix   = other.m_matrix;
    m_dirty    = other.m_dirty;
    return *this;
}

void Transform2D::SetIdentity()
{
    m_pos   = Vect2f32(0.0f, 0.0f);
    m_scale = Vect2f32(1.0f, 1.0f);
    m_angle = 0.0f;
    
    m_matrix = Mat4f32::Identity();
    m_dirty = 0;
}

void Transform2D::UpdateMatrix()
{
    if (GetDirtyState(RotationScale))
    {
        m_matrix.m00 =  MathUtils::Cos(m_angle) * m_scale.x;
        m_matrix.m01 =  MathUtils::Sin(m_angle) * m_scale.x;
        m_matrix.m10 = -MathUtils::Sin(m_angle) * m_scale.y;
        m_matrix.m11 =  MathUtils::Cos(m_angle) * m_scale.y;
        RemoveFlag(RotationScale);
    }
    
    if (GetDirtyState(Position))
    {
        m_matrix.m30 = m_pos.x;
        m_matrix.m31 = m_pos.y;
        RemoveFlag(Position);
    }
}

bool Transform2D::IsWorldDirty()
{
    return GetDirtyState(Position) || GetDirtyState(RotationScale);
}

Mat4f32 const& Transform2D::GetMatrix()
{
    if ( IsWorldDirty() )
        UpdateMatrix();
    
    return m_matrix;
}

void Transform2D::UpdateFromParent(Transform2D const& parent)
{
    m_scale.x *= parent.m_scale.x;
    m_scale.y *= parent.m_scale.y;
    
    m_angle += parent.m_angle;
    
    float c = MathUtils::Cos(parent.m_angle);
    float s = MathUtils::Sin(parent.m_angle);

    float lx = m_pos.x * parent.m_scale.x;
    float ly = m_pos.y * parent.m_scale.y;

    m_pos.x = lx * c - ly * s + parent.m_pos.x;
    m_pos.y = lx * s + ly * c + parent.m_pos.y;

    AddFlag(World);
}

void Transform2D::AddFlag(uint8 _flag)
{
    m_dirty |= _flag;
}

void Transform2D::RemoveFlag(uint8 _flag)
{
    m_dirty &= ~_flag;
}

bool Transform2D::GetDirtyState(uint8 _flag) const
{
    return (m_dirty & _flag) == _flag;   
}

// ---------------------------------------------------------------------------
// Position
// ---------------------------------------------------------------------------

Vect2f32 const& Transform2D::GetPosition()
{
    return m_pos;
}

void Transform2D::SetPosition(Vect2f32 const& position)
{
    m_pos = position;
    AddFlag(Position);
}

void Transform2D::Move(Vect2f32 const& delta)
{
    m_pos.x += delta.x;
    m_pos.y += delta.y;
    AddFlag(Position);
}

void Transform2D::Move(Vect2f32 const& dir, float distance)
{
    m_pos.x += dir.x * distance;
    m_pos.y += dir.y * distance;
    AddFlag(Position);
}

// ---------------------------------------------------------------------------
// Scale
// ---------------------------------------------------------------------------

Vect2f32 const& Transform2D::GetScale()
{
    return m_scale;
}

void Transform2D::SetScale(Vect2f32 const& _scale)
{
    m_scale = _scale;
    AddFlag(RotationScale);
}

void Transform2D::SetScale(float _scale)
{
    m_scale = { _scale, _scale };
    AddFlag(RotationScale);
}

void Transform2D::Scale(Vect2f32 const& _scale)
{
    m_scale.x *= _scale.x;
    m_scale.y *= _scale.y;
    AddFlag(RotationScale);
}

void Transform2D::Scale(float _scale)
{
    m_scale.x *= _scale;
    m_scale.y *= _scale;
    AddFlag(RotationScale);
}

// ---------------------------------------------------------------------------
// Rotation
// ---------------------------------------------------------------------------

float Transform2D::GetRotation() const
{
    return m_angle;
}

Vect2f32 Transform2D::GetRight() const
{
    return Vect2f32(MathUtils::Cos(m_angle), MathUtils::Sin(m_angle));
}

Vect2f32 Transform2D::GetUp() const
{
    return Vect2f32(-MathUtils::Sin(m_angle), MathUtils::Cos(m_angle));
}

void Transform2D::SetRotation(float _angle)
{
    m_angle = _angle;
    AddFlag(RotationScale);
}

void Transform2D::Rotate(float _delta)
{
    m_angle += _delta;
    AddFlag(RotationScale);
}

void Transform2D::ResetRotation()
{
    m_angle = 0.0f;
    AddFlag(RotationScale);
}
