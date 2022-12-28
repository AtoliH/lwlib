#include "lwlib/FastDevice.hpp"

#ifdef __APPLE__
#include <mach-o/getsect.h>
#include <mach-o/ldsyms.h>
#else
extern "C"
{
    extern char _binary_dmgcalc_cl_start[];
    extern char _binary_dmgcalc_cl_end[];
    extern size_t _binary_dmgcalc_cl_size;

    extern char _binary_types_cl_start[];
    extern char _binary_types_cl_end[];
    extern size_t _binary_types_cl_size;

    extern char _binary_macro_h_start[];
    extern char _binary_macro_h_end[];
    extern size_t _binary_macro_h_size;
}
#endif

std::string FastDevice::getTypes()
{
#ifdef __APPLE__
    unsigned long size;
    uint8_t *data = getsectiondata(&_mh_dylib_header, "binary", "types_cl_bin", &size);
    std::string typesCode(reinterpret_cast<char*>(data), size * sizeof(uint8_t) / sizeof(char));
#else
    std::string typesCode(_binary_types_cl_start, _binary_types_cl_size);
#endif
    return typesCode;
}

std::string FastDevice::getDmgcalc()
{
#ifdef __APPLE__
    unsigned long size;
    uint8_t *data = getsectiondata(&_mh_dylib_header, "binary", "dmgcalc_cl_bin", &size);
    std::string dmgcalcCode(reinterpret_cast<char*>(data), size * sizeof(uint8_t) / sizeof(char));
#else
    std::string dmgcalcCode(_binary_dmgcalc_cl_start, _binary_dmgcalc_cl_size);
#endif
    return dmgcalcCode;
}

std::string FastDevice::getMacro()
{
#ifdef __APPLE__
    unsigned long size;
    uint8_t *data = getsectiondata(&_mh_dylib_header, "binary", "macro_h_bin", &size);
    std::string macroCode(reinterpret_cast<char*>(data), size * sizeof(uint8_t) / sizeof(char));
#else
    std::string macroCode(_binary_macro_h_start, _binary_macro_h_size);
#endif
    return macroCode;
}