#include "string_ref.hpp"

std::string string_ref::to_string() const
{
    if (length() == 0)
        return "";
    return std::string(&mString->c_str()[mPosition], mLength);
}

const char* string_ref::starting_ptr() const
{
    return &mString->c_str()[mPosition];
}

bool operator ==(const std::string& a, string_ref b)
{
    if (a.length() != b.mLength)
        return false;
    return strncmp(a.c_str(), &b.mString->c_str()[b.mPosition], a.length()) == 0;
}

bool operator ==(string_ref a, const std::string& b)
{
    return operator ==(b, a);
}

bool operator ==(string_ref a, string_ref b)
{
    if (a.mLength != b.mLength)
        return false;
    return strncmp(&a.mString->c_str()[a.mPosition], &b.mString->c_str()[b.mPosition], a.mLength) == 0;
}

bool operator !=(const std::string& a, string_ref b)
{
    return !operator ==(a, b);
}

bool operator !=(string_ref a, const std::string& b)
{
    return !operator ==(b, a);
}

bool operator !=(string_ref a, string_ref b)
{
    return !operator ==(a, b);
}
