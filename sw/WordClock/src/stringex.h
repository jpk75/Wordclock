#pragma once
#include "WString.h"

class StringEx : public String
{
   public:
    StringEx() = delete;
    ~StringEx() = delete;
    static int StringSplit(const String& sInput, char cDelim, String sParams[], int iMaxParams);
};