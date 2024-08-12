#include "NearbyClient/Client.hpp"
#include "Ash/AshCRC32.h"
#include "AshLogger/AshLoggerPassage.h"
#include "AshLogger/AshLoggerTag.h"
#include "NearbyClient/DiscoveredAdvertisements.hpp"
#include "NearbyClient/DiscoveredEndpointId.hpp"
#include "NearbyClient/DiscoveredEndpoints.hpp"
#include "NearbyClient/Services/OAuth/Token.hpp"
#include "NearbyClient/Services/Variables.hpp"
#include "NearbyLayers/Bluetooth.h"
#include "NearbyProto/rpc_resources.pb.h"
#include "NearbyRenderer/Renderer.hpp"
#include "NearbyStorage/Certificate.h"
#include "NearbyStorage/CertificateManager.h"
#include "NearbyStorage/Metadata.h"
#include "fmt/chrono.h"
#include "fmt/format.h"
#include "imgui.h"
#include "nanopb-extension/pb_string_extension.h"
#include "pb.h"
#include "pb_decode.h"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <future>
#include <thread>
#include <utility>
#include <vector>

namespace nearby::client
{

    static std::chrono::seconds smEndpointTimeoutBle = std::chrono::seconds(2);

    NearbyClient::NearbyClient()
        : m_Logger("NearbyClient", {}), m_Renderer(nullptr), m_BucketPath(), m_Bucket(), m_OAuthorizer([this](services::OAuthToken Token) { OnReceivedOAuthToken(Token); }), m_NearbyShare(m_Logger),
          m_CertificateManager(nearby_storage_certificate_manager_create()), m_LayerBluetooth(nullptr), m_DiscoveredEndpoints()
    {
        m_Logger.AddLoggerPassage(
            new ash::AshLoggerFunctionPassage([](ash::AshLoggerDefaultPassage* This, ash::AshLoggerTag Tag, std::string Format, fmt::format_args Args, std::string FormattedString)
                                              { std::cout << This->GetParent()->GetPrefixFunction()(Tag, Format, Args) << " " << FormattedString << std::endl; }));

        m_Logger.SetPrefixFunction([](ash::AshLoggerTag LoggerTag, std::string LoggerFormat, fmt::format_args LoggerFormatArgs) -> std::string
                                   { return fmt::format("[{}/{}/{}]", "Nearby", LoggerTag.GetPrefix(), std::chrono::system_clock::now()); });
    }

    void NearbyClient::Run()
    {
        // Bucket

        m_BucketPath = std::filesystem::current_path() / "Bucket.json";
        m_Bucket.LoadFromFile(m_BucketPath);

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

        // Google Services

        if (m_Bucket.HasToken() == false)
        {
            m_OAuthorizer.Start();

#if _WIN32
            std::string cmdPrefix = "start";
#else
            std::string cmdPrefix = "xdg-open";
#endif

            std::system(fmt::format("{} \"{}\"", cmdPrefix, m_OAuthorizer.GetBrowserLoginUrl()).c_str());

            while (m_Bucket.HasToken() == false)
            {
                std::this_thread::yield();
            }
        }
        else
        {
            m_Logger.Log("Debug", "Loading OAuth Token from file");
            m_Logger.Log("Warning", "OAuth Token will expire in {} seconds.", m_Bucket.GetToken().GetSecondsWillExpireIn());

            OnReceivedOAuthToken(m_Bucket.GetToken());
            CheckIfNeedToRefreshToken();
        }

        this->FetchPublicCertificates();

        // Renderer

        m_Renderer = new renderer::NearbyRendererVulkan(800, 600, "NearbyClient");

        m_Renderer->Initialize();

        this->ApplyStyleGui();

        while (m_Renderer->ShallRender())
        {
            // Move this into another thread

            CheckForLostEndpointsAndCleanup();
            CheckIfNeedToRefreshToken();

            // Actual render stuff

            m_Renderer->BeginFrame();

            this->RenderGui();

            m_Renderer->EndFrame();
        }

        m_Renderer->Shutdown();

        delete m_Renderer;

        // BLE

        nearby_layer_bluetooth_destroy(m_LayerBluetooth);

        // Bucket

        SaveBucket();
    }

