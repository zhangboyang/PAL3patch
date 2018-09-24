#pragma once

#define _CRT_SECURE_NO_WARNINGS

// C headers
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cassert>

// C++ headers
#include <vector>
#include <map>
#include <set>
#include <list>
#include <string>
#include <algorithm>
#include <memory>

// Windows headers
#define NOMINMAX 
#include <windows.h>

// scAsmTool headers
#include "util.h"
#include "scAsmDefinition.h"
#include "scAsmDisassembler.h"
#include "scAsmCommand.h"
#include "scAsmAssembler.h"
#include "scAsmTool.h"

#define stricmp _stricmp
#define MAXLINE 4096