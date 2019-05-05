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
template<typename Hash_Type, typename Value_Type, Value_Type default_value = Value_Type()>
class Hash_Table
{
    static const size_t   max_nb_values = std::numeric_limits<Hash_Type>::max() + 1;

public:
    struct Hash_Pair
    {
        Hash_Type    hash;
        Value_Type   value;
    };

    Hash_Table()
    {
        for (size_t i = 0; i < max_nb_values; i++)
            m_table[i] = default_value;
    }

    Hash_Table(std::initializer_list<std::pair<Hash_Type, Value_Type>> values)
    {
        for (size_t i = 0; i < max_nb_values; i++)
            m_table[i] = default_value;
        for (auto& value_pair : values)
        {
            assert(m_table[value_pair.first] == default_value);  // check against conflict
            m_table[value_pair.first] = value_pair.second;
        }
    }

    Value_Type& operator[](const Hash_Type& hash) { return m_table[hash]; }


private:
    std::array<Value_Type, max_nb_values>   m_table;
};
