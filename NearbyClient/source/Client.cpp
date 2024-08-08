#include "NearbyClient/Client.hpp"
#include "NearbyClient/DiscoveredAdvertisements.hpp"
#include "NearbyClient/DiscoveredEndpointId.hpp"
#include "NearbyClient/DiscoveredEndpoints.hpp"
#include "NearbyLayers/Bluetooth.h"
#include "NearbyRenderer/Renderer.hpp"
#include "imgui.h"
#include <chrono>
#include <cstdio>
#include <cstring>
#include <future>
#include <thread>
#include <utility>
#include <vector>

namespace nearby::client
{

    static std::chrono::seconds smEndpointTimeoutBle = std::chrono::seconds(2);

    NearbyClient::NearbyClient() : m_Renderer(nullptr), m_LayerBluetooth(nullptr), m_DiscoveredEndpoints()
    {
    }

    void NearbyClient::Run()
    {
        // BLE

        m_LayerBluetooth = nearby_layer_bluetooth_create();

        nearby_layer_bluetooth_set_discovered_advertisement_handler(
            m_LayerBluetooth,
            [](unsigned char* MacAddress, bool IsRandomMacAddress, unsigned char* AdvertisementData, unsigned short AdvertisementLength, void* UserParameter) -> void
            {
                // We do not want to block the scanning thread, so we exit as fast as possible.
                std::ignore =
                    std::async(std::launch::async,
                               [&]
                               {
                                   // Forward this to the class handler, mmmmmhhhhhh I love C.
                                   static_cast<NearbyClient*>(UserParameter)->OnDiscoveredAdvertisement(MacAddress, IsRandomMacAddress, AdvertisementData, AdvertisementLength, UserParameter);
                               });
            },
            this);

        // Start scanning by default
        nearby_layer_bluetooth_start_scanning(m_LayerBluetooth);

        // Renderer

        m_Renderer = new renderer::NearbyRendererVulkan(800, 600, "NearbyClient");

        m_Renderer->Initialize();

        this->ApplyStyleGui();

        while (m_Renderer->ShallRender())
        {
            // Move this into another thread

            CheckForLostEndpointsAndCleanup();

            // Actual render stuff

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
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::SetNextWindowPos({0, 0});

        if (ImGui::Begin("##NearbyClientMainWindow", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_MenuBar))
        {
            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("Layers"))
                {
                    if (ImGui::BeginMenu("Bluetooth"))
                    {
                        if (bool enableScanning = nearby_layer_bluetooth_is_scanning(m_LayerBluetooth); ImGui::MenuItem("Enable Scanning", "", &enableScanning))
                        {
                            if (enableScanning)
                            {
                                nearby_layer_bluetooth_start_scanning(m_LayerBluetooth);
                            }
                            else
                            {
                                nearby_layer_bluetooth_stop_scanning(m_LayerBluetooth);
                            }
                        }

                        ImGui::EndMenu();
                    }

                    ImGui::EndMenu();
                }
            }
            ImGui::EndMenuBar();

            if (ImGui::TreeNode("Debug"))
            {
                for (auto currentDiscoveredEndpointIterator : m_DiscoveredEndpoints)
                {
                    auto currentMetadata = currentDiscoveredEndpointIterator.second->GetMetadata();

                    if (ImGui::TreeNode(currentMetadata.m_Display.data()))
                    {
                        currentDiscoveredEndpointIterator.second->RenderDebugFrame();

                        ImGui::TreePop();
                    }
                }

                ImGui::TreePop();
            }
        }
        ImGui::End();
    }

    void NearbyClient::CheckForLostEndpointsAndCleanup()
    {
        std::vector<NearbyDiscoveredEndpointId> lostEndpointIds = {};

        for (auto currentIterator : m_DiscoveredEndpoints)
        {
            if (currentIterator.second->HasLastLifeSignTimeouted(smEndpointTimeoutBle) == true)
            {
                lostEndpointIds.push_back(currentIterator.first);
            }
        }

        for (auto currentLostEndpointId : lostEndpointIds)
        {
            NearbyDiscoveredEndpointBase* lostEndpoint = m_DiscoveredEndpoints.at(currentLostEndpointId);
            m_DiscoveredEndpoints.erase(currentLostEndpointId);
            delete lostEndpoint;
        }
    }

    void NearbyClient::OnDiscoveredAdvertisement(unsigned char* MacAddress, bool IsRandomMacAddress, unsigned char* AdvertisementData, unsigned short AdvertisementLength, void* UserParameter)
    {
        std::this_thread::yield();

        NearbyDiscoveredAdvertisementBle* advertisementBle = new NearbyDiscoveredAdvertisementBle();

        if (advertisementBle->Deserialize(AdvertisementData, AdvertisementLength))
        {
            NearbyDiscoveredEndpointBle* endpoint = nullptr;

            if (m_DiscoveredEndpoints.contains(advertisementBle->GetEndpointId()))
            {
                endpoint = dynamic_cast<NearbyDiscoveredEndpointBle*>(m_DiscoveredEndpoints.at(advertisementBle->GetEndpointId()));
                endpoint->SetAdvertisement(advertisementBle);
                endpoint->SetReceivedLastLifeSign();
            }
            else
            {
                endpoint = new NearbyDiscoveredEndpointBle(MacAddress, advertisementBle);
                endpoint->SetReceivedLastLifeSign();
                m_DiscoveredEndpoints.emplace(advertisementBle->GetEndpointId(), endpoint);
            }
        }
        else
        {
            delete advertisementBle;
        }
    }

} // namespace nearby::client
