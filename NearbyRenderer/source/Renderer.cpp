#include "NearbyRenderer/Renderer.hpp"
#include "GLFW/glfw3.h"
#include "NearbyRenderer/Guard.hpp"
#include "imgui.h"

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <vector>

#define check_vk_result(x) sfCheckVkResultImplemented(__FUNCTION__, __LINE__, x)

namespace nearby::renderer
{

    // This should be the exception rather than the rule.
    // This is ghetto
    static NeabryRendererGuard smGuard = NeabryRendererGuard();

    NearbyRendererBase::NearbyRendererBase() : m_DropFileHandler([](...) {}), m_KeyHandler([](...) {})
    {
    }

    // Vulkan

    static ImVec4 smClearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    static constexpr int smMinImageCount = 2;

    static std::vector<const char*> sfGetGlfwRequiredInstanceExtensions()
    {
        std::vector<const char*> extensions = std::vector<const char*>();

        unsigned int extensionsCount = 0;
        const char** extensionsData = glfwGetRequiredInstanceExtensions(&extensionsCount);

        for (unsigned int currentExtensionIndex = 0; currentExtensionIndex < extensionsCount; currentExtensionIndex++)
        {
            extensions.push_back(extensionsData[currentExtensionIndex]);
        }

        return extensions;
    }

    static bool sfIsExtensionAvailable(const std::vector<VkExtensionProperties>& properties, const char* extension)
    {
        for (const VkExtensionProperties& p : properties)
            if (strcmp(p.extensionName, extension) == 0)
                return true;
        return false;
    }

    static void sfCheckVkResultImplemented(std::string Function, int Line, VkResult Error)
    {
        if (Error == 0)
            return;
        printf("[CheckVkResultImplemented] Error occurred in %s at line %i (%i)\n", Function.data(), Line, Error);
        if (Error < 0)
            std::abort();
    }

    VkPhysicalDevice NearbyRendererVulkan::SetupVulkan_SelectPhysicalDevice()
    {
        uint32_t gpu_count;
        VkResult err = vkEnumeratePhysicalDevices(m_VkInstance, &gpu_count, nullptr);
        check_vk_result(err);

        std::vector<VkPhysicalDevice> gpus;
        gpus.resize(gpu_count);
        err = vkEnumeratePhysicalDevices(m_VkInstance, &gpu_count, gpus.data());
        check_vk_result(err);

        // If a number >1 of GPUs got reported, find discrete GPU if present, or use first one available. This covers
        // most common cases (multi-gpu/integrated+dedicated graphics). Handling more complicated setups (multiple
        // dedicated GPUs) is out of scope of this sample.
        for (VkPhysicalDevice& device : gpus)
        {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(device, &properties);
            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                return device;
        }

        // Use first GPU (Integrated) is a Discrete one is not available.
        if (gpu_count > 0)
            return gpus[0];
        return VK_NULL_HANDLE;
    }

    void NearbyRendererVulkan::SetupVulkan(std::vector<const char*> instance_extensions)
    {
        VkResult err;

        // Create Vulkan Instance
        {
            VkInstanceCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

            // Enumerate available extensions
            uint32_t propertiem_count;
            std::vector<VkExtensionProperties> properties;
            vkEnumerateInstanceExtensionProperties(nullptr, &propertiem_count, nullptr);
            properties.resize(propertiem_count);
            err = vkEnumerateInstanceExtensionProperties(nullptr, &propertiem_count, properties.data());
            check_vk_result(err);

            // Enable required extensions
            if (sfIsExtensionAvailable(properties, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
                instance_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
            if (sfIsExtensionAvailable(properties, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME))
            {
                instance_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
                create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
            }
#endif

            // Enabling validation layers
#ifdef IMGUI_VULKAN_DEBUG_REPORT
            const char* layers[] = {"VK_LAYER_KHRONOS_validation"};
            create_info.enabledLayerCount = 1;
            create_info.ppEnabledLayerNames = layers;
            instance_extensions.push_back("VK_EXT_debum_Vkreport");
#endif

            // Create Vulkan Instance
            create_info.enabledExtensionCount = (uint32_t)instance_extensions.size();
            create_info.ppEnabledExtensionNames = instance_extensions.data();
            err = vkCreateInstance(&create_info, m_VkAllocator, &m_VkInstance);
            check_vk_result(err);

            // Setup the debug report callback
#ifdef IMGUI_VULKAN_DEBUG_REPORT
            auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(m_VkInstance, "vkCreateDebugReportCallbackEXT");
            IM_ASSERT(vkCreateDebugReportCallbackEXT != nullptr);
            VkDebugReportCallbackCreateInfoEXT debum_Vkreport_ci = {};
            debum_Vkreport_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
            debum_Vkreport_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
            debum_Vkreport_ci.pfnCallback = debum_Vkreport;
            debum_Vkreport_ci.pUserData = nullptr;
            err = vkCreateDebugReportCallbackEXT(m_VkInstance, &debum_Vkreport_ci, m_VkAllocator, &m_VkDebugReport);
            check_vk_result(err);
#endif
        }

        // Select Physical Device (GPU)
        m_VkPhysicalDevice = SetupVulkan_SelectPhysicalDevice();

        // Select graphics queue family
        {
            uint32_t count;
            vkGetPhysicalDeviceQueueFamilyProperties(m_VkPhysicalDevice, &count, nullptr);
            VkQueueFamilyProperties* queues = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * count);
            vkGetPhysicalDeviceQueueFamilyProperties(m_VkPhysicalDevice, &count, queues);
            for (uint32_t i = 0; i < count; i++)
                if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    m_VkQueueFamily = i;
                    break;
                }
            free(queues);
            // IM_ASSERT(m_VkQueueFamily != (uint32_t)-1);
        }

        // Create Logical Device (with 1 queue)
        {
            std::vector<const char*> device_extensions;
            device_extensions.push_back("VK_KHR_swapchain");

            // Enumerate physical device extension
            uint32_t propertiem_count;
            std::vector<VkExtensionProperties> properties;
            vkEnumerateDeviceExtensionProperties(m_VkPhysicalDevice, nullptr, &propertiem_count, nullptr);
            properties.resize(propertiem_count);
            vkEnumerateDeviceExtensionProperties(m_VkPhysicalDevice, nullptr, &propertiem_count, properties.data());
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
            if (IsExtensionAvailable(properties, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME))
                device_extensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

            const float queue_priority[] = {1.0f};
            VkDeviceQueueCreateInfo queue_info[1] = {};
            queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_info[0].queueFamilyIndex = m_VkQueueFamily;
            queue_info[0].queueCount = 1;
            queue_info[0].pQueuePriorities = queue_priority;
            VkDeviceCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            create_info.queueCreateInfoCount = sizeof(queue_info) / sizeof(queue_info[0]);
            create_info.pQueueCreateInfos = queue_info;
            create_info.enabledExtensionCount = (uint32_t)device_extensions.size();
            create_info.ppEnabledExtensionNames = device_extensions.data();
            err = vkCreateDevice(m_VkPhysicalDevice, &create_info, m_VkAllocator, &m_VkDevice);
            check_vk_result(err);
            vkGetDeviceQueue(m_VkDevice, m_VkQueueFamily, 0, &m_VkQueue);
        }

        // Create Descriptor Pool
        // The example only requires a single combined image sampler descriptor for the font image and only uses one descriptor set (for that)
        // If you wish to load e.g. additional textures you may need to alter pools sizes.
        {
            VkDescriptorPoolSize pool_sizes[] = {
                {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
            };
            VkDescriptorPoolCreateInfo pool_info = {};
            pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            pool_info.maxSets = 1;
            pool_info.poolSizeCount = (uint32_t)sizeof(pool_sizes) / sizeof(*pool_sizes);
            pool_info.pPoolSizes = pool_sizes;
            err = vkCreateDescriptorPool(m_VkDevice, &pool_info, m_VkAllocator, &m_VkDescriptorPool);
            check_vk_result(err);
        }
    }

    void NearbyRendererVulkan::SetupVulkanWindow(ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height)
    {
        wd->Surface = surface;

        // Check for WSI support
        VkBool32 res;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_VkPhysicalDevice, m_VkQueueFamily, wd->Surface, &res);
        if (res != VK_TRUE)
        {
            fprintf(stderr, "Error no WSI support on physical device 0\n");
            exit(-1);
        }

        // Select Surface Format
        const VkFormat requestSurfaceImageFormat[] = {VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM};
        const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(m_VkPhysicalDevice, wd->Surface, requestSurfaceImageFormat,
                                                                  (size_t)(sizeof(requestSurfaceImageFormat) / sizeof(*requestSurfaceImageFormat)), requestSurfaceColorSpace);

        // Select Present Mode
#ifdef IMGUI_UNLIMITED_FRAME_RATE
        VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR};
#else
        VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_FIFO_KHR};
