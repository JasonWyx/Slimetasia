#pragma once

#include <cstring>

#define SMARTENUM_VAL(type, val) e##type##_##val,
#define SMARTENUM_STRING(type, val) #val,

#define SMARTENUM_DEFINE_ENUM(type, vals)    \
    enum type                                \
    {                                        \
        vals(SMARTENUM_VAL) e##type##_count, \
    };

#define SMARTENUM_DEFINE_NAMES(type, vals) static const char* type##Array[] = {vals(SMARTENUM_STRING)};

#define SMARTENUM_DEFINE_GET_VALUE_FROM_STRING(type)          \
    static type get##type##FromString(const char* str)        \
    {                                                         \
        for (int i = 0; i < e##type##_count; ++i)             \
            if (!strcmp(type##Array[i], str)) return (type)i; \
        return e##type##_count;                               \
    }

#define SMARTENUM_DEFINE_GET_ALL_VALUES_IN_STRING(type)              \
    static std::vector<const char*> get##all##type##ValuesInString() \
    {                                                                \
        std::vector<const char*> tmp;                                \
        tmp.reserve(e##type##_count);                                \
        for (int i = 0; i < e##type##_count; i++)                    \
            tmp.push_back(type##Array[i]);                           \
        return tmp;                                                  \
    }

#define getStringFromEnumVal(type, val) type##Array[val]
#define getEnumValueFromString(type, name) get##type##FromString(name)
#define getMaxEnumValue(type) e##type##_count
