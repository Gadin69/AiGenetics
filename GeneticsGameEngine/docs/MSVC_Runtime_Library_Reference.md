# MSVC Runtime Library Linking - Reference Guide
# Created: 2026-05-01
# Purpose: Correct CMake configuration for MSVC runtime libraries in Windows C++ projects

## PROBLEM IDENTIFIED
Error: LNK2019 unresolved external symbols:
- __vcrt_initialize
- __vcrt_uninitialize
- __vcrt_uninitialize_critical
- __vcrt_thread_attach
- __vcrt_thread_detach
- _is_c_termination_complete
- __acrt_initialize
- __acrt_uninitialize
- __acrt_uninitialize_critical
- __acrt_thread_attach
- __acrt_thread_detach

Source: msvcrt.lib(utility.obj)

## ROOT CAUSE
When using CMake's MSVC_RUNTIME_LIBRARY property (which sets /MD or /MDd flags),
CMake automatically handles CRT library linking. EXPLICITLY linking msvcrt.lib,
ucrt.lib, or vcruntime.lib creates conflicts because:

1. msvcrt.lib is an import library that references symbols in vcruntime140.dll and ucrtbase.dll
2. The linker uses /alternatename pragmas to map __vcrt_* symbols to __acrt_* stubs
3. Manual linking breaks this mapping and causes unresolved externals
4. Modern Windows SDK (10.0.22621+) handles CRT differently than older versions

## SOLUTION - CORRECT CMAKE CONFIGURATION

### DO THIS:
```cmake
# At project level (CMakeLists.txt)
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
endif()

# OR at target level:
set_target_properties(MyTarget PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
)
```

### NEVER DO THIS:
```cmake
# WRONG - causes LNK2019 errors
target_link_libraries(MyTarget PRIVATE
    msvcrt.lib    # CONFLICT with /MD flag
    ucrt.lib      # CONFLICT with /MD flag
    vcruntime.lib # CONFLICT with /MD flag
)
```

## RUNTIME LIBRARY OPTIONS

| CMake Property Value | Compiler Flag | Description |
|---------------------|---------------|-------------|
| MultiThreaded | /MT | Static release CRT |
| MultiThreadedDebug | /MTd | Static debug CRT |
| MultiThreadedDLL | /MD | Dynamic release CRT (RECOMMENDED) |
| MultiThreadedDebugDLL | /MDd | Dynamic debug CRT |

## WINDOWS SUBSYSTEM CONFIGURATION

### For Windows GUI Application:
```cmake
if(MSVC)
    target_link_options(MyTarget PRIVATE
        $<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS>
        $<$<CONFIG:Debug>:/SUBSYSTEM:WINDOWS>
    )
endif()
```

### For Console Application:
```cmake
if(MSVC)
    target_link_options(MyTarget PRIVATE
        $<$<CONFIG:Release>:/SUBSYSTEM:CONSOLE>
        $<$<CONFIG:Debug>:/SUBSYSTEM:CONSOLE>
    )
endif()
```

### DO NOT set custom entry points unless necessary:
```cmake
# AVOID - this causes CRT initialization issues
target_link_options(MyTarget PRIVATE /ENTRY:WinMain)

# Let CMake/MSVC handle entry point automatically
# The CRT will call your WinMain or main function
```

## STANDARD WINDOWS LIBRARIES TO LINK

```cmake
target_link_libraries(MyTarget PRIVATE
    # DirectX 12
    d3d12.lib
    dxgi.lib
    d3dcompiler.lib
    
    # Windows system libraries
    kernel32.lib
    user32.lib
    gdi32.lib
    winspool.lib
    shell32.lib
    ole32.lib
    oleaut32.lib
    uuid.lib
    comdlg32.lib
    advapi32.lib
    
    # DO NOT ADD: msvcrt.lib, ucrt.lib, vcruntime.lib
    # CMake handles these automatically with MSVC_RUNTIME_LIBRARY
)
```

## CMAKE MINIMUM REQUIREMENTS

```cmake
cmake_minimum_required(VERSION 3.15)  # 3.15+ for MSVC_RUNTIME_LIBRARY
project(MyProject VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set runtime library for all targets
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
endif()
```

## COMPILER OPTIMIZATION FLAGS

### CORRECT MSVC optimization flags:
```cmake
if(MSVC)
    set_target_properties(MyTarget PROPERTIES
        COMPILE_OPTIONS "$<$<CONFIG:Release>:/O2 /GL /Gw /Gy>"
    )
endif()
```

### INCORRECT flags that cause warnings:
- /O  (space after O - INVALID)
- /O/ (slash after O - INVALID)
- /OL (old flag - DEPRECATED)
- /Ow (deprecated)

### Correct flags:
- /O2 - Optimize for speed
- /O1 - Optimize for size
- /GL - Whole program optimization
- /Gw - Separate global data
- /Gy - Enable function-level linking

## KNOWN ISSUES AND WORKAROUNDS

### Issue 1: Linker still fails after removing explicit CRT libs
Solution: Clean build directory and regenerate
```powershell
Remove-Item -Recurse -Force build
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### Issue 2: Multiple targets with different runtime settings
Solution: Set runtime library at project level, not target level
```cmake
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
endif()
```

### Issue 3: Static libraries need runtime library too
Solution: Set runtime library on static library targets as well
```cmake
add_library(MyLib STATIC ...)
if(MSVC)
    set_target_properties(MyLib PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
    )
endif()
```

## REFERENCE LINKS
- StackOverflow: LNK2019 __vcrt_initialize - https://stackoverflow.com/questions/54124505
- CMake MSVC_RUNTIME_LIBRARY docs - https://cmake.org/cmake/help/latest/prop_tgt/MSVC_RUNTIME_LIBRARY.html
- MSVC CRT linking - https://learn.microsoft.com/en-us/cpp/build/reference/md-mt-ld-use-run-time-library

## SUMMARY
1. Use CMAKE_MSVC_RUNTIME_LIBRARY property
2. NEVER manually link msvcrt.lib, ucrt.lib, or vcruntime.lib
3. Let CMake handle CRT automatically
4. Clean build directory if you made changes
5. Use proper MSVC optimization flags (/O2, not /O )
