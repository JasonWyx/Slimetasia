#include "AppConsole.h"

#include <fstream>
#include <iostream>
#include <limits>

void AppConsole::Draw()
{
    // SetNextWindowSize(ImVec2((GetIO().DisplaySize.x) / 4 * 3, 360.f));
    // SetNextWindowPos(ImVec2((float)(0.f), GetIO().DisplaySize.y - 360.f));

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    if (m_IsActiveWindow)
    {
        if (ImGui::BeginTabItem("Console", &m_IsActiveWindow))
        {
            if (ImGui::Button("Trigger Auto Scroll"))
            {
                m_TriggerAutoScroll = !m_TriggerAutoScroll;
            }

            ImGui::SameLine();

            if (ImGui::Button("Clear Log "))
            {
                ClearLog();
            }

            ImGui::SameLine();

            if (ImGui::Button("Output Log "))
            {
                OutLog();
            }

            ImGui::SameLine();
            ImGui::PushItemWidth(60.f);
            ImGui::Text("Console's Max Lines : ");
            ImGui::SameLine();
            ImGui::InputScalar("", ImGuiDataType_U32, &m_MaxLines);
            ImGui::PopItemWidth();

            ImGui::Separator();

            const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();

            ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);
            for (unsigned i = 0; i < m_Items.size(); i++)
            {
                if (m_Items[i] == "") continue;
                std::string item = m_Items[i];
                ImVec4 col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                ImGui::PushStyleColor(ImGuiCol_Text, col);
                ImGui::TextUnformatted((item).c_str());
                ImGui::PopStyleColor();
            }
            if (m_ScrollToBtm && m_TriggerAutoScroll)
            {
                ImGui::SetScrollHereY(1.0f);
            }

            m_ScrollToBtm = false;

            ImGui::EndChild();
            ImGui::EndTabItem();
        }
    }
}

void AppConsole::SetMaxLines(unsigned lines)
{
    m_MaxLines = lines;
}

void AppConsole::AddLog(const char* log)
{
    if (log != std::string(""))
    {
        if (m_Items.size() == m_MaxLines) m_Items.pop_front();
        m_Items.push_back(std::string(log));
        m_ScrollToBtm = true;
    }
}

void AppConsole::OutLog()
{
    std::ofstream output;
    output.open("OutputLog.txt");
    for (auto l : m_Items)
    {
        output << l;
    }
    output.close();
}

void AppConsole::ClearLog()
{
    m_Items.clear();
    m_Items.resize(m_MaxLines);
}
