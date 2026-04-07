
#include "Transform2D.h"
#include <cmath>

static XMMATRIX Build2DMatrix(const XMFLOAT2& pos, const XMFLOAT2& scale, float angle)
{
    float c = cosf(angle);
    float s = sinf(angle);
    
    return XMMATRIX(
        scale.x * c,  scale.x * s,  0.0f, 0.0f,
       -scale.y * s,  scale.y * c,  0.0f, 0.0f,
        0.0f,         0.0f,         1.0f, 0.0f,
        pos.x,        pos.y,        0.0f, 1.0f
    );
}

static void StoreFloat3x3From2DMatrix(XMFLOAT3X3& out, const XMFLOAT2& pos, const XMFLOAT2& scale, float angle)
{
    float c = cosf(angle);
    float s = sinf(angle);

    out._11 = scale.x * c;   out._12 = scale.x * s;  out._13 = 0.0f;
    out._21 = -scale.y * s;  out._22 = scale.y * c;  out._23 = 0.0f;
    out._31 = pos.x;         out._32 = pos.y;         out._33 = 1.0f;
}

// ---------------------------------------------------------------------------

Transform2D::Transform2D()
{
    dirty = 0;
    SetIdentity();
}

Transform2D::Transform2D(const Transform2D& other)
    : pos(other.pos),
      scale(other.scale),
      angle(other.angle),
      matrix(other.matrix),
      dirty(other.dirty)
{
}

Transform2D::Transform2D(Transform2D&& other) noexcept
    : pos(std::move(other.pos)),
      scale(std::move(other.scale)),
      angle(other.angle),
      matrix(std::move(other.matrix)),
      dirty(other.dirty)
{
}

Transform2D& Transform2D::operator=(const Transform2D& other)
{
    if (this == &other) return *this;
    pos      = other.pos;
    scale    = other.scale;
    angle    = other.angle;
    matrix   = other.matrix;
    dirty    = other.dirty;
    return *this;
}

Transform2D& Transform2D::operator=(Transform2D&& other) noexcept
{
    if (this == &other) return *this;
    pos      = std::move(other.pos);
    scale    = std::move(other.scale);
    angle    = other.angle;
    matrix   = std::move(other.matrix);
    dirty    = other.dirty;
    return *this;
}

// ---------------------------------------------------------------------------
// Matrix
// ---------------------------------------------------------------------------

XMFLOAT3X3& Transform2D::GetMatrix()
{
    if (dirty & (uint32)DIRTY_FLAG_2D::WORLD)
        UpdateMatrix();
    return matrix;
}

void Transform2D::SetIdentity()
{
    pos   = XMFLOAT2(0.0f, 0.0f);
    scale = XMFLOAT2(1.0f, 1.0f);
    angle = 0.0f;

    XMFLOAT3X3 id;
    id._11 = 1; id._12 = 0; id._13 = 0;
    id._21 = 0; id._22 = 1; id._23 = 0;
    id._31 = 0; id._32 = 0; id._33 = 1;
    matrix    = id;

    dirty = 0;
}

void Transform2D::UpdateMatrix()
{
    StoreFloat3x3From2DMatrix(matrix, pos, scale, angle);

    dirty &= ~(uint32)DIRTY_FLAG_2D::POS;
    dirty &= ~(uint32)DIRTY_FLAG_2D::SCALE;
    dirty &= ~(uint32)DIRTY_FLAG_2D::ROTATE;
    dirty &= ~(uint32)DIRTY_FLAG_2D::WORLD;
    dirty |=  (uint32)DIRTY_FLAG_2D::INVERSE;
}

void Transform2D::UpdateFromParent(Transform2D const& parent)
{
    scale.x *= parent.scale.x;
    scale.y *= parent.scale.y;
    
    angle += parent.angle;
    
    float c = cosf(parent.angle);
    float s = sinf(parent.angle);

    float lx = pos.x * parent.scale.x;
    float ly = pos.y * parent.scale.y;

    pos.x = lx * c - ly * s + parent.pos.x;
    pos.y = lx * s + ly * c + parent.pos.y;

    dirty |= (uint32)DIRTY_FLAG_2D::WORLD | (uint32)DIRTY_FLAG_2D::INVERSE;
}

// ---------------------------------------------------------------------------
// Position
// ---------------------------------------------------------------------------

void Transform2D::SetPosition(XMFLOAT2 const& position)
{
    pos = position;
    dirty |= (uint32)DIRTY_FLAG_2D::POS;
}

void Transform2D::Move(XMFLOAT2 const& delta)
{
    pos.x += delta.x;
    pos.y += delta.y;
    dirty |= (uint32)DIRTY_FLAG_2D::POS;
}

void Transform2D::Move(XMFLOAT2 const& dir, float distance)
{
    pos.x += dir.x * distance;
    pos.y += dir.y * distance;
    dirty |= (uint32)DIRTY_FLAG_2D::POS;
}

// ---------------------------------------------------------------------------
// Scale
// ---------------------------------------------------------------------------

void Transform2D::SetScale(XMFLOAT2 const& _scale)
{
    scale = _scale;
    dirty |= (uint32)DIRTY_FLAG_2D::SCALE;
}

void Transform2D::SetScale(float _scale)
{
    scale = { _scale, _scale };
    dirty |= (uint32)DIRTY_FLAG_2D::SCALE;
}

void Transform2D::Scale(XMFLOAT2 const& _scale)
{
    scale.x *= _scale.x;
    scale.y *= _scale.y;
    dirty |= (uint32)DIRTY_FLAG_2D::SCALE;
}

void Transform2D::Scale(float _scale)
{
    scale.x *= _scale;
    scale.y *= _scale;
    dirty |= (uint32)DIRTY_FLAG_2D::SCALE;
}

// ---------------------------------------------------------------------------
// Rotation
// ---------------------------------------------------------------------------

void Transform2D::SetRotation(float _angle)
{
    angle = _angle;
    dirty |= (uint32)DIRTY_FLAG_2D::ROTATE;
}

void Transform2D::Rotate(float _delta)
{
    angle += _delta;
    dirty |= (uint32)DIRTY_FLAG_2D::ROTATE;
}

void Transform2D::ResetRotation()
{
    angle = 0.0f;
    dirty |= (uint32)DIRTY_FLAG_2D::ROTATE;
}
