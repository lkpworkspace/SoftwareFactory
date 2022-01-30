#include "SFEditor.hpp"
#include "SFEPanelMainMenu.hpp"
#include "SFEPanelBp.hpp"
#include "SFEPanelLib.hpp"
#include "SFEPanelDragTip.hpp"
#include "SFEPanelGraph.hpp"
#include "SFEPanelLog.hpp"
#include "SFEPanelUINodes.hpp"
#include "SFEPanelPlot.hpp"
#include "bpcommon.hpp"
#include "Bp.hpp"

namespace sfe {

bool SFEditor::Init() {
    _panels.emplace_back(std::make_shared<SFEPanelUINodes>());
    _panels.emplace_back(std::make_shared<SFEPanelMainMenu>());
    _panels.emplace_back(std::make_shared<SFEPanelBp>());
    _panels.emplace_back(std::make_shared<SFEPanelLib>());
    _panels.emplace_back(std::make_shared<SFEPanelDragTip>());
    _panels.emplace_back(std::make_shared<SFEPanelGraph>());
    _panels.emplace_back(std::make_shared<SFEPanelLog>());
    _panels.emplace_back(std::make_shared<SFEPanelPlot>());

    for (auto it = _panels.begin(); it != _panels.end(); ++it) {
        (*it)->Init();
    }
    return true;
}

void SFEditor::ProcEvent(const SDL_Event event) {
    for (auto it = _panels.begin(); it != _panels.end(); ++it) {
        (*it)->ProcEvent(event);
    }
}

void SFEditor::Update() {
    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! 
    // You can browse its code to learn more about Dear ImGui!).
    if (_show_demo) {
        ImGui::ShowDemoWindow(&_show_demo);
    }

    for (auto it = _panels.begin(); it != _panels.end(); ++it) {
        (*it)->Update();
    }
}

void SFEditor::DispatchMessage() {
    // dispatch this message
    for (int j = 0; j < _send_que.size(); ++j) {
        // boardcast
        if (_send_que[j].dst == "all") {
            ProcEditorMessage(_send_que[j]);
            for (int k = 0; k < _panels.size(); ++k) {
                if (_panels[k]->PanelName() == _send_que[j].src) {
                    continue;
                }
                _panels[k]->RecvMessage(_send_que[j]);
            }
            continue;
        }
        auto panel = GetPanel(_send_que[j].dst);
        if (panel == nullptr) {
            LOG(ERROR) << "Can't find panel, msg: " << _send_que[j].Print();
            continue;
        }
        panel->RecvMessage(_send_que[j]);
    }
    _send_que.clear();
    // dispatch panel message
    for (auto it = _panels.begin(); it != _panels.end(); ++it) {
        auto msgs = (*it)->GetDispatchMessage();
        for (int j = 0; j < msgs.size(); ++j) {
            if (msgs[j].dst == "editor") {
                ProcEditorMessage(msgs[j]);
                continue;
            }
            // boardcast
            if (msgs[j].dst == "all") {
                ProcEditorMessage(msgs[j]);
                for (int k = 0; k < _panels.size(); ++k) {
                    if (_panels[k]->PanelName() == msgs[j].src) {
                        continue;
                    }
                    _panels[k]->RecvMessage(msgs[j]);
                }
                continue;
            }
            auto panel = GetPanel(msgs[j].dst);
            if (panel == nullptr) {
                LOG(ERROR) << "Can't find panel, msg: " << msgs[j].Print();
                continue;
            }
            panel->RecvMessage(msgs[j]);
        }
        (*it)->ClearDispathMessage();
    }
}

void SFEditor::ProcMessage() {
    for (auto it = _panels.begin(); it != _panels.end(); ++it) {
        (*it)->ProcMessage();
    }
}

void SFEditor::Exit() {
    for (auto it = _panels.begin(); it != _panels.end(); ++it) {
        (*it)->Exit();
    }
}

const std::shared_ptr<SFEPanel> SFEditor::GetPanel(const std::string& name) {
    for (auto it = _panels.begin(); it != _panels.end(); ++it) {
        if ((*it)->PanelName() == name) {
            return (*it);
        }
    }
    return nullptr;
}

void SFEditor::ProcEditorMessage(const SFEMessage& msg) {
    if (msg.msg == "show_demo") {
        _show_demo = !_show_demo;
    }
    if (msg.msg.empty()) {
        auto jmsg = msg.json_msg;
        auto cmd = jmsg["command"];
        if (cmd == "create_new") {
            auto graph_name = jmsg["graph_name"].asString();
            auto graph_type = jmsg["graph_type"].asString() == "mod graph" ? bp::BpNodeType::BP_GRAPH : bp::BpNodeType::BP_GRAPH_EXEC;
            if (bp::Bp::Instance().HasEditGraph(graph_name)) {
                UILog(graph_name + " graph has exist", UILogLv::WARNING);
                return;
            }
            auto g = std::dynamic_pointer_cast<bp::BpGraph>(bp::Bp::Instance().SpawnNode(graph_name, graph_type));
            bp::Bp::Instance().AddEditGraph(graph_name, g);
            bp::Bp::Instance().SetCurEditGraph(g);
            // send to graph
            Json::Value v;
            v["command"] = "set_cur_graph";
            v["graph_name"] = graph_name;
            SendMessage({_name, "graph", "", v});
        } else if (cmd == "spawn_node") {
            auto obj_type = bp::BpNodeType::BP_NONE;
            int contents_type = jmsg["type"].asInt();
            if (contents_type == (int)BpContents::LeafType::EV) {
                obj_type = bp::BpNodeType::BP_NODE_EV;
            } else if (contents_type == (int)BpContents::LeafType::FUNC) {
                obj_type = bp::BpNodeType::BP_NODE_FUNC;
            } else if (contents_type == (int)BpContents::LeafType::VAL) {
                obj_type = bp::BpNodeType::BP_NODE_VAR;
            } else if (contents_type == (int)BpContents::LeafType::BASE) {
                obj_type = bp::BpNodeType::BP_NODE_BASE;
            } else if (contents_type == (int)BpContents::LeafType::USER) {
                obj_type = bp::BpNodeType::BP_NODE_USER;
            } else if (contents_type == (int)BpContents::LeafType::GRAPH) {
                obj_type = bp::BpNodeType::BP_GRAPH;
            }
            if (obj_type == bp::BpNodeType::BP_NONE) {
                LOG(ERROR) << "obj_type is none, return";
                return;
            }
            std::shared_ptr<bp::BpNode> node = nullptr;
            auto g = bp::Bp::Instance().CurEditGraph();
            if (obj_type == bp::BpNodeType::BP_GRAPH && g == nullptr) {
                node = bp::Bp::Instance().SpawnNode(jmsg["node_name"].asString(), obj_type);
                auto new_graph = std::dynamic_pointer_cast<bp::BpGraph>(node);
                bp::Bp::Instance().AddEditGraph(jmsg["node_name"].asString(), new_graph);
                bp::Bp::Instance().SetCurEditGraph(new_graph);
                Json::Value v;
                v["command"] = "set_nodes_pos";
                v["desc"] = bp::BpCommon::Json2Str(new_graph->GetNodesPos());
                SendMessage({_name, "bp editor", "", v});
                return;
            } else if (g == nullptr) {
                LOG(WARNING) << "cur edit graph is nullptr";
                return;
            }
            if (obj_type == bp::BpNodeType::BP_NODE_VAR) {
                if (jmsg["node_name"].asString().empty()) {
                    node = bp::Bp::Instance().SpawnVarNode(g, 
                                jmsg["var_name"].asString(),
                                jmsg["is_get"].asBool());
                } else {
                    node = bp::Bp::Instance().SpawnVarNode(g, 
                                jmsg["node_name"].asString(),
                                jmsg["var_name"].asString(),
                                jmsg["is_get"].asBool());
                }
            } else {
                node = bp::Bp::Instance().SpawnNode(jmsg["node_name"].asString(), obj_type);
            }
            if (node == nullptr) {
                LOG(WARNING) << "SpawnNode failed";
                return;
            }
            if (node->GetNodeType() == bp::BpNodeType::BP_NODE_EV) {
                LOG(INFO) << "Create ev node " << node->GetName();
                g->AddEventNode(node);
            } else {
                LOG(INFO) << "Create node " << node->GetName();
                g->AddNode(node);
            }
            // 发送给bp editor消息,设置node的坐标
            Json::Value v;
            v["command"] = "set_node_pos";
            v["node_id"] = node->GetID();
            v["x"] = jmsg["x"];
            v["y"] = jmsg["y"];
            SendMessage({_name, "bp editor", "", v});
        } else if (cmd == "open_graph") {
            auto path = jmsg["path"].asString();
            std::shared_ptr<bp::BpGraph> g = nullptr;
            bp::LoadSaveState state = bp::LoadSaveState::OK;
            Json::Value nodes_pos;
            if (bp::LoadSaveState::OK == (state = bp::Bp::Instance().LoadGraph(path, g, nodes_pos))) {
                bp::Bp::Instance().AddEditGraph(g->GetName(), g);
                bp::Bp::Instance().SetCurEditGraph(g);
                // set nodes pos
                Json::Value msg2;
                msg2["command"] = "set_nodes_pos";
                msg2["desc"] = bp::BpCommon::Json2Str(nodes_pos);;
                SendMessage({_name, "bp editor", "", msg2});
            } else {
                LOG(ERROR) << "Load graph " << path << " failed, " << (int)state;
            }
        } else if (cmd == "save_graph_step2") {
            auto path = jmsg["path"].asString();
            auto g = bp::Bp::Instance().CurEditGraph();
            if (g == nullptr) {
                LOG(WARNING) << "cur edit graph is nullptr";
                return;
            }
            bp::LoadSaveState state = bp::LoadSaveState::OK;

            auto nodes_desc = bp::BpCommon::Str2Json(jmsg["nodes_pos"].asString());
            if (nodes_desc == Json::Value::null) {
                LOG(ERROR) << "parse nodes desc failed";
                return;
            }
            if (bp::LoadSaveState::OK != (state = bp::Bp::Instance().SaveGraph(path, g, nodes_desc))) {
                LOG(ERROR) << "Save graph " << path << " failed, " << (int)state;
            }
        } else if (cmd == "switch_graph") {
            bp::Bp::Instance().SetCurEditGraph(jmsg["name"].asString());
            auto g = bp::Bp::Instance().CurEditGraph();
            Json::Value v;
            v["command"] = "set_nodes_pos";
            v["desc"] = bp::BpCommon::Json2Str(g->GetNodesPos());
            SendMessage({_name, "bp editor", "", v});
        } else if (cmd == "del_node") {
            auto g = bp::Bp::Instance().CurEditGraph();
            g->DelNode(jmsg["id"].asInt());
        } else if (cmd == "del_evnode") {
            auto g = bp::Bp::Instance().CurEditGraph();
            g->DelEventNode(jmsg["name"].asString());
        } else if (cmd == "del_link") {
            auto g = bp::Bp::Instance().CurEditGraph();
            g->DelLink(jmsg["id"].asInt());
        } else if (cmd == "create_link") {
            auto g = bp::Bp::Instance().CurEditGraph();
            auto start_pin = g->SearchPin(jmsg["start_pin_id"].asInt());
            auto end_pin = g->SearchPin(jmsg["end_pin_id"].asInt());
            g->AddLink(*start_pin, *end_pin);
            g->GetLinks().back().SetColor(jmsg["color"][0].asFloat(), jmsg["color"][1].asFloat(), jmsg["color"][2].asFloat(), jmsg["color"][3].asFloat());
        }
    }
}

} // namespace sfe