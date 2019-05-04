#pragma once

#include <array>
#include <limits>

#include <cassert>

/// !!! Warning the Key type have to be unsigned fundamental type
/// A pure hash table
/// It contains only an array that take the size necessary to contains all possible
/// values of Key type

// TODO
// Restrict by the Key types (unsigned int,...)
//
// Take a look if it is possible to fragment the table in multiple with a very simple
// division mecanisme, this will allow to have only used table allocated
template<typename HashType, typename ValueType, ValueType default_value = ValueType()>
class hash_table
{
    static const size_t   max_nb_values = std::numeric_limits<HashType>::max() + 1;

public:
    struct hash_pair
    {
        HashType    hash;
        ValueType   value;
    };

    hash_table()
    {
        for (size_t i = 0; i < max_nb_values; i++)
            mTable[i] = default_value;
    }

    hash_table(std::initializer_list<std::pair<HashType, ValueType>> values)
    {
        for (size_t i = 0; i < max_nb_values; i++)
            mTable[i] = default_value;
        for (auto& value_pair : values)
        {
            assert(mTable[value_pair.first] == default_value);  // check against conflict
            mTable[value_pair.first] = value_pair.second;
        }
    }

    ValueType& operator[](const HashType& hash) { return mTable[hash]; }


private:
    std::array<ValueType, max_nb_values>   mTable;
};

/// This version is safe as the original key is saved in the table
/// A deep comparaison is performed when hash are matching
template<typename KeyType, typename HashType, typename ValueType, ValueType default_value = ValueType()>
class safe_hash_table
{
    static const size_t   max_nb_values = std::numeric_limits<HashType>::max() + 1;

    struct ValueData {
        KeyType     key;
        ValueType   value;
    };

public:
    struct hash_pair
    {
        HashType    hash;
        ValueType   value;
    };

    safe_hash_table()
    {
        for (size_t i = 0; i < max_nb_values; i++)
            mTable[i] = default_value;
    }

    safe_hash_table(std::initializer_list<std::tuple<KeyType, HashType, ValueType>> values)
    {
        for (size_t i = 0; i < max_nb_values; i++)
            mTable[i] = {KeyType(), default_value};
        for (auto& value_pair : values)
        {
            assert(mTable[std::get<1>(value_pair)].value == default_value);  // check against conflict
            mTable[std::get<1>(value_pair)].key = std::get<1>(value_pair);
            mTable[std::get<1>(value_pair)].value = std::get<2>(value_pair);
        }
    }

    template<typename OtherKeyType>
    ValueType& getValue(const OtherKeyType& key, const HashType& hash)
    {
        ValueData&  valueData = mTable[hash];

        if (valueData.value != default_value    // Performing a deep comparaison only if the value was set
            && key == valueData.key)
            return valueData.value;
        return mDefaultValue;
    }

private:
    ValueType                               mDefaultValue = default_value;
    std::array<ValueData, max_nb_values>    mTable;
};
