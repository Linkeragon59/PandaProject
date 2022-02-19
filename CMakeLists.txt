cmake_minimum_required(VERSION 3.16)

project(PandaProject)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if ( CMAKE_SYSTEM_NAME MATCHES "Windows" )
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /WX)
elseif ( CMAKE_SYSTEM_NAME MATCHES "Linux" )
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

add_subdirectory(code/Games)
add_subdirectory(code/Modules)
add_subdirectory(code/Frameworks)
add_subdirectory(code/Extern)
