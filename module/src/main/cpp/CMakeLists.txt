cmake_minimum_required(VERSION 3.22.1)
project(auditpatch)

set(C_FLAGS "${C_FLAGS} -fvisibility=hidden")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${C_FLAGS}")



# Build LSPlt as static library
set(LSPLT_BUILD_SHARED OFF CACHE BOOL "" FORCE)
add_subdirectory(external/LSPlt/lsplt/src/main/jni)

add_library(${MODULE_NAME} SHARED hook.c)

target_include_directories(${MODULE_NAME} PRIVATE 
    external/LSPlt/lsplt/src/main/jni/include
)

target_link_libraries(${MODULE_NAME} PRIVATE log lsplt_static)
