#include "EditorSystem.h"
#include "../Vendor/imgui/imgui.h"
#include "../Vendor/imgui/imgui_impl_glfw.h"
#include "../Vendor/imgui/imgui_impl_vulkan.h"
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

namespace Crescent
{
	EditorSystem::EditorSystem(GLFWwindow* window)
	{
		//Setup ImGui Context.
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		(void)io;

		//Setup Dear ImGui Style.
		ImGui::StyleColorsDark();

		//Setup Platform/Renderer Bindings
		ImGui_ImplGlfw_InitForVulkan(window, true);
		ImGui_ImplVulkan_InitInfo initializationInfo{};
	}

	EditorSystem::~EditorSystem()
	{

	}
}