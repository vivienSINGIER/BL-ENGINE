#include "CriticalSection.h"
CriticalSection::CriticalSection() : m_isOccupied(false)
{

}

CriticalSection::~CriticalSection()
{
}

bool CriticalSection::TryEnter()
{
    if (!m_isOccupied)
    {
        m_isOccupied = true;
        return true;
    }
    return false;
}

void CriticalSection::Enter()
{
    while (!TryEnter()) {}
}

void CriticalSection::Leave()
{
    m_isOccupied = false;
}