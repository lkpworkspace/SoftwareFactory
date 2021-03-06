#include "SFEPanelMainMenu.hpp"
#include "nfd.h"
#include "bpcommon.hpp"

namespace sfe {

bool SFEPanelMainMenu::Init() {
    SetDisplayInMenu(false);
    return true;
}

void SFEPanelMainMenu::Update() {
    static bool create_new = false;
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", "")) {
                create_new = true;
            }
            if (ImGui::MenuItem("Open...", "")) {
                OpenGraph();
            }
            // ImGui::Separator();
            // if (ImGui::MenuItem("Export...", "")) {}
            ImGui::Separator();
            // if (ImGui::MenuItem("Save...", "CTRL+S")) {
            //     LOG(INFO) << "Save...";
            // }
            if (ImGui::MenuItem("Save as...", "CTRL+SHIFT+S")) {
                SaveGraph();
            }
            // ImGui::Separator();
            // if (ImGui::MenuItem("Exit", "CTRL+Q")) {}
            ImGui::EndMenu();
        }
        // if (ImGui::BeginMenu("Edit")) {
        //     if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
        //     if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
        //     ImGui::Separator();
        //     if (ImGui::MenuItem("Cut", "CTRL+X")) {}
        //     if (ImGui::MenuItem("Copy", "CTRL+C")) {}
        //     if (ImGui::MenuItem("Paste", "CTRL+V")) {}
        //     ImGui::EndMenu();
        // }
        if (ImGui::BeginMenu("Run")) {
            if (ImGui::MenuItem("Start", "F5", false, _debug_mode == 1 && !_runing)) {
                Json::Value v;
                v["command"] = "run_cur_graph";
                v["type"] = "req";
                v["run"] = true;
                SendMessage("all", v);
            }
            if (ImGui::MenuItem("Stop", "SHIFT+F5", false, _debug_mode == 1 && _runing)) {
                Json::Value v;
                v["command"] = "run_cur_graph";
                v["type"] = "req";
                v["run"] = false;
                SendMessage("all", v);
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Start Debug", "", false, _debug_mode == 1 && !_runing)) {
                Json::Value v;
                v["command"] = "debug_cur_graph";
                v["type"] = "req";
                v["stage"] = "start";
                SendMessage("all", v);
            }
            if (ImGui::MenuItem("Continue Debug", "F10", false, _debug_mode == 2 && !_runing)) {
                Json::Value v;
                v["command"] = "debug_cur_graph";
                v["type"] = "req";
                v["stage"] = "continue";
                SendMessage("all", v);
            }
            if (ImGui::MenuItem("Stop Debug", "", false, _debug_mode == 2 && !_runing)) {
                Json::Value v;
                v["command"] = "debug_cur_graph";
                v["type"] = "req";
                v["stage"] = "stop";
                SendMessage("all", v);
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Enable All Breakpoints", "", false)) {
                Json::Value v;
                v["command"] = "breakpoint_cur_graph";
                v["type"] = "req";
                v["id"] = "all";
                v["set"] = true;
                SendMessage("all", v);
            }
            if (ImGui::MenuItem("Remove All Breakpoints", "", false)) {
                Json::Value v;
                v["command"] = "breakpoint_cur_graph";
                v["type"] = "req";
                v["id"] = "all";
                v["set"] = false;
                SendMessage("all", v);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("ShowDemo")) {
                SendMessage("editor", std::string("show_demo"));
            }
            auto& panels = SFEPanel::GetPanels();
            for (auto& panel : panels) {
                if (!panel.second->DisplayInMenu()) {
                    continue;
                }
                ImGui::MenuItem(panel.first.c_str(), nullptr, &panel.second->IsShow());
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (create_new) {
        create_new = false;
        ImGui::OpenPopup("Create graph...");
    }
    CreateNewGraph();
}

void SFEPanelMainMenu::Exit() {

}

void SFEPanelMainMenu::CreateNewGraph() {
    if (ImGui::BeginPopupModal("Create graph...", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        static std::string graph_type = "exec graph";
        static int style_idx = 0;
        if (ImGui::Combo("graph type", &style_idx, "exec graph\0mod graph\0")) {
            switch (style_idx) {
            case 0: graph_type = "exec graph"; break;
            case 1: graph_type = "mod graph"; break;
            }
        }
        static char buf[64] = "";
        if (graph_type == "mod graph") {
            ImGui::InputText("graph name", buf, 64, ImGuiInputTextFlags_CharsNoBlank);
        } else {
            strcpy(buf, "__main__");
        }

        if (ImGui::Button("OK", ImVec2(120, 0))) { 
            if (!bp::BpCommon::IsName(buf, strlen(buf))) {
                LOG(WARNING) << "graph name is not availdable";
            } else {
                Json::Value v;
                v["command"] = "create_new";
                v["graph_type"] = graph_type;
                v["graph_name"] = buf;
                SendMessage("editor", v);
            }
            memset(buf, 0, sizeof(buf));
            ImGui::CloseCurrentPopup(); 
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
}

void SFEPanelMainMenu::OpenGraph() {
    nfdchar_t *out_path = NULL;
    nfdresult_t result = NFD_OpenDialog("json", NULL, &out_path);
    if (result == NFD_OKAY) {
        Json::Value v;
        v["command"] = "open_graph";
        v["path"] = out_path;
        SendMessage("editor", v);
        free(out_path);
    } else if (result == NFD_CANCEL) {
        LOG(INFO) << "Open cancel";
    } else {
        LOG(ERROR) << "Import error " << NFD_GetError();
    }
}

void SFEPanelMainMenu::SaveGraph() {
    nfdchar_t *out_path = NULL;
    nfdresult_t result = NFD_SaveDialog("", NULL, &out_path);
    if (result == NFD_OKAY) {
        LOG(INFO) << "Save as " << out_path;
        Json::Value v;
        v["command"] = "save_graph_step1";
        v["path"] = out_path;
        SendMessage("bp editor", v);
        free(out_path);
    } else if (result == NFD_CANCEL) {
        LOG(INFO) << "Save cancel";
    } else {
        LOG(ERROR) << "Save error " << NFD_GetError();
    }
}

void SFEPanelMainMenu::OnMessage(const SFEMessage& msg) {
    if (msg.json_msg.isNull()) {
        return;
    }
    auto jmsg = msg.json_msg;
    auto cmd = jmsg["command"];
    if (cmd == "run_cur_graph" && jmsg["type"].asString() == "resp") {
        bool is_run = jmsg["run"].asBool();
        _runing = is_run;
    } else if (cmd == "debug_cur_graph" && jmsg["type"].asString() == "resp") {
        if (jmsg["stage"] == "start") {
            _debug_mode = 2;
        } else if (jmsg["stage"] == "stop") {
            _debug_mode = 1;
        }
    }
}

} // namespace sfe