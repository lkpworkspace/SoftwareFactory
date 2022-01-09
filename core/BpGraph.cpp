﻿#include <fstream>

#include "Bp.hpp"
#include "BpGraph.hpp"
#include "BpPin.hpp"
#include "BpLink.hpp"

namespace bp {

BpGraph::BpGraph(std::shared_ptr<BpGraph> parent) 
	: BpObj("", parent)
	, _next_id(0) {}

BpGraph::BpGraph(std::string name, BpObjType t, std::shared_ptr<BpGraph> parent)
	: BpObj(name, parent)
	, _next_id(0) {
	_obj_type = t;
	if (t == BpObjType::BP_GRAPH) {
		// TODO 添加输入输出节点, 并对_input_node赋值
	}
}

void BpGraph::Clear() {
	_next_id = 0;
	_links.clear();
	_vars.clear();
	_nodes.clear();
	_event_nodes.clear();
}

int BpGraph::GetNextID() {
	return ++_next_id;
}

void BpGraph::SetNextID(int id) {
	// 从配置文件加载蓝图时会用到该函数设置下次node的id
	// 其他情况不要调用该函数
	_next_id = id - 1;
}

BpVariable& BpGraph::GetVariable(const std::string& name) {
	if (_vars.find(name) == _vars.end()) return _null_val;
	return _vars[name];
}

bool BpGraph::AddVariable(const std::string& name, const BpVariable& v) {
	if (_vars.find(name) == _vars.end()) {
		_vars[name] = v;
		return true;
	}
	return false;
}

bool BpGraph::ModifyVariableName(const std::string& old_name, const std::string& new_name) {
	if (_vars.find(old_name) == _vars.end()) return false;
	if (_vars.find(new_name) != _vars.end()) return false;

	_vars[new_name] = _vars[old_name];
	_vars.erase(old_name);
	return true;
}

void BpGraph::RemoveVariable(const std::string& name) {
	if (_vars.find(name) == _vars.end()) return;
	_vars.erase(name);
}

void BpGraph::AddNode(std::shared_ptr<BpObj> node) {
	node->SetParentGraph(std::dynamic_pointer_cast<BpGraph>(shared_from_this()));
	_nodes.push_back(node);
}

int BpGraph::AddLink(BpPin& start_pin, BpPin& end_pin) {
	_links.emplace_back(GetNextID(), start_pin.ID, end_pin.ID);
	start_pin.SetLinked();
	end_pin.SetLinked();
	return _links.back().ID;
}

void BpGraph::DelLink(int id) {
	for (auto it = _links.begin(); it != _links.end(); ++it) {
		if ((*it).ID == id) {
			BpPin* sp = SearchPin((*it).StartPinID);
			BpPin* ep = SearchPin((*it).EndPinID);
			sp->SetLinked(false);
			ep->SetLinked(false);
			_links.erase(it);
			break;
		}
	}
}
void BpGraph::DelNode(std::shared_ptr<BpObj> node) {
	for (auto it = _nodes.begin(); it != _nodes.end(); ++it) {
		if (*it == node) {
			_nodes.erase(it);
			break;
		}
	}
}

std::vector<BpLink> BpGraph::SearchLinks(int id) {
	std::vector<BpLink> ret;
	for (int i = 0; i < _links.size(); ++i) {
		if (_links[i].StartPinID == id || _links[i].EndPinID == id) {
			ret.push_back(_links[i]);
		}
	}
	return ret;
}

BpPin* BpGraph::SearchPin(int id) {
	for (auto p : _event_nodes) {
		auto com = p.second;
		std::vector<BpPin>& inputs = com->GetPins(BpPinKind::BP_INPUT);
		std::vector<BpPin>& outputs = com->GetPins(BpPinKind::BP_OUTPUT);
		for (BpPin& pin : inputs) {
			if (pin.ID == id)
				return &pin;
		}
		for (BpPin& pin : outputs) {
			if (pin.ID == id)
				return &pin;
		}
	}
	for (auto com : _nodes) {
		std::vector<BpPin>& inputs = com->GetPins(BpPinKind::BP_INPUT);
		std::vector<BpPin>& outputs = com->GetPins(BpPinKind::BP_OUTPUT);
		for (BpPin& pin : inputs) {
			if (pin.ID == id)
				return &pin;
		}
		for (BpPin& pin : outputs) {
			if (pin.ID == id)
				return &pin;
		}
	}
	return nullptr;
}

std::shared_ptr<BpObj> BpGraph::GetNode(int id) {
	for (auto com : _nodes) {
		if (com->GetID() == id) return com;
	}
	return nullptr;
}

bool BpGraph::AddEventNode(std::shared_ptr<BpObj> node) {
	if (_obj_type == BpObjType::BP_GRAPH) return false;
	if (_event_nodes.find(node->GetName()) == _event_nodes.end()) {
		_event_nodes[node->GetName()] = node;
		return true;
	}
	return false;
}

void BpGraph::DelEventNode(std::string name) {
	if (_event_nodes.find(name) != _event_nodes.end()) {
		auto com = _event_nodes[name];
		_event_nodes.erase(name);
	}
}

void BpGraph::ClearFlag() {
	for (auto& ev_com : _event_nodes) {
		ev_com.second->ClearFlag();
	}
	for (auto& com : _nodes) {
		com->ClearFlag();
	}
}

void BpGraph::RunEvent(std::string ev) {
	if (_event_nodes.find(ev) == _event_nodes.end()) return;
	auto com = _event_nodes[ev];
	ClearFlag();
	com->Run();
}

void BpGraph::Run() {
	if (_obj_type == BpObjType::BP_GRAPH) {
		BpObj::Run();
	} else {
		// TODO 应该按事件优先级依次执行
		// RunEvent("in");
	}
}

} // namespace bp