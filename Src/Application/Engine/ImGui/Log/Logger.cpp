#include "Logger.h"

static ImGuiTextBuffer logBuffer;
static ImVector<int> lineOffsets;
static bool scrollToBottom = false;

void EngineCore::Logger::Log(const std::string& category, const std::string& msg)
{
	std::lock_guard<std::mutex> lock(s_mutex);
	std::cout << "[" << category << "]" << msg << std::endl;
	s_logs.push_back({ category, GetTimestamp() + " " + msg });
}

void EngineCore::Logger::Error(const std::string& msg)
{
	Log("Error", msg);
}

void EngineCore::Logger::DrawImGui()
{

	std::vector<LogEntry>logsCopy;
	{
		std::lock_guard<std::mutex> lock(s_mutex);
		logsCopy = s_logs;
	}
	
	if (ImGui::Begin("Log Window"))
	{
		if (ImGui::Button("Clear"))
		{
			std::lock_guard<std::mutex>lock(s_mutex);
			s_logs.clear();
			return;
		}
		std::set<std::string> categories;
		for (const auto& log : logsCopy)
		{
			categories.insert(log.category);
		}
		if (ImGui::BeginTabBar("LogTabs"))
		{
			for (const auto& cat : categories)
			{
				if (ImGui::BeginTabItem(cat.c_str()))
				{
					if (ImGui::BeginChild((cat + "_scroll").c_str(), ImVec2(0, 0), true))
					{
						for (const auto& log : logsCopy)
						{
							if (log.category == cat)
							{
								ImGui::TextWrapped("%s", log.message.c_str());
							}
						}
					}
					ImGui::EndChild();
				}
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	ImGui::End();
}

void EngineCore::Logger::DrawAddLog()
{
	if (ImGui::Begin("AddLog"))
	{
		if (ImGui::Button("Clear"))
		{
			logBuffer.clear();
			lineOffsets.clear();
			lineOffsets.push_back(0);
		}

		ImGui::Separator();
		ImGui::BeginChild("scrolling");

		const char* buf		= logBuffer.begin();
		const char* buf_end = logBuffer.end();

		ImGuiListClipper clipper;
		clipper.Begin(lineOffsets.size());
		while (clipper.Step())
		{
			for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
			{
				const char* line_start = buf + lineOffsets[line_no];
				const char* line_end = (line_no + 1 < lineOffsets.Size) ? (buf + lineOffsets[line_no + 1] - 1) : buf_end;
				ImGui::TextUnformatted(line_start, line_end);
			}
		}
		clipper.End();

		if (scrollToBottom)
		{
			ImGui::SetScrollHereY(1.0f);
			scrollToBottom = false;
		}
		ImGui::EndChild();
	}
	ImGui::End();
}

void EngineCore::Logger::Add(const char* fmt, ...)
{
	static char buffer[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	int old_size = logBuffer.size();
	logBuffer.append(buffer);
	for (int i = old_size; i < logBuffer.size(); i++) {
		if (logBuffer[i] == '\n')
			lineOffsets.push_back(i + 1);
	}
	scrollToBottom = true;
}
