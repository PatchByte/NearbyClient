#include "NearbyRenderer/Guard.hpp"
#include "GLFW/glfw3.h"
#include <cstdio>
#include <cstdlib>

namespace nearby::renderer
{

    NeabryRendererGuard::NeabryRendererGuard()
    {
        glfwSetErrorCallback(
            [](int ErrorCode, const char* Description) -> void
            {
                printf("[!] GLFW Error: %i (%s)\n", ErrorCode, Description ? Description : "No Description");
                exit(-1);
            });

        glfwInit();
    }

    NeabryRendererGuard::~NeabryRendererGuard()
    {
        glfwTerminate();
    }

} // namespace nearby::renderer
