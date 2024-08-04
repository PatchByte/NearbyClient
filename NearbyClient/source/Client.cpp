#include "NearbyClient/Client.hpp"
#include "NearbyLayers/Bluetooth.h"
#include "NearbyRenderer/Renderer.hpp"
#include "imgui.h"

namespace nearby::client
{

    NearbyClient::NearbyClient() : m_Renderer(nullptr), m_LayerBluetooth(nullptr)
    {
    }

    void NearbyClient::Run()
    {
        // BLE

        m_LayerBluetooth = nearby_layer_bluetooth_create();

        // Renderer

        m_Renderer = new renderer::NearbyRendererVulkan(800, 600, "NearbyClient");

        m_Renderer->Initialize();

        while (m_Renderer->ShallRender())
        {
            m_Renderer->BeginFrame();

            this->RenderGui();

            m_Renderer->EndFrame();
        }

        m_Renderer->Shutdown();

        delete m_Renderer;

        // BLE

        nearby_layer_bluetooth_destroy(m_LayerBluetooth);
    }

    void NearbyClient::RenderGui()
    {
        if (ImGui::Begin("Test"))
        {
            if (nearby_layer_bluetooth_is_running(m_LayerBluetooth) == false)
            {
                if (ImGui::Button("Start Scanning"))
                {
                    nearby_layer_bluetooth_start_scanning(m_LayerBluetooth);
                }
            }
            else
            {
                if (ImGui::Button("Stop Scanning"))
                {
                    nearby_layer_bluetooth_stop_scanning(m_LayerBluetooth);
                }
            }
        }
        ImGui::End();
    }

} // namespace nearby::client
