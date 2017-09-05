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


// OPTIONAL MACROS:
//
//    PLUGIN_NAME                :   name of current compiling plugin
//
//    NO_VARIADIC_MACROS         :   no C99 variadic macros
//    USE_PAL3_DEFINITIONS       :   use patch's PAL3 types and definitions
//

// USEFUL MACROS:
//
//    MAKE_PLUGINENTRY()         :   declare plugin entry
//    PAL3_STRUCT_SELFCHECK()    :   run self check on struct definitions
//                                   (must define USE_PAL3_DEFINITIONS first)
//


// check compiler versions
#if defined(_MSC_VER) && _MSC_VER < 1400
#define NO_VARIADIC_MACROS
#endif


// mark we are 'importing' symbols
#define PATCHAPI_IMPORTS


// define PLUGIN_NAME if needed
#ifndef PLUGIN_NAME
#ifdef TCCPLUGIN_FILE
#define PLUGIN_NAME TCCPLUGIN_FILE
#else
#define PLUGIN_NAME __FILE__
#endif
#endif


// finally, include "common.h"
#include "PAL3patch/common.h"

#endif
