#include "Renderer/Renderer.h"

/*
    A RHI (Render Hardware Interface) is the abstraction layer between a graphics API and the renderer. It allows the renderer to be completely API
    independant. The RHI abstracts API concepts to provide a simpler interface for the renderer.

    - Renderer Class: Serves as our highest level of abstraction. All command calls are generic here, such as DrawInstanced(), CreateTexture() that are
    filtered to the underlying RHI interface. The Renderer class may utilize various subclasses to keep its interface clean. For example, a subclass could
    exist for the creation of pipeline resources such as samplers and PSOs, whilst another could keep track of render options, such as gizmo sizes and multisample levels.

    - RHI Classes: Our second layer of abstraction between our renderer and each of our underlying implemented graphics APIs. This is the meat of our RHI
    concept and involves countless classes, each for a specific part of our rendering pipeline. Think RHI_Texture, RHI_Device and RHI_Buffer, which
    when inherited, identifies as DX11_Texture, DX12_Device and Vulkan_Buffer. Our renderer class will make all calls in reference to the RHI classes.
    It should have absolutely no references to the underlying API implementations.

    Not just so, utility functions will have to be implemented to convert abstract concepts such as comparison functions, primitive topologies, formats etc from our
    native engine implementations to each API that we're utilizing.

    - API Classes: Finally, we have implementations for each API in this final layer.

    There is to be no #include, #ifdefs or whatsoever in our code base. Everything is compiled and can be dynamically chosen.
*/

int main(int argc, int argv[])
{
    Aurora::Renderer rendererContext;
    rendererContext.SetRenderAPI(Aurora::RenderAPI::DirectX12);
    rendererContext.Initialize();

    return 0;
}