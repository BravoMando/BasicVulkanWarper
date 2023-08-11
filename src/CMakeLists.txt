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
                    Render_System
                    Camera
                    Input_Controller
                    ../deps/tinyobjloader
                    Utils
                    Buffer
                    ${VulkanSDK_Include_Dir})

add_custom_target(BuildShader
                ALL
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/res/shaders
                COMMAND glslc basic_vert.vert -o basic_vert.spv
                COMMAND glslc basic_frag.frag -o basic_frag.spv)

set(CMAKE_CXX_STANDARD 17)

if(MSVC)
    set(CMAKE_CXX_FLAGS "/Zi /W3")
else()
    set(CMAKE_CXX_FLAGS "-g -Wall")
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
