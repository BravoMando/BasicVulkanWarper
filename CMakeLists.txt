cmake_minimum_required(VERSION 3.20.0)

#set(CMAKE_C_COMPILER clang.exe)
#set(CMAKE_CXX_COMPILER clang++.exe)

project(VKWARPER VERSION 1.0.0 LANGUAGES CXX)

add_subdirectory(src)
add_subdirectory(deps/glfw)

set(GLFW_BUILD_DOCS OFF CACHE BOOL  "GLFW lib only" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL  "GLFW lib only" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "GLFW lib only" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "GLFW lib only" FORCE)

if(MSVC)
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup")
    if(${CMAKE_VERSION} VERSION_LESS "3.6.0")
        message(WARNING "\n\t - CMake version lower than 3.6.0;\n\t - Please update CMake and rerun; OR\n\t - Manually set '${PROJECT_NAME}' as StartUp Project in Visual Studio.\n")
    else()
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
    endif()
endif()
