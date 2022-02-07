﻿#ifndef __BpGraph_hpp__
#define __BpGraph_hpp__
#include <vector>
#include <unordered_map>

#include "BpLink.hpp"
#include "BpVariable.hpp"
#include "BpNode.hpp"
#include "evnode/BpNodeEv.hpp"

namespace bp {

class BpGraph : public BpNode
{
	typedef std::set<std::shared_ptr<BpNodeEv>, BpNodeEvCmp>            event_node_run_t;
	typedef std::unordered_map<std::string, std::shared_ptr<BpNodeEv>>  event_node_map_t;
	typedef std::vector<std::shared_ptr<BpNode>>                        node_vec_t;
	typedef std::unordered_map<std::string, BpVariable>                 variable_map_t;
public:
	friend class Bp;
public:
	BpGraph(std::shared_ptr<BpGraph> parent = nullptr);
	BpGraph(std::string name, BpNodeType t, std::shared_ptr<BpGraph> parent = nullptr);

	virtual void Run() override;
	virtual void Logic() {
		if (_node_type == BpNodeType::BP_GRAPH && !_input_node.expired()){
			_input_node.lock()->Run();
		}
	}

	bool AddModGraphPin(const std::string& name, BpNodeType nt, const BpVariable& v);
	bool DelModGraphPin(int id);

	event_node_map_t& GetEvNodes() { return _event_nodes; }
	bool AddEventNode(std::shared_ptr<BpNode>);
	void DelEventNode(std::string name);

	node_vec_t& GetNodes() { return _nodes; }
	std::shared_ptr<BpNode> GetNode(int id);
	void AddNode(std::shared_ptr<BpNode>);
	void DelNode(std::shared_ptr<BpNode>);
	void DelNode(int id);

	std::vector<BpLink> SearchLinks(int id);
	std::vector<BpLink>& GetLinks() { return _links; }
	int AddLink(BpPin& start_pin, BpPin& end_pin);
	void DelLink(int id);
	BpLink GetLink(int id);
	
	BpPin* SearchPin(int id);

	/* 获得变量列表 */
	const variable_map_t& GetVariables() { return _vars; }

	/* 获得变量 */
	BpVariable& GetVariable(const std::string& name);
	
	/* 添加变量 */
	bool AddVariable(const std::string& name, const BpVariable& v);

	/* 修改自定义变量名字 */
	bool ModifyVariableName(const std::string& old_name, const std::string& new_name);

	/* 删除一个变量 */
	void RemoveVariable(const std::string& name);

	/* 清空图 */
	void Clear();

	int GetNextID();
	
	void ClearFlag();

	void SetNodesPos(const Json::Value& desc) { _nodes_pos = desc; }
	const Json::Value& GetNodesPos() { return _nodes_pos; }

	/* 编辑器调用的函数 */
	void RunNextEventBeign();
	bool RunNextEvent();

private:
	void SetNextID(int id);

	int                         _next_id;
	/* 图的起点 */
	event_node_run_t            _event_nodes_run;
	event_node_map_t            _event_nodes;
	/* 图中所有节点列表 */
	node_vec_t                  _nodes;
	/* 图中所有连线 */
    std::vector<BpLink>         _links;
	/* 图中的成员变量 */
	variable_map_t              _vars;
	BpVariable                  _null_val;

	std::weak_ptr<BpNode>       _input_node;
	std::weak_ptr<BpNode>       _output_node;

	Json::Value                 _nodes_pos;
};

} // namespace bp
#endif