#endif
        wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(m_VkPhysicalDevice, wd->Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));
        // printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);

        ImGui_ImplVulkanH_CreateOrResizeWindow(m_VkInstance, m_VkPhysicalDevice, m_VkDevice, wd, m_VkQueueFamily, m_VkAllocator, width, height, smMinImageCount);
    }

    void NearbyRendererVulkan::CleanupVulkan()
    {
        vkDestroyDescriptorPool(m_VkDevice, m_VkDescriptorPool, m_VkAllocator);

#ifdef IMGUI_VULKAN_DEBUG_REPORT
        // Remove the debug report callback
        auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(g_Instance, "vkDestroyDebugReportCallbackEXT");
        vkDestroyDebugReportCallbackEXT(g_Instance, g_DebugReport, g_Allocator);
#endif // IMGUI_VULKAN_DEBUG_REPORT

        vkDestroyDevice(m_VkDevice, m_VkAllocator);
        vkDestroyInstance(m_VkInstance, m_VkAllocator);
    }

    void NearbyRendererVulkan::FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data)
    {
        VkResult err;

        VkSemaphore image_acquired_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
        VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
        err = vkAcquireNextImageKHR(m_VkDevice, wd->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &wd->FrameIndex);
        if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
        {
            m_SwapChainRebuild = true;
            return;
        }
        check_vk_result(err);

        ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
        {
            err = vkWaitForFences(m_VkDevice, 1, &fd->Fence, VK_TRUE, UINT64_MAX); // wait indefinitely instead of periodically checking
            check_vk_result(err);

            err = vkResetFences(m_VkDevice, 1, &fd->Fence);
            check_vk_result(err);
        }
        {
            err = vkResetCommandPool(m_VkDevice, fd->CommandPool, 0);
            check_vk_result(err);
            VkCommandBufferBeginInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
            check_vk_result(err);
        }
        {
            VkRenderPassBeginInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            info.renderPass = wd->RenderPass;
            info.framebuffer = fd->Framebuffer;
            info.renderArea.extent.width = wd->Width;
            info.renderArea.extent.height = wd->Height;
            info.clearValueCount = 1;
            info.pClearValues = &wd->ClearValue;
            vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
        }

        // Record dear imgui primitives into command buffer
        ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

        // Submit command buffer
        vkCmdEndRenderPass(fd->CommandBuffer);
        {
            VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            VkSubmitInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            info.waitSemaphoreCount = 1;
            info.pWaitSemaphores = &image_acquired_semaphore;
            info.pWaitDstStageMask = &wait_stage;
            info.commandBufferCount = 1;
            info.pCommandBuffers = &fd->CommandBuffer;
            info.signalSemaphoreCount = 1;
            info.pSignalSemaphores = &render_complete_semaphore;

            err = vkEndCommandBuffer(fd->CommandBuffer);
            check_vk_result(err);
            err = vkQueueSubmit(m_VkQueue, 1, &info, fd->Fence);
            check_vk_result(err);
        }
    }

    void NearbyRendererVulkan::FramePresent(ImGui_ImplVulkanH_Window* wd)
    {
        if (m_SwapChainRebuild)
            return;
        VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
        VkPresentInfoKHR info = {};
        info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &render_complete_semaphore;
        info.swapchainCount = 1;
        info.pSwapchains = &wd->Swapchain;
        info.pImageIndices = &wd->FrameIndex;
        VkResult err = vkQueuePresentKHR(m_VkQueue, &info);
        if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
        {
            m_SwapChainRebuild = true;
            return;
        }
        check_vk_result(err);
        wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->ImageCount;
    }

    // Actual own stuff

    void NearbyRendererVulkan::CleanupVulkanWindow()
    {
        ImGui_ImplVulkanH_DestroyWindow(m_VkInstance, m_VkDevice, &m_WindowData, m_VkAllocator);
    }

    NearbyRendererVulkan::NearbyRendererVulkan(unsigned Width, unsigned Height, std::string Title)
        : NearbyRendererBase(), m_Width(Width), m_Height(Height), m_Title(Title), m_SwapChainRebuild(false), m_VkAllocator(nullptr), m_VkInstance(VK_NULL_HANDLE), m_VkPhysicalDevice(VK_NULL_HANDLE),
          m_VkDevice(VK_NULL_HANDLE), m_VkQueueFamily(-1), m_VkQueue(VK_NULL_HANDLE), m_VkDebugReport(VK_NULL_HANDLE), m_VkPipelineCache(VK_NULL_HANDLE), m_VkDescriptorPool(VK_NULL_HANDLE),
          m_GlfwWindow(nullptr), m_OriginalDropFunction(nullptr), m_OriginalKeyFunction(nullptr)
    {
    }

    NearbyRendererVulkan::~NearbyRendererVulkan()
    {
        if (m_Running)
            Shutdown();
    }

    bool NearbyRendererVulkan::Initialize()
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_GlfwWindow = glfwCreateWindow(m_Width, m_Height, m_Title.data(), nullptr, nullptr);

        if (glfwVulkanSupported() == false)
        {
            printf("[VulkanImplementation::Initialize] glfwVulkanSupported() returned false!\n");
            return false;
        }

        std::vector<const char*> extensions = std::vector<const char*>();
        uint32_t extensions_count = 0;
        const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&extensions_count);
        for (uint32_t i = 0; i < extensions_count; i++)
            extensions.push_back(glfw_extensions[i]);
        SetupVulkan(extensions);

        VkSurfaceKHR surface;
        VkResult err = glfwCreateWindowSurface(m_VkInstance, m_GlfwWindow, m_VkAllocator, &surface);
        check_vk_result(err);

        int w, h;
        glfwGetFramebufferSize(m_GlfwWindow, &w, &h);
        ImGui_ImplVulkanH_Window* wd = &m_WindowData;
        SetupVulkanWindow(wd, surface, w, h);

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsLight();

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForVulkan(m_GlfwWindow, true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = m_VkInstance;
        init_info.PhysicalDevice = m_VkPhysicalDevice;
        init_info.Device = m_VkDevice;
        init_info.QueueFamily = m_VkQueueFamily;
        init_info.Queue = m_VkQueue;
        init_info.PipelineCache = m_VkPipelineCache;
        init_info.DescriptorPool = m_VkDescriptorPool;
        init_info.RenderPass = wd->RenderPass;
        init_info.Subpass = 0;
        init_info.MinImageCount = smMinImageCount;
        init_info.ImageCount = wd->ImageCount;
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.Allocator = m_VkAllocator;
        init_info.CheckVkResultFn = [](VkResult err) { check_vk_result(err); };
        ImGui_ImplVulkan_Init(&init_info);

        glfwSetWindowUserPointer(m_GlfwWindow, this);

        m_OriginalDropFunction = glfwSetDropCallback(m_GlfwWindow,
                                                     [](GLFWwindow* Window, int FilePathCount, const char** FilePathArray) -> void
                                                     {
                                                         NearbyRendererVulkan* vulkanImplementation = reinterpret_cast<NearbyRendererVulkan*>(glfwGetWindowUserPointer(Window));

                                                         vulkanImplementation->m_DropFileHandler(FilePathCount, FilePathArray);

                                                         if (vulkanImplementation->m_OriginalDropFunction)
                                                         {
                                                             vulkanImplementation->m_OriginalDropFunction(Window, FilePathCount, FilePathArray);
                                                         }
                                                     });

        m_OriginalKeyFunction =
            glfwSetKeyCallback(m_GlfwWindow,
                               [](GLFWwindow* Window, int Key, int ScanCode, int Action, int Mods) -> void
                               {
                                   NearbyRendererVulkan* vulkanImplementation = reinterpret_cast<NearbyRendererVulkan*>(glfwGetWindowUserPointer(Window));

                                   vulkanImplementation->m_KeyHandler(Mods & GLFW_MOD_CONTROL, Mods & GLFW_MOD_SHIFT, Mods & GLFW_MOD_ALT, Key, Action == GLFW_RELEASE, Action == GLFW_PRESS);

                                   if (vulkanImplementation->m_OriginalKeyFunction)
                                   {
                                       vulkanImplementation->m_OriginalKeyFunction(Window, Key, ScanCode, Action, Mods);
                                   }
                               });

        m_Running = true;

        return true;
    }

    bool NearbyRendererVulkan::Shutdown()
    {
        VkResult err = vkDeviceWaitIdle(m_VkDevice);
        check_vk_result(err);
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        CleanupVulkanWindow();
        CleanupVulkan();

        glfwDestroyWindow(m_GlfwWindow);
        glfwTerminate();

        m_Running = false;

        return true;
    }

    bool NearbyRendererVulkan::BeginFrame()
    {
        if (ShallRender() == false)
        {
            return false;
        }

        glfwPollEvents();

        int width, height;
        glfwGetFramebufferSize(m_GlfwWindow, &width, &height);

        if (m_SwapChainRebuild || (m_Width != width || m_Height != height))
        {
            m_Width = width;
            m_Height = height;

            if (width > 0 && height > 0)
            {
                ImGui_ImplVulkan_SetMinImageCount(smMinImageCount);
                ImGui_ImplVulkanH_CreateOrResizeWindow(m_VkInstance, m_VkPhysicalDevice, m_VkDevice, &m_WindowData, m_VkQueueFamily, m_VkAllocator, width, height, smMinImageCount);
                m_WindowData.FrameIndex = 0;
                m_SwapChainRebuild = false;
            }
        }

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        return true;
    }

    bool NearbyRendererVulkan::EndFrame()
    {
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
        if (!is_minimized)
        {
            m_WindowData.ClearValue.color.float32[0] = smClearColor.x * smClearColor.w;
            m_WindowData.ClearValue.color.float32[1] = smClearColor.y * smClearColor.w;
            m_WindowData.ClearValue.color.float32[2] = smClearColor.z * smClearColor.w;
            m_WindowData.ClearValue.color.float32[3] = smClearColor.w;
            FrameRender(&m_WindowData, draw_data);
            FramePresent(&m_WindowData);
        }

        return true;
    }

    bool NearbyRendererVulkan::ShallRender()
    {
        return m_Running && (glfwWindowShouldClose(m_GlfwWindow) == false);
    }

} // namespace nearby::renderer
