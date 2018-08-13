#ifndef PAL3APATCH_H
#define PAL3APATCH_H

// NOTE:
//
//    FOR ALL C SOURCE FILES IN PAL3APATCH ITSELF:
//        SHOULD NOT INCLUDE THIS FILE AT ALL
//        (AND, THIS FILE IS NOT IN THE INCLUDE SEARCH PATH AT ALL)
//
//    FOR APPLICATIONS USES PAL3APATCH API:
//        SHOULD INCLUDE AND ONLY INCLUDE THIS FILE
//        SHOULD NOT INCLUDE "common/PAL3Apatch.h" DIRECTLY
//


// OPTIONAL MACROS:
//
//    PLUGIN_NAME                :   name of current compiling plugin
//
//    NO_VARIADIC_MACROS         :   no C99 variadic macros
//    USE_PAL3A_DEFINITIONS      :   use patch's PAL3 types and definitions
//

// USEFUL MACROS:
//
//    MAKE_PLUGINENTRY()         :   declare plugin entry
//    PAL3A_STRUCT_SELFCHECK()   :   run self check on struct definitions
//                                   (must define USE_PAL3_DEFINITIONS first)
//


// check compiler versions
#if defined(_MSC_VER) && _MSC_VER < 1400
#define NO_VARIADIC_MACROS
#endif


// mark we are 'importing' symbols
#define PATCHAPI_IMPORTS


// define PLUGIN_INTERNAL_NAME if needed
#ifndef PLUGIN_INTERNAL_NAME
#ifdef PLUGIN_NAME
#define PLUGIN_INTERNAL_NAME PLUGIN_NAME
#else
#ifdef TCCPLUGIN_FILE
#define PLUGIN_INTERNAL_NAME TCCPLUGIN_FILE
#else
#define PLUGIN_INTERNAL_NAME __FILE__
#endif
#endif
#endif

// define PLUGIN_FRIENDLY_NAME if needed
#ifndef PLUGIN_FRIENDLY_NAME
#define PLUGIN_FRIENDLY_NAME PLUGIN_INTERNAL_NAME
#endif

// define PLUGIN_VERSION is needed
#ifndef PLUGIN_VERSION
#define PLUGIN_VERSION BUILD_DATE
#endif

// finally, include "common.h"
#include "PAL3Apatch/common.h"

#endif
