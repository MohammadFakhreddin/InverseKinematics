#include "VisualizationApp.hpp"

using namespace MFA;

int main()
{
    LogicalDevice::InitParams params{.windowWidth = 800,
                                     .windowHeight = 800,
                                     .resizable = true,
                                     .fullScreen = false,
                                     .applicationName = "InverseKinematics"};

    auto device = LogicalDevice::Instantiate(params);
    assert(device->IsValid() == true);
    {
        VisualizationApp app{};
        app.Run();
    }

    return 0;
}