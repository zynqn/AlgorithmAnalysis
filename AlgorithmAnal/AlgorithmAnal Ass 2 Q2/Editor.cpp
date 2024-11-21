#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"

#include "Windows.h"
#include "gl/GL.h"
#include "IconsFontAwesome5.h"
#include "Editor.h"
#include "SeamCarving2.h"

#include <filesystem>

// File Loader
// Seam Carving
// Windows

//todo choose to reset image or not after carving
//todo expose api for editor

/*! ------------ Function Wrapper Macros ------------ */

#define IconWrap(code) { ImGui::PushFont(iconFont); { code } ImGui::PopFont(); }
#define StyleVarWrap(style, vec, code) { ImGui::PushStyleVar(style, vec); { code } ImGui::PopStyleVar(); }
#define StyleWrap(style, color, code) { ImGui::PushStyleColor(style, color); { code } ImGui::PopStyleColor(); }

#define LIGHT_GRAY IM_COL32(192, 192, 192, 255)
#define LIGHT_ROSE IM_COL32(0xFF, 0x99, 0xC8, 0xFF)
#define LIGHT_YELLOW IM_COL32(0xFC, 0xF6, 0xBD, 0xFF)
#define LIGHT_GREEN IM_COL32(0xD0, 0xF4, 0xDE, 0xFF)
#define LIGHT_BLUE IM_COL32(0xA9, 0xDE, 0xF9, 0xFF)
#define LIGHT_PURPLE IM_COL32(0xE4, 0xC1, 0xF9, 0xFF)

ImFont *textFont = nullptr;
ImFont *iconFont = nullptr;

