set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

file(GLOB_RECURSE
    SRC_LIST
    ./*.hpp
    ./*.cpp)

find_package(VulkanSDK REQUIRED)
if(NOT VulkanSDK_FOUND)
    message(FATAL_ERROR "Couldn't find Vulkan SDK!")
endif()

include_directories(../deps/glfw/include
                    ../deps/glm
                    Window
                    Device
                    SwapChain
                    Pipeline
                    Model
                    Game_Object
                    Renderer
                    Systems
                    Camera
                    Input_Controller
                    ../deps/tinyobjloader
                    Utils
                    Buffer
                    Descriptor
                    ${VulkanSDK_Include_Dir})

file(GLOB_RECURSE
    GLSL_SRC_LIST
    ${PROJECT_SOURCE_DIR}/res/shaders/*.vert
    ${PROJECT_SOURCE_DIR}/res/shaders/*.tesc
    ${PROJECT_SOURCE_DIR}/res/shaders/*.tese
    ${PROJECT_SOURCE_DIR}/res/shaders/*.geom
    ${PROJECT_SOURCE_DIR}/res/shaders/*.frag
    ${PROJECT_SOURCE_DIR}/res/shaders/*.comp)

foreach(GLSL ${GLSL_SRC_LIST})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV ${PROJECT_SOURCE_DIR}/res/shaders/${FILE_NAME}.spv)
    add_custom_command(OUTPUT ${SPIRV}
                        PRE_BUILD
                        COMMAND ${GLSLC} ${GLSL} -o ${SPIRV}
                        DEPENDS ${GLSL})
    list(APPEND SPIRV_BIN_LIST ${SPIRV})
endforeach()

add_custom_target(BUILD_SHADER
                ALL
                DEPENDS ${SPIRV_BIN_LIST})

set(CMAKE_CXX_STANDARD 17)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi /W3")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall")
endif()

add_executable(${PROJECT_NAME} ${SRC_LIST})

target_compile_definitions(${PROJECT_NAME} PRIVATE HOME_DIR="${PROJECT_SOURCE_DIR}/")

target_link_directories(${PROJECT_NAME} PRIVATE ${VulkanSDK_Libraries_Dir})

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE glfw vulkan-1)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE glfw vulkan)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_NAME App)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build)
