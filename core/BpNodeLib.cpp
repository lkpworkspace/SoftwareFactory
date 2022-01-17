#include "BpNodeLib.hpp"
#include "BpNode.hpp"
#include "BpNodeNormal.hpp"
#include "BpEvNodeTick.hpp"
#include "BpBaseNodeVar.hpp"
#include "BpBaseNodePrint.hpp"

namespace bp {
BpNodeLib::BpNodeLib() 
    : _ev_nodes {
        {"Tick", [](){ 
            auto node = std::make_shared<BpEvNodeTick>(nullptr); 
            node->AddPin("", BpPinKind::BP_OUTPUT, BpPinType::BP_FLOW, BpVariable()); 
            return node; }
        }
    },
    _base_nodes {
        {"Print", [](){ 
            auto node = std::make_shared<BpBaseNodePrint>(nullptr); 
            node->AddPin("", BpPinKind::BP_INPUT, BpPinType::BP_FLOW, BpVariable()); 
            node->AddPin("any", BpPinKind::BP_INPUT, BpPinType::BP_VALUE, BpVariable("any", "any", nullptr)); 
            return node; }
        }
    }
{
    _root_contents = std::make_shared<BpContents>(nullptr, BpContents::Type::CONTENTS, "");
    // 创建事件目录
    _ev_contents = std::make_shared<BpContents>(nullptr, BpContents::Type::CONTENTS, "event node");
    _ev_contents->AddChild(std::make_shared<BpContents>(nullptr, BpContents::Type::EV, "Tick"));
    _root_contents->AddChild(_ev_contents);
    // 创建基础节点目录
    _base_contents = std::make_shared<BpContents>(nullptr, BpContents::Type::CONTENTS, "base node");
    _base_contents->AddChild(std::make_shared<BpContents>(nullptr, BpContents::Type::BASE, "Print"));
    _root_contents->AddChild(_base_contents);
}

BpNodeLib::~BpNodeLib() {

}

std::shared_ptr<BpNode> BpNodeLib::CreateFuncNode(BpModuleFunc func_info, 
            std::vector<BpVariable>& args,
            std::vector<BpVariable>& res) {
    LOG(INFO) << "create func node " << func_info.name;
    auto node = std::make_shared<BpNodeNormal>(func_info.name, nullptr);
    node->SetFuncInfo(func_info);
    // // 创建一个输入输出flow
    node->AddPin("", BpPinKind::BP_INPUT, BpPinType::BP_FLOW, BpVariable());
    node->AddPin("", BpPinKind::BP_OUTPUT, BpPinType::BP_FLOW, BpVariable());
    // 根据函数输入参数设置输入pin
    for (int i = 0; i < func_info.type_args.size(); ++i) {
        // 根据描述创建变量
        node->AddPin(func_info.type_args[i], BpPinKind::BP_INPUT, BpPinType::BP_VALUE, args[i]);
    }
    // 根据函数输出参数设置输出pin
    for (int i = 0; i < func_info.type_res.size(); ++i) {
        node->AddPin(func_info.type_res[i], BpPinKind::BP_OUTPUT, BpPinType::BP_VALUE, res[i]);
    }
    return node;
}

/* 创建变量Node */
std::shared_ptr<BpNode> BpNodeLib::CreateVarNode(BpVariable var, bool is_get) {
    LOG(INFO) << "create var node " << var.GetName() << "(" << var.GetType() << ")";
    auto node = std::make_shared<BpBaseNodeVar>(is_get, var, nullptr);
    if (is_get) {
        node->AddPin(var.GetType(), BpPinKind::BP_OUTPUT, BpPinType::BP_VALUE, var);
    } else {
        node->AddPin(var.GetType(), BpPinKind::BP_INPUT, BpPinType::BP_VALUE, var);
    }
    return node;
}

/* 创建事件Node */
std::shared_ptr<BpNode> BpNodeLib::CreateEvNode(const std::string& name) {
    if (_ev_nodes.find(name) == _ev_nodes.end()) {
        LOG(ERROR) << "can't find ev node " << name;
        return nullptr;
    }
    return _ev_nodes[name]();
}

/* 创建分支结构Node */
std::shared_ptr<BpNode> BpNodeLib::CreateBaseNode(const std::string& name) {
    if (_base_nodes.find(name) == _base_nodes.end()) {
        LOG(ERROR) << "can't find base node " << name;
        return nullptr;
    }
    return _base_nodes[name]();
}

std::shared_ptr<BpNode> BpNodeLib::CreateGraphIONode(const std::shared_ptr<BpGraph>& g) {
    return nullptr;
}

std::shared_ptr<BpContents> BpNodeLib::GetContents() {
    return _root_contents;
}

} // namespace bp