#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_opengl3.h"

#include "Editor.h"

// Global variables
HWND hwnd = nullptr;  // Window handle
HDC hdc = nullptr;    // Device context

// Window procedure to handle window messages
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_SIZE:
	case WM_SYSCOMMAND:
	case WM_CLOSE:
		// Handle window close
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

namespace editor
{

	EditorWindow::EditorWindow(const std::string &_name, bool _isToggleable)
		: name(_name), isToggleable(_isToggleable)
	{

	}

	EditorWindow::~EditorWindow()
	{
	}

	void EditorWindow::Open()
	{
		isOpen = true;
		OnEnter();
	}

	void EditorWindow::Close()
	{
		isOpen = false;
		OnExit();
	}

	bool EditorWindow::IsOpen()
	{
		return isOpen;
	}

	bool EditorWindow::IsToggleable()
	{
		return isToggleable;
	}

	void Editor::Init()
	{
		// Register window class
		const wchar_t *className = L"ImGuiWindowClass";
		WNDCLASS wc = { };
		wc.lpfnWndProc = WindowProc;
		wc.hInstance = GetModuleHandle(nullptr);
		wc.lpszClassName = className;
		RegisterClass(&wc);

		// Create the window
		hwnd = CreateWindowEx(
			0,
			className,
			L"ImGui Window",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
			nullptr, nullptr, wc.hInstance, nullptr
		);

		// Set the OpenGL context
		PIXELFORMATDESCRIPTOR pfd = { };
		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.cRedBits = 8;
		pfd.cGreenBits = 8;
		pfd.cBlueBits = 8;
		pfd.cAlphaBits = 8;
		pfd.iLayerType = PFD_MAIN_PLANE;

		hdc = GetDC(hwnd);
		int pixelFormat = ChoosePixelFormat(hdc, &pfd);
		SetPixelFormat(hdc, pixelFormat, &pfd);

		// Create OpenGL rendering context
		//HGLRC hglrc = wglCreateContext(hdc);
		//wglMakeCurrent(hdc, hglrc);

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		Configure();

		ImGui_ImplWin32_Init(hwnd);
		ImGui_ImplOpenGL3_Init("#version 330 core");
	}

	void Editor::Shutdown()
	{
		// Cleanup ImGui
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void Editor::Update()
	{
		// Start ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport()->ID, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

		for (auto &[name, window] : windows)
			if (window->IsOpen())
				window->OnUpdate();

		// More imgui stuff
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Swap buffers
		SwapBuffers(hdc);
	}

	void Editor::OpenWindow(const std::string &windowName)
	{
		windows.at(windowName)->Open();
	}

	void Editor::CloseWindow(const std::string &windowName)
	{
		windows.at(windowName)->Close();
	}

	void Editor::ToggleWindow(const std::string &windowName)
	{
		if (windows.at(windowName)->IsOpen())
			windows.at(windowName)->Close();
		else
			windows.at(windowName)->Open();
	}

	const std::unordered_map<std::string, std::unique_ptr<EditorWindow>> &Editor::GetWindows()
	{
		return windows;
	}

	void Editor::Configure()
	{
		ImGuiIO &io = ImGui::GetIO();
		io.Fonts->AddFontFromFileTTF("../Project/Exported/Fonts/Quicksand-Regular.ttf", 27.f);
		io.WantCaptureMouse = true;

		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking

		ImGuiStyle &style = ImGui::GetStyle();
		style.FramePadding = { 6.f, 3.f };
		style.ItemSpacing = { 8.f, 5.f };
		style.ItemInnerSpacing = { 4.f, 3.f };
		style.ScrollbarSize = 16.f;
		style.GrabMinSize = 12.f;

		ImVec4 *colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text] = ImVec4(0.87f, 0.87f, 0.87f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.21f, 0.21f, 0.21f, 0.94f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.15f, 0.15f, 0.15f, 0.78f);
		colors[ImGuiCol_Border] = ImVec4(0.74f, 0.74f, 0.74f, 0.00f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.47f, 0.47f, 0.47f, 0.54f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.71f, 1.00f, 0.33f, 0.39f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.58f, 0.82f, 0.26f, 0.39f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.09f, 0.09f, 0.09f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.46f, 0.80f, 0.00f, 0.86f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.46f, 0.80f, 0.00f, 0.86f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.58f, 1.00f, 0.00f, 0.86f);
		colors[ImGuiCol_Button] = ImVec4(0.78f, 0.78f, 0.78f, 0.40f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.71f, 1.00f, 0.33f, 0.50f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.58f, 0.82f, 0.26f, 0.50f);
		colors[ImGuiCol_Header] = ImVec4(0.39f, 0.68f, 0.00f, 0.50f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.71f, 1.00f, 0.33f, 0.50f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.58f, 0.82f, 0.26f, 0.50f);
		colors[ImGuiCol_Separator] = ImVec4(0.39f, 0.68f, 0.00f, 0.76f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.45f, 0.78f, 0.00f, 0.77f);
		colors[ImGuiCol_Tab] = ImVec4(0.53f, 0.53f, 0.53f, 0.86f);
		colors[ImGuiCol_TabSelected] = ImVec4(0.39f, 0.68f, 0.00f, 0.76f);
		colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.46f, 0.80f, 0.00f, 1.00f);
		colors[ImGuiCol_TabDimmed] = ImVec4(0.28f, 0.28f, 0.28f, 0.97f);
		colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.32f, 0.55f, 0.00f, 0.76f);
		colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_DockingPreview] = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
		colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
		colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
		colors[ImGuiCol_TextLink] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	}

}