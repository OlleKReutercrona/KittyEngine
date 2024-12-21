#pragma once
#include <unordered_map>

struct ImDrawList;
struct ImVec2;

namespace KE_EDITOR
{
	struct ConsoleMessageData
	{
		std::string file;
		std::string function;
		unsigned int line;
	};

	struct EditorConsoleMessage
	{
		ConsoleMessageData data;

		std::string channel;
		std::string message;

		int logCount = 1;
	};

	class EditorWindowBase;

	class ConsoleInternal
	{
		friend class EditorConsole;
		private:
			std::vector<EditorConsoleMessage> myMessageHistory;
			std::vector<std::pair<size_t, float>> myMessagePopups;
			std::unordered_map<std::string, char> myChannelFlags;
			std::unordered_map<std::string, unsigned int> myChannelColours;
			constexpr static float popupDuration = 5.0f;
		public:
			ConsoleInternal();
			void ClearMessages() { myMessageHistory.clear(); myMessagePopups.clear(); };
			inline const std::vector<EditorConsoleMessage>& GetMessages() const { return myMessageHistory; }

			void LogMessage(
				const char* aChannel,
				const char* aFile,
				unsigned int aLine,
				const char* aFunction,
				const char* aMessage,
				...
			);
	};

	class EditorConsole : public EditorWindowBase
	{
		KE_EDITOR_FRIEND
	private:

	public:

		EditorConsole(EditorWindowInput aStartupData = {}) : EditorWindowBase(aStartupData) {}
		~EditorConsole() override = default;

		const char* GetWindowName() const override { return "Console"; }
		void Init() override;
		void Update() override;
		void Render() override;

		//void DumpToFile(const std::string& aFilePath);
		void RenderPopups(const ImVec2&	aPosition, ImDrawList* aDrawList);

	};
}
