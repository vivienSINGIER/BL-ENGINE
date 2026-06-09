#ifndef TEST_H_DEFINED
#define TEST_H_DEFINED

#include <iostream>

class Test
{
public:
    Test() = default;
    virtual ~Test() = default;
    
    virtual void Run() {}
    
    void DisplayResults()
    {
        std::cout << "\033[92m [PASS] -> " << m_successCount << "\n";
        std::cout << "\033[91m [FAIL] -> " << m_failCount << "\n";
    }
    
    ///////////////////////////////////////////////////////////////////////////
    
    template <typename ObjectType, typename ReturnType, typename... FuncArgs, typename... Args>
    bool TestObjectValue(
        const char* _name, 
        ObjectType& _object, 
        ReturnType (ObjectType::*_func)(FuncArgs...),
        ReturnType _expected,
        Args&&... _args)
    {
        ReturnType result = (_object.*_func)(_args...);
        bool passed = (result == _expected);
        
        Report(_name, result, _expected, passed);
        
        return passed;
    }
    
    template <typename ObjectType, typename ReturnType, typename... FuncArgs, typename... Args>
    bool TestObjectValue(
        const char* _name, 
        ObjectType& _object, 
        ReturnType (ObjectType::*_func)(FuncArgs...),
        bool (*_compareFunc)(ReturnType const&, ReturnType const&),
        ReturnType _expected,
        Args&&... _args)
    {
        ReturnType result = (_object.*_func)(_args...);
        bool passed = (*_compareFunc)(result, _expected);
        
        Report(_name, result, _expected, passed);
        
        return passed;
    }
    
    template <typename ObjectType, typename ReturnType, typename... FuncArgs, typename... Args>
    bool TestObjectValue(
        const char* _name, 
        const ObjectType& _object, 
        ReturnType (ObjectType::*_func)(FuncArgs...) const,
        ReturnType _expected,
        Args&&... _args)
    {
        ReturnType result = (_object.*_func)(_args...);
        bool passed = (result == _expected);
        
        Report(_name, result, _expected, passed);
        
        return passed;
    }
    
    template <typename ObjectType, typename ReturnType, typename... FuncArgs, typename... Args>
    bool TestObjectValue(
        const char* _name, 
        const ObjectType& _object, 
        ReturnType (ObjectType::*_func)(FuncArgs...) const,
        bool (*_compareFunc)(ReturnType const&, ReturnType const&),
        ReturnType _expected,
        Args&&... _args)
    {
        ReturnType result = (_object.*_func)(_args...);
        bool passed = (*_compareFunc)(result, _expected);
        
        Report(_name, result, _expected, passed);
        
        return passed;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    
    template <typename ObjectType, typename... FuncArgs, typename... Args>
    bool TestObjectSelf(
        const char* _name, 
        ObjectType& _object, 
        ObjectType& (ObjectType::*_func)(FuncArgs...),
        ObjectType _expected,
        Args&&... _args)
    {
        ObjectType copy = _object;
        (copy.*_func)(_args...);
        bool passed = (copy == _expected);
        
        Report(_name, copy, _expected, passed);
        
        return passed;
    }
    
    template <typename ObjectType, typename... FuncArgs, typename... Args>
    bool TestObjectSelf(
        const char* _name, 
        ObjectType& _object, 
        ObjectType& (ObjectType::*_func)(FuncArgs...),
        bool (*_compareFunc)(ObjectType const&, ObjectType const&),
        ObjectType _expected,
        Args&&... _args)
    {
        ObjectType copy = _object;
        (copy.*_func)(_args...);
        bool passed = (*_compareFunc)(copy, _expected);
        
        Report(_name, copy, _expected, passed);
        
        return passed;
    }
    
    template <typename ObjectType, typename... FuncArgs, typename... Args>
    bool TestObjectSelf(
        const char* _name, 
        const ObjectType& _object, 
        ObjectType& (ObjectType::*_func)(FuncArgs...) const,
        ObjectType _expected,
        Args&&... _args)
    {
        ObjectType copy = _object;
        (copy.*_func)(_args...);
        bool passed = (copy == _expected);
        
        Report(_name, copy, _expected, passed);
        
        return passed;
    }
    
    template <typename ObjectType, typename... FuncArgs, typename... Args>
    bool TestObjectSelf(
        const char* _name, 
        const ObjectType& _object, 
        ObjectType& (ObjectType::*_func)(FuncArgs...) const,
        bool (*_compareFunc)(ObjectType const&, ObjectType const&),
        ObjectType _expected,
        Args&&... _args)
    {
        ObjectType copy = _object;
        (copy.*_func)(_args...);
        bool passed = (*_compareFunc)(copy, _expected);
        
        Report(_name, copy, _expected, passed);
        
        return passed;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    
    template <typename ReturnType, typename... FuncArgs,typename... Args>
    bool TestValue(
        const char* _name,
        ReturnType (*_func)(FuncArgs...),
        ReturnType _expected,
        Args&&... _args)
    {
        ReturnType result = (_func)(_args...);
        bool passed = (result == _expected);
        
        Report(_name, result, _expected, passed);
        
        return passed;
    }
    
    template <typename ReturnType, typename... FuncArgs,typename... Args>
    bool TestValue(
        const char* _name,
        ReturnType (*_func)(FuncArgs...),
        bool (*_compareFunc)(ReturnType const&, ReturnType const&),
        ReturnType _expected,
        Args&&... _args)
    {
        ReturnType result = (_func)(_args...);
        bool passed = (*_compareFunc)(_expected, result);
        
        Report(_name, result, _expected, passed);
        
        return passed;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    
private:
    int m_successCount = 0;
    int m_failCount = 0;
    
    template <typename T>
    void Report(const char* _name, const T& _value, const T& _expected, bool _passed)
    {
        std::cout << (_passed ? "\033[92m" : "\033[91m")
              << "[" << (_passed ? "PASS" : "FAIL") << "] "
              << "\033[37m" << _name << " -> "
              << "Expected (" << _expected << "), "
              << "Got (" << _value << ")"
              << "\033[0m\n";
        
        if (_passed) m_successCount++;
        else m_failCount++;
    }
};

#endif