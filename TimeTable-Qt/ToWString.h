#pragma once

#if EXPERENCE
#include <string>
#include <locale>
#include <codecvt>

// convert string to wstring
std::wstring to_wide_string(const std::string& input)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(input);
}
// convert wstring to string
std::string to_byte_string(const std::wstring& input)
{
    // std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(input);
}

#ifdef UNICODE
#define string wstring
#define asString() asString().c_str()
#endif // UNICODE

#endif //  EXPERENCE

