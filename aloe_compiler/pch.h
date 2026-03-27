// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"
#pragma warning(disable : 4146)
#pragma warning(disable : 4244)
#pragma warning(disable : 4624)
#pragma warning(disable : 4267)

#include <iostream>
#include <filesystem>
#include <string>
#include <stdarg.h>


#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/DIBuilder.h>
#include <llvm/BinaryFormat/Dwarf.h>
#include <llvm/IR/Type.h>
#include <aloe/logger.h>
#include <llvm/Support/raw_os_ostream.h>
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Function.h"



#endif //PCH_H
