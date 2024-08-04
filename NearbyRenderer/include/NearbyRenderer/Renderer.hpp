#ifndef _NEARBYRENDERER_HPP
#define _NEARBYRENDERER_HPP

#include <functional>
#include <string>
#include <iostream>
#include <vector>

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <imgui.h>

namespace nearby::renderer
{

    class NearbyRendererBase
    {
    public:
        using sdDropFileHandler = std::function<void(int FilePathCount, const char** FilePathArray)>;
        using sdKeyHandler = std::function<void(bool IsCtrl, bool IsShift, bool IsAlt, int KeyCode, bool IsRelease, bool IsDown)>;

        NearbyRendererBase();
        virtual ~NearbyRendererBase() = default;

        virtual bool Initialize()
        {
            return false;
        }

        virtual bool Shutdown()
        {
            return false;
        }

        virtual bool BeginFrame()
        {
            return false;
        }

        virtual bool EndFrame()
        {
            return false;
        }

        virtual bool ShallRender()
        {
            return false;
        }

        virtual sdDropFileHandler GetDropFileHandler()
        {
            return m_DropFileHandler;
        }

        virtual void SetDropFileHandler(sdDropFileHandler DropFileHandler)
        {
            m_DropFileHandler = DropFileHandler;
        }

        virtual sdKeyHandler GetKeyHandler()
        {
            return m_KeyHandler;
        }

        virtual void SetKeyHandler(sdKeyHandler KeyHandler)
        {
            m_KeyHandler = KeyHandler;
        }

    protected:
        sdDropFileHandler m_DropFileHandler;
        sdKeyHandler m_KeyHandler;
    };

    class NearbyRendererVulkan : public NearbyRendererBase
        {
        public:
            NearbyRendererVulkan(unsigned Width, unsigned Height, std::string Title);
            ~NearbyRendererVulkan();

            inline unsigned GetWidth()
            {
                return m_Width;
            }

            inline unsigned GetHeight()
            {
                return m_Height;
            }

            bool Initialize();
            bool Shutdown();
            bool BeginFrame();
            bool EndFrame();
            bool ShallRender();

            inline bool IsRunning()
            {
                return m_Running;
            }

        protected:
            VkPhysicalDevice SetupVulkan_SelectPhysicalDevice();
            void SetupVulkan(std::vector<const char*> instance_extensions);
            void SetupVulkanWindow(ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height);
            void CleanupVulkan();
            void CleanupVulkanWindow();
            void FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data);
            void FramePresent(ImGui_ImplVulkanH_Window* wd);

        protected:
            unsigned m_Width;
            unsigned m_Height;
            std::string m_Title;
            bool m_Running;
            GLFWwindow* m_GlfwWindow;
            bool m_SwapChainRebuild;

            VkAllocationCallbacks* m_VkAllocator;
            VkInstance m_VkInstance;
            VkPhysicalDevice m_VkPhysicalDevice;
            VkDevice m_VkDevice;
            uint32_t m_VkQueueFamily;
            VkQueue m_VkQueue;
            VkDebugReportCallbackEXT m_VkDebugReport;
            VkPipelineCache m_VkPipelineCache;
            VkDescriptorPool m_VkDescriptorPool;

            ImGui_ImplVulkanH_Window m_WindowData;

            GLFWdropfun m_OriginalDropFunction;
            GLFWkeyfun m_OriginalKeyFunction;
        };

} // namespace nearby::renderer

#endif // !_NEARBYRENDERER_HPP
