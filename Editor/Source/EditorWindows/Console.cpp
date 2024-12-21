#include "stdafx.h"

#ifndef KITTYENGINE_NO_EDITOR
#include "Console.h"
#include "Editor/Source/ImGui/ImGuiHandler.h"

KE_EDITOR::ConsoleInternal::ConsoleInternal()
{
	myMessageHistory.reserve(128);
	myMessagePopups.reserve(16);

	myChannelFlags["log"] = 1;
	myChannelFlags["warning"] = 1;
	myChannelFlags["error"] = 1;

	myChannelColours["log"] = ImColor(1.0f,1.0f,1.0f,0.75f);
	myChannelColours["warning"] = ImColor(1.0f, 1.0f, 0.0f, 0.75f);
	myChannelColours["error"] = ImColor(1.0f,0.0f,0.0f, 0.75f);
}

void KE_EDITOR::ConsoleInternal::LogMessage(
	const char* aChannel,
	const char* aFile,
	unsigned int aLine,
	const char* aFunction,
	const char* aMessage,
	...
)
{
	if (!this) { return; }

	char buffer[1024];
	va_list args;
	va_start(args, aMessage);
	vsprintf_s(buffer, aMessage, args);
	va_end(args);

	std::string message = buffer;
	message = message.substr(0, message.find_last_not_of("\n") + 1);

	std::string file = aFile;
	file = file.substr(file.find_last_of("\\") + 1);

	std::string function = aFunction;
	function = function.substr(0, function.find_last_of(")") + 1);

	std::string c = aChannel;

	if (!myMessageHistory.empty())
	{

		auto& lastMSG = myMessageHistory.back();
		if (
			lastMSG.channel == c &&
			lastMSG.message == message &&
			lastMSG.data.file == file &&
			lastMSG.data.function == function &&
			lastMSG.data.line == aLine
			) // if the last message was the same, just increment the log count
		{
			lastMSG.logCount++;
			return;
		}
	}


	if (!myChannelFlags.contains(c))
	{
		myChannelFlags[c] = 0;
		myChannelColours[c] = ImColor(1.0f, 1.0f, 1.0f, 1.0f);
	}

	auto& m = myMessageHistory.emplace_back();
	m.message = message;
	m.channel = aChannel;

	m.data.file = file;
	m.data.function = function;
	m.data.line = aLine;

	if (myChannelFlags[c])
	{
		myMessagePopups.push_back(std::pair<int, float>(static_cast<int>(myMessageHistory.size() - 1), popupDuration));
	}
}

void KE_EDITOR::EditorConsole::Init()
{
}

void KE_EDITOR::EditorConsole::Update()
{
	
	auto& console = KE_GLOBAL::console;

	for (int i = 0; i < console.myMessagePopups.size(); i++)
	{
		auto& message = console.myMessagePopups[i];
		if (message.second > 0.0f)
		{
			message.second -= KE_GLOBAL::deltaTime;
		}
		else
		{
			console.myMessagePopups.erase(console.myMessagePopups.begin() + i);
			i--;
		}
	}
}

