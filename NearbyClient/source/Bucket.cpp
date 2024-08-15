#include "NearbyClient/Bucket.hpp"
#include "nlohmann/json_fwd.hpp"
#include <filesystem>
#include <fstream>

namespace nearby::client
{

    Bucket::Bucket() : m_HasToken(false), m_Token()
    {
    }

    void Bucket::SaveToFile(std::filesystem::path Path)
    {
        nlohmann::json data = {};

        if (m_HasToken)
        {
            data["token"] = m_Token.Export();
        }

        std::string dataSerialized = data.dump();
        std::ofstream outputStream = std::ofstream(Path, std::ios::trunc);

        outputStream << dataSerialized;
        outputStream.flush();
        outputStream.close();
    }

    bool Bucket::LoadFromFile(std::filesystem::path Path)
    {
        if (std::filesystem::exists(Path) == false)
        {
            return false;
        }

        nlohmann::json data = {};
        std::ifstream inputStream = std::ifstream(Path);

        inputStream >> data;

        if (data["token"].is_object())
        {
            m_HasToken = m_Token.Import(data["token"]);
        }

        return true;
    }

} // namespace nearby::client
