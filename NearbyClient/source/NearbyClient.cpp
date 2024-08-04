#include "NearbyClient/NearbyClient.hpp"
#include "NearbyRenderer/Renderer.hpp"

namespace nearby::client
{

    NearbyClient::NearbyClient() : m_Renderer(nullptr)
    {
    }

    void NearbyClient::Run()
    {
        m_Renderer = new renderer::NearbyRendererVulkan(800, 600, "NearbyClient");

        m_Renderer->Initialize();

        while (m_Renderer->ShallRender())
        {
            m_Renderer->BeginFrame();
            m_Renderer->EndFrame();
        }

        m_Renderer->Shutdown();

        delete m_Renderer;
    }

} // namespace nearby::client