void KE_EDITOR::EditorConsole::Render()
{
	auto& console = KE_GLOBAL::console;

	if (ImGui::Checkbox("Log", (bool*)&console.myChannelFlags["log"])) {}  ImGui::SameLine();
	if (ImGui::Checkbox("Warning", (bool*)&console.myChannelFlags["warning"])) {} ImGui::SameLine();
	if (ImGui::Checkbox("Error", (bool*)&console.myChannelFlags["error"])) {} ImGui::SameLine();
	if (ImGui::BeginMenu("Other"))
	{
		for (auto& channel : console.myChannelFlags)
		{
			if (channel.first != "log" && channel.first != "warning" && channel.first != "error")
			{
				if (ImGui::Checkbox(channel.first.c_str(), (bool*)&channel.second)) {}
			}
		}
		ImGui::EndMenu();
	}
	ImGui::Separator();

	ImGui::PushStyleColor(ImGuiCol_ChildBg, (ImU32)ImColor(0.0f, 0.0f, 0.0f, 0.5f));
	if (ImGui::BeginChild("Message Log", ImGui::GetContentRegionAvail() - ImVec2(0.0f, 25.0f)))
	{
		ImGui::PopStyleColor();
		int i = 0;
		for (auto& message : console.myMessageHistory)
		{
			if (console.myChannelFlags[message.channel] == 0) { continue; }
			ImGui::Dummy(ImVec2(0.0f, 3.0f));

			ImGui::PushID(i++);

			if (ImGui::BeginChild("ConsoleChild", ImVec2(0.0f, 0.0f), ImGuiChildFlags_AutoResizeY))
			{
				bool b = ImGui::TreeNode("Details:"); ImGui::SameLine(); ImGui::TextWrapped("(%d)", message.logCount);
				if (b)
				{
					ImGui::TextWrapped("Function: %s", message.data.function.c_str());
					ImGui::TextWrapped("File: %s", message.data.file.c_str());
					ImGui::TextWrapped("Line: %d", message.data.line);
					ImGui::TreePop();
				}

				ImGui::PushStyleColor(ImGuiCol_Text, console.myChannelColours[message.channel]);
				ImGui::TextWrapped("%s", message.message.c_str());
				ImGui::PopStyleColor();

			}
			ImGui::EndChild();

			ImGui::GetWindowDrawList()->AddRect(
				ImGui::GetItemRectMin() + ImVec2(0.0f, -4.0f),
				ImGui::GetItemRectMax() + ImVec2(0.0f, 4.0f),
				console.myChannelColours[message.channel],
				5.0f,
				0,
				2.0f
			);

			ImGui::PopID();
		}
	}
	else { ImGui::PopStyleColor(); }
	ImGui::EndChild();

	char buf[128] = { 0 };
	if (ImGui::InputTextWithHint("##ConsoleInput", "Console Command", buf, 128, ImGuiInputTextFlags_EnterReturnsTrue))
	{
		std::string inputString = buf;
		std::string firstWord;
		std::string rest;

		std::istringstream iss(inputString);
		iss >> firstWord;  // Extract the first word
		std::getline(iss, rest);  // Get the rest of the string

		if (firstWord == "log")
		{
			KE_LOG(rest.c_str());
		}
		else if (firstWord == "warning")
		{
			KE_WARNING(rest.c_str());
		}
		else if (firstWord == "error")
		{
			KE_ERROR(rest.c_str());
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear Log")) { console.myMessageHistory.clear(); console.myMessagePopups.clear(); }

}

void KE_EDITOR::EditorConsole::RenderPopups(const ImVec2& aPosition, ImDrawList* aDrawList)
{
	auto& console = KE_GLOBAL::console;

	ImGui::SetCursorScreenPos(aPosition);
	for (int i = 0; i < console.myMessagePopups.size(); i++)
	{
		auto& message = console.myMessagePopups[i];
		auto& ogMessage = console.myMessageHistory[message.first];
		const char* text = ogMessage.message.c_str();

		ImGui::PushStyleColor(ImGuiCol_Text, console.myChannelColours[ogMessage.channel]);
		ImVec2 pos = ImGui::GetCursorPos();
		ImGui::PushTextWrapPos(256.0f);
		ImGui::Text(text);
		ImVec2 min = ImGui::GetItemRectMin();
		ImVec2 max = ImGui::GetItemRectMax();


		bool inside = ImRect(min, max).Contains(ImGui::GetIO().MousePos);
		aDrawList->AddRectFilled(
			min + ImVec2(-4.0f,-2.0f),
			max + ImVec2(8.0f,4.0f),
			inside ? (ImU32)ImColor(0.25f, 0.25f, 0.25f, 0.75f) : (ImU32)ImColor(0.0f, 0.0f, 0.0f, 0.75f),
			5.0f,
			ImDrawFlags_RoundCornersRight
		);
		if (inside && ImGui::IsMouseClicked(ImGuiMouseButton_Left,false)) {}

		ImGui::SetCursorPos(pos);
		ImGui::Text(text);
		ImGui::PopTextWrapPos();
		ImGui::PopStyleColor();
		ImGui::Dummy(ImVec2(0.0f, 1.0f));
	}
}
#endif
