set(VulkanSDK_FOUND TRUE)
if(NOT DEFINED ENV{VULKAN_SDK})

    find_package(Vulkan REQUIRED)

    if(NOT Vulkan_FOUND)
        message(FATAL_ERROR "\nYour PC doesn't seem to have Vulkan installed!\n")
    endif()

else()

    set(VulkanSDK_Include_Dir $ENV{VULKAN_SDK}/Include)
    set(VulkanSDK_Libraries_Dir $ENV{VULKAN_SDK}/Lib)
    set(VulkanSDK_Runtime_Dir $ENV{VULKAN_SDK}/Bin)

endif()
