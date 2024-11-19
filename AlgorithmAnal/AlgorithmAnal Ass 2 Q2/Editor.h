#include "Utility2.h"

/*! ------------ Editor Windows ------------ */

namespace editor
{

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