#pragma once

#include <cstddef>
#include <string>
#include <cstring>

//#define DEBUG_STRING_REF

class string_ref
{
    friend bool operator ==(const std::string& a, string_ref b);
    friend bool operator ==(string_ref a, const std::string& b);
    friend bool operator ==(string_ref a, string_ref b);
    friend bool operator !=(const std::string& a, string_ref b);
    friend bool operator !=(string_ref a, const std::string& b);
    friend bool operator !=(string_ref a, string_ref b);

public:
    string_ref() {}
    string_ref(const std::string& string, std::size_t position, std::size_t length)
        : mString(&const_cast<std::string&>(string))
        , mPosition(position)
        , mLength(length)
    {
#if defined(DEBUG_STRING_REF)
        mDebug = to_string();
#endif
    }

    const std::string&  string() const {return *mString;}
    size_t              length() const {return mLength;}
    size_t              position() const {return mPosition;}

    inline bool start_with(const std::string& string) const
    {
        if (mLength < string.length())
            return false;
        return strncmp(&mString->c_str()[mPosition], string.c_str(), string.length()) == 0;
    }

    inline bool end_with(const std::string& string) const
    {
        if (mLength < string.length())
            return false;
        return strncmp(&mString->c_str()[(mPosition + mLength) - string.length()], string.c_str(), string.length()) == 0;
    }

    std::string to_string() const;  /// Return a deep copy
    const char* starting_ptr() const;

private:
    std::string*    mString = nullptr;
    size_t          mPosition = 0;
    size_t          mLength = 0;
#if defined(DEBUG_STRING_REF)
    std::string     mDebug;
#endif
};

bool operator ==(const std::string& a, string_ref b);
bool operator ==(string_ref a, const std::string& b);
bool operator ==(string_ref a, string_ref b);
bool operator !=(const std::string& a, string_ref b);
bool operator !=(string_ref a, const std::string& b);
bool operator !=(string_ref a, string_ref b);
