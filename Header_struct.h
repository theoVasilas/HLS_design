#pragma once

#include <ac_int.h>

struct myStruct
{
    ac_int<3, false> flag = 1;
    ac_int<32, true> address = 0;
    ac_int<32, true> value = 0;          //signed
};