// Global variables
HWND hwnd = nullptr;  // Window handle
HDC hdc = nullptr;    // Device context

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Window procedure to handle window messages
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	// Crucial: Add ImGui message handling
	if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
		return true;

	switch (uMsg) {
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;

	case WM_SIZE: {
		int width = LOWORD(lParam);  // Client area width
		int height = HIWORD(lParam); // Client area height
		if (wParam != SIZE_MINIMIZED) {
			glViewport(0, 0, width, height); // Update OpenGL viewport
		}
		return 0;
	}

	case WM_SYSCOMMAND:
		if ((wParam & 0xFFF0) == SC_CLOSE) {
			PostQuitMessage(0); // Close the application
			return 0;
		}
		break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

namespace edit
{

	inline void AddSpace(int spaceCount)
	{
		for (int i = 0; i < spaceCount; ++i)
			ImGui::Spacing();
	}

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

	ImageLoader::ImageLoader(const std::string &_name, bool _isToggleable)
		: EditorWindow(_name, _isToggleable)
	{

	}

	void ImageLoader::OnEnter()
	{

	}

	void ImageLoader::OnUpdate()
	{
		//ImGuiWindowClass windowClass;
		//windowClass.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
		//ImGui::SetNextWindowClass(&windowClass);

		ImGui::Begin(name.c_str(), nullptr, ImGuiWindowFlags_NoDecoration);
		ImGui::PushTextWrapPos(ImGui::GetWindowWidth());

		AddSpace(2);
		ImGui::Text("Double click a file in the list box to open it or drag and drop it into the bar below to load it.");
		AddSpace(2);

		if (ImGui::BeginListBox("##Files", ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing())))
		{
			size_t count = 0;

			for (const std::filesystem::directory_entry &entry : std::filesystem::directory_iterator("assets/images"))
			{
				if (ImGui::Selectable((entry.path().filename().string() + "##ListBox").c_str(), count == selected))
					selected = count;

				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemClicked())
					ShellExecute(0, 0, entry.path().wstring().c_str(), 0, 0, SW_SHOW);

				if (ImGui::BeginDragDropSource())
				{
					std::string payload = entry.path().string();
					ImGui::SetDragDropPayload("File_Payload", payload.c_str(), payload.size(), ImGuiCond_Once);
					ImGui::Text("%s", entry.path().filename().string().c_str());
					ImGui::EndDragDropSource();
				}

				++count;
			}

			ImGui::EndListBox();
		}

		AddSpace(2);
		ImGui::InputText("##InputFile", &loadedFile, ImGuiInputTextFlags_ReadOnly);

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("File_Payload"))
			{
				std::filesystem::path asset = std::string(static_cast<const char *>(payload->Data), payload->DataSize);
				if ((asset.extension() == ".png" || asset.extension() == ".jpg") && std::filesystem::exists(asset))
				{
					loadedFile = asset.filename().string();
					originalImg = cv::imread(asset.filename().string());
				}
			}

			ImGui::EndDragDropTarget();
		}

		ImGui::SameLine();
		IconWrap(ImGui::Button(ICON_FA_MINUS "##Remove_File");)
		if (ImGui::IsItemClicked())
			loadedFile = "No file selected";

		ImGui::PopTextWrapPos();
		ImGui::End();
	}

	void ImageLoader::OnExit()
	{

	}

	SeamCarver::SeamCarver(const std::string &_name, bool _isToggleable)
		: EditorWindow(_name, _isToggleable)
	{

	}

	void SeamCarver::OnEnter()
	{

	}

	void SeamCarver::OnUpdate()
	{
		ImGui::Begin(name.c_str());
		AddSpace(2);

		static float width = 0, height = 0;

		ImGui::RadioButton("Carve to Size", &carveSelected, CARVE_TO_SIZE);
		AddSpace(1);

		if (carveSelected != CARVE_TO_SIZE)
			ImGui::BeginDisabled();

		ImGui::SliderFloat("Target Width", &width, 0.f, 1920.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		ImGui::SliderFloat("Target Height", &height, 0.f, 1080.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);

		if (ImGui::BeginCombo("Algorithm", modes[modeSelected]))
		{
			for (size_t i = 0; i < modes.size(); ++i)
				if (ImGui::Selectable(modes[i], i == modeSelected))
					modeSelected = i;

			ImGui::EndCombo();
		}

		if (carveSelected != CARVE_TO_SIZE)
			ImGui::EndDisabled();

		AddSpace(2);
		ImGui::RadioButton("Object Removal", &carveSelected, OBJECT_REMOVAL);
		AddSpace(1);

		if (carveSelected != OBJECT_REMOVAL)
			ImGui::BeginDisabled();

		ImGui::Button("Test");

		if (carveSelected != OBJECT_REMOVAL)
			ImGui::EndDisabled();

		AddSpace(2);
		ImGui::Separator();
		AddSpace(2);

		if (ImGui::Button("Reset"))
		{
			imgClone = originalImg.clone();
			brushMask = cv::Mat::zeros(originalImg.size(), CV_8UC1);
		}

		ImGui::SameLine();

		if (ImGui::Button("Carve"))
		{
			switch (carveSelected)
			{
			case CARVE_TO_SIZE:
				switch (modeSelected)
				{
				case GREEDY:
					VerticalSeamCarvingGreedy(imgClone, width);
					HorizontalSeamCarvingGreedy(imgClone, height);
					break;

				case DYNAMIC:
					VerticalSeamCarvingDP(imgClone, width);
					HorizontalSeamCarvingDP(imgClone, height);
					break;

				case GRAPH:
					VerticalSeamCarvingGraphCut(imgClone, width);
					HorizontalSeamCarvingGraphCut(imgClone, height);
					break;
				}
				break;

			case OBJECT_REMOVAL:
				
				ContentAwareRemoval(imgClone);
				break;
			}
		}

		ImGui::End();
	}

	void SeamCarver::OnExit()
	{

	}

	WindowsManager::WindowsManager(const std::string &_name, bool _isToggleable)
		: EditorWindow(_name, _isToggleable)
	{

	}

	void WindowsManager::OnEnter()
	{

	}

	void WindowsManager::OnUpdate()
	{
		ImGui::Begin(name.c_str());
		AddSpace(2);

		ImGui::Checkbox("Show Original Image", &shldOpenOriginalImage);
		ImGui::Checkbox("Show Carved Image", &shldOpenCarvedImage);
		ImGui::Checkbox("Show All Seams", &shldOpenAllSeams);
		ImGui::Checkbox("Show Energy Map", &shldOpenEnergyMap);
		ImGui::Checkbox("Show Energy Graph", &shldOpenEnergyGraph);

		AddSpace(2);
		ImGui::InputFloat("Scale", &scale, 100.f, 1000.f, "%.2f");

		ImGui::SameLine();
		StyleWrap(ImGuiCol_Text, LIGHT_BLUE, IconWrap(ImGui::Text(ICON_FA_INFO_CIRCLE);))
		ImGui::SetItemTooltip("Set the width of all windows to this value. The resultant heights will be calculated from the resolution.");
		ImGui::Text("Resolution: %d * %d", cols, rows);

		ImGui::End();
	}

	void WindowsManager::OnExit()
	{

	}

	void Editor::Init()
	{
		// Register window class
		const wchar_t *className = L"Inspector";
		WNDCLASS wc = { };
		wc.lpfnWndProc = WindowProc;
		wc.hInstance = GetModuleHandle(nullptr);
		wc.lpszClassName = className;
		RegisterClass(&wc);
		SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

		hwnd = CreateWindowEx(
			0,
			className,
			L"Inspector",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT, CW_USEDEFAULT,
			1200, 900, // Increase logical window size for high resolution
			nullptr, nullptr, wc.hInstance, nullptr
		);

		// Set the OpenGL context
		PIXELFORMATDESCRIPTOR pfd = { };
		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.cAccumBits = 64; // Accumulation buffer bits for high-quality rendering
		pfd.dwFlags |= PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_SWAP_COPY | PFD_SUPPORT_COMPOSITION;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.cRedBits = 8;
		pfd.cGreenBits = 8;
		pfd.cBlueBits = 8;
		pfd.cAlphaBits = 8;
		pfd.cDepthBits = 24;   // Depth buffer precision
		pfd.iLayerType = PFD_MAIN_PLANE;

		hdc = GetDC(hwnd);
		int pixelFormat = ChoosePixelFormat(hdc, &pfd);
		SetPixelFormat(hdc, pixelFormat, &pfd);

		// Create OpenGL rendering context
		HGLRC hglrc = wglCreateContext(hdc);
		wglMakeCurrent(hdc, hglrc);

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		Configure();

		ImGui_ImplWin32_Init(hwnd);
		ImGui::SetCurrentContext(ImGui::GetCurrentContext());
		ImGui_ImplOpenGL3_Init("#version 330 core");

		AddWindow<ImageLoader>(true, true);
		AddWindow<SeamCarver>(true, true);
		AddWindow<WindowsManager>(true, true);
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
		// Process all pending Windows messages
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// Clear the screen
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
		static constexpr ImWchar iconRanges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };  // Load Font Awesome icons in this range
		textFont = io.Fonts->AddFontFromFileTTF("assets/fonts/Quicksand-Regular.ttf", 27.f);
		iconFont = io.Fonts->AddFontFromFileTTF("assets/fonts/Font Awesome 5 Free-Solid-900.otf", 24.f, nullptr, iconRanges);
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