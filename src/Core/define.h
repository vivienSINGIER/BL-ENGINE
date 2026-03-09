#ifndef DEFINE_H_INCLUDED
#define DEFINE_H_INCLUDED

#include <any>
#include <DirectXCollision.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <windows.h>

#include <DirectXMath.h>
#include <map>

using namespace DirectX;

using String  = std::string;
using WString = std::wstring;
using Any     = std::any;
using Byte    = std::byte;

template <typename Type>
using Vector  = std::vector<Type>;

template<typename Key, typename Value, typename Hash = std::hash<Key>, typename Equality = std::equal_to<Key>>
using UnorderedMap = std::unordered_map<Key, Value, Hash, Equality>;

template<typename Key, typename Value, typename Hash = std::hash<Key>, typename Equality = std::equal_to<Key>>
using Map = std::map<Key, Value>;

template <typename Type>
using List = std::list<Type>;

using int8      = int8_t;
using int16     = int16_t;
using int32     = int32_t;
using int64     = int64_t;

using uint8      = uint8_t;
using uint16     = uint16_t;
using uint32     = uint32_t;
using uint64     = uint64_t;

const WString GetResPath();

#define RES(path) (GetResPath() + L##path)

#endif