    void NearbyClient::SaveBucket()
    {
        m_Bucket.SaveToFile(m_BucketPath);
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
                if (ImGui::TreeNode("Public Certificates"))
                {
                    nearby_storage_certificate_manager_iterate_public_certificates(
                        this->m_CertificateManager,
                        [](nearby_storage_public_certificate* PublicCertificate, void*) -> bool
                        {
                            ash::AshCRC32 secretIdCrc = ash::AshCRC32();
                            secretIdCrc.Update(PublicCertificate->secret_id_data, PublicCertificate->secret_id_length);

                            std::string label = fmt::format("{:04x}", secretIdCrc.GetValue());

                            if (ImGui::TreeNode(label.data()))
                            {
                                ImGui::Text("IsInTimeFrame: %s", nearby_storage_public_certificate_is_in_time_frame(PublicCertificate) == true ? "yes" : "no");

                                ImGui::TreePop();
                            }

                            return true;
                        },
                        nullptr);

                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Endpoints"))
                {
                    for (auto currentDiscoveredEndpointIterator : m_DiscoveredEndpoints)
                    {
                        auto currentMetadata = currentDiscoveredEndpointIterator.second->GetMetadata();

                        if (ImGui::TreeNode(currentMetadata.m_Display.data()))
                        {
                            currentDiscoveredEndpointIterator.second->RenderDebugFrame();

                            if (ImGui::Button("Try Decrypt"))
                            {
                                auto share = static_cast<NearbyDiscoveredEndpointBle*>(currentDiscoveredEndpointIterator.second)->GetAdvertisement()->GetShare();

                                nearby_storage_decrypted_metadata_buffer* decrypted_metadata_buffer = NULL;

                                bool res = nearby_storage_certificate_manager_try_decrypt_encrypted_metadata(m_CertificateManager, share->metadata_encryption_key_hash_byte,
                                                                                                             sizeof(share->metadata_encryption_key_hash_byte), share->salt, sizeof(share->salt),
                                                                                                             &decrypted_metadata_buffer);

                                if (res)
                                {
                                    nearby_sharing_proto_Device parsedDevice = {};

                                    pb_create_string_decode_callback(&parsedDevice.name);
                                    pb_create_string_decode_callback(&parsedDevice.display_name);

                                    pb_istream_t protoInputStream = pb_istream_from_buffer(decrypted_metadata_buffer->data, decrypted_metadata_buffer->length);
                                    if (pb_decode(&protoInputStream, nearby_sharing_proto_Device_fields, &parsedDevice) == false)
                                    {
                                        m_Logger.Log("Error", "Failed to deserialize protobuf device.");
                                    }

                                    m_Logger.Log("Info", "Display Name: {}; Name: {}", pb_get_string_for_decode_callback(&parsedDevice.display_name), pb_get_string_for_decode_callback(&parsedDevice.name));

                                    pb_destroy_string_decode_callback(&parsedDevice.display_name);
                                    pb_destroy_string_decode_callback(&parsedDevice.name);

                                    nearby_storage_decrypted_metadata_buffer_destroy(decrypted_metadata_buffer);
                                }
                            }

                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }
                ImGui::TreePop();
            }
        }
        ImGui::End();
    }

    void NearbyClient::OnReceivedOAuthToken(services::OAuthToken Token)
    {
        m_Logger.Log("Info", "Received OAuth Token, saving to file");

        m_Bucket.SetToken(Token);
        m_NearbyShare.SetToken(Token);

        SaveBucket();
    }

    void NearbyClient::CheckIfNeedToRefreshToken()
    {
        if (m_Bucket.GetToken().IsExpired())
        {
            if (m_Bucket.GetToken().RefreshToken(services::Variables::GetClientId(), services::Variables::GetClientSecret()))
            {
                m_Logger.Log("Info", "Successfully refreshed token.");

                OnReceivedOAuthToken(m_Bucket.GetToken());
            }
        }
    }

    void NearbyClient::FetchPublicCertificates()
    {
        auto certificates = m_NearbyShare.ListPublicCertificates();

        for (nearby_storage_public_certificate* currentCertificate : certificates.m_Ceritficates)
        {
            if (nearby_storage_certificate_manager_get_public_certificate(this->m_CertificateManager, currentCertificate->secret_id_data, currentCertificate->secret_id_length) == nullptr)
            {
                nearby_storage_certificate_manager_add_public_certificate(this->m_CertificateManager, currentCertificate);
            }
        }
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
