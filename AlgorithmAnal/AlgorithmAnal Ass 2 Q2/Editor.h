#include <string>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <array>

#include "Utility.h"

/*! ------------ Editor Windows ------------ */

#undef LoadImage

namespace edit
{

	enum CarveMode
	{
		CARVE_TO_SIZE,
		OBJECT_REMOVAL,
		MAX_CARVE
	};

	enum AlgoMode
	{
		GREEDY,
		DYNAMIC,
		GRAPH,
		MAX_ALGO
	};

	enum ExtMode
	{
		PNG,
		JPG,
		MAX_EXT
	};

	class EditorWindow
	{
	protected:

		std::string name;
		bool isOpen = false;
		bool isToggleable = true;

	public:

		EditorWindow(const std::string &_name = "", bool _isToggleable = true);
		virtual ~EditorWindow();

		virtual void OnEnter() = 0;
		virtual void OnUpdate() = 0;
		virtual void OnExit() = 0;

		void Open();
		void Close();
		bool IsOpen();
		bool IsToggleable();
	};

	class ImageLoader : public EditorWindow
	{
		size_t selected = 0;
		std::string loadedFile = "No file selected";

		size_t ext = PNG;
		int compression = 100;
		std::string newFileName;

		const std::array<const char *, MAX_EXT> exts =
		{
			"png",
			"jpg",
		};

	public:

		ImageLoader(const std::string &_name = "", bool _isToggleable = true);

		void OnEnter() override;
		void OnUpdate() override;
		void OnExit() override;

		void LoadImage();
		void UnloadImage();
		void ReloadImage();

		bool isFileLoaded = false;
	};

	class SeamCarver : public EditorWindow
	{
		int width = 1.f, height = 1.f;

		const std::array<const char *, MAX_ALGO> modes =
		{
			"Greedy",
			"Dynamic programming",
			"Graph cut"
		};

	public:

		SeamCarver(const std::string &_name = "", bool _isToggleable = true);

		void OnEnter() override;
		void OnUpdate() override;
		void OnExit() override;

		int carveSelected = 0;
		size_t modeSelected = 0;
	};

	class WindowsManager : public EditorWindow
	{
	public:

		WindowsManager(const std::string &_name = "", bool _isToggleable = true);

		void OnEnter() override;
		void OnUpdate() override;
		void OnExit() override;

		bool shldOpenOriginalImage = true;
		bool shldOpenAllSeams = true;
		bool shldOpenCarvedImage = true;
		bool shldOpenEnergyMap = true;
		bool shldOpenEnergyGraph = true;

		float scale = 1000.f;
	};

	/*! ------------ Editor Windows Manager ------------ */

	class Editor
	{
		std::unordered_map<std::string, std::unique_ptr<EditorWindow>> windows;

	public:

		void Init();
		void Update();
		void Shutdown();

		void OpenWindow(const std::string &windowName);
		void CloseWindow(const std::string &windowName);
		void ToggleWindow(const std::string &windowName);
		const std::unordered_map<std::string, std::unique_ptr<EditorWindow>> &GetWindows();
		void Configure();

		template <typename T>
		void AddWindow(bool shldOpen, bool isToggleable)
		{
			std::string name = typeid(T).name();
			name = util::ToCapitalCase(util::TrimString(name, "::", false));
			windows[name] = std::make_unique<T>(name, isToggleable);

			if (shldOpen)
				OpenWindow(name);
		}

		template <typename T>
		T *GetWindow()
		{
			std::string name = typeid(T).name();
			name = util::ToCapitalCase(util::TrimString(name, "::", false));
			return dynamic_cast<T *>(windows.at(name).get());
		}
	};

}

void initializeBrushMask(const cv::Mat& img);
void drawBrush(cv::Mat& img, cv::Point point);
void mouseCallback(int event, int x, int y, int flags, void* data);