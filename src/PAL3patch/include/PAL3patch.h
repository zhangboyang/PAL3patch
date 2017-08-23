#ifndef PAL3PATCH_H
#define PAL3PATCH_H

// NOTE:
//
//    FOR ALL C SOURCE FILES IN PAL3PATCH ITSELF:
//        SHOULD NOT INCLUDE THIS FILE AT ALL
//        (AND, THIS FILE IS NOT IN THE INCLUDE SEARCH PATH AT ALL)
//
//    FOR APPLICATIONS USES PAL3PATCH API:
//        SHOULD INCLUDE AND ONLY INCLUDE THIS FILE
//        SHOULD NOT INCLUDE "common/PAL3patch.h" DIRECTLY
//


// check compiler versions
#if defined(_MSC_VER) && _MSC_VER < 1400
#define NO_VARIADIC_MACROS
#endif


// mark we are 'importing' symbols
#define PATCHAPI_IMPORTS


// finally, include "common.h"
#include "PAL3patch/common.h"

#endif
