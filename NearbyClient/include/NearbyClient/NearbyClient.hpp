#ifndef _NEARBYCLIENT_NEARBYCLIENT_HPP
#define _NEARBYCLIENT_NEARBYCLIENT_HPP

#include "NearbyRenderer/Renderer.hpp"

namespace nearby::client
{

    class NearbyClient
    {
    public:
        NearbyClient();

        void Run();
    private:
        renderer::NearbyRendererBase* m_Renderer;
    };

} // namespace nearby::client

#endif // !_NEARBYCLIENT_NEARBYCLIENT_HPP
