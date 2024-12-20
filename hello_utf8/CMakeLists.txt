cmake_minimum_required(VERSION 3.18)

# uncomment for debug
# set(CMAKE_VERBOSE_MAKEFILE 1)

# force LLVM MinGW
#set(CMAKE_C_COMPILER   x86_64-w64-mingw32-clang)
#set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-clang++)

# Force multi-byte build for UTF-8 support
add_definitions(-D_MBCS)

project(hello_utf8 VERSION 0.1.0 LANGUAGES C)

add_executable(hello_utf8 WIN32
  hello_utf8.c
  hello_utf8.rc
)

if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
  target_compile_options(hello_utf8 PRIVATE
    -Os
  )
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
  target_compile_options(hello_utf8 PRIVATE
    # Use UTF-8 source code
    /utf-8
    # Enable C++ exception
    /EHsc
  )

  # Use hybrid C-runtime (Dynamic universal CRT + static VC++ specific)
  target_link_options(hello_utf8 PRIVATE
    /subsystem:console
    /NODEFAULTLIB:vcruntime$<$<CONFIG:Debug>:d>.lib
    /DEFAULTLIB:libvcruntime$<$<CONFIG:Debug>:d>.lib
  )
endif()
