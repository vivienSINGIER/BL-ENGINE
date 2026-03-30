#ifndef CRITICALSECTION_H_DEFINED
#define CRITICALSECTION_H_DEFINED

class CriticalSection
{
public:
    CriticalSection();
    ~CriticalSection();

    bool TryEnter();
    void Enter();
    void Leave();

private:
    bool m_isOccupied;
};

#endif // !CRITICALSECTION_H_DEFINED


