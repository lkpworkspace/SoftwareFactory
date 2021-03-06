#ifndef __BpVariable_hpp__
#define __BpVariable_hpp__
#include <iostream>
#include <memory>
#include <string>

#include <google/protobuf/message.h>

#include "BpModule.hpp"

namespace bp {

class BpVariable
{
	friend class BpGraph;
	friend std::ostream& operator<<(std::ostream& out, const BpVariable& var) {
		out << var._var_type << "(" << var._var_desc << ") = " << var._var->DebugString();
		return out;
	}
public:
	BpVariable();
	// 引用传递
	BpVariable(const std::string& vt, const BpModuleVar& bpvar);
	BpVariable(const std::string& vt, const std::string& desc, const pb_msg_ptr_t& v);
	BpVariable(const BpVariable& v){ operator=(v); }
	BpVariable& operator=(const BpVariable& o);
	BpVariable(BpVariable&& v);

	/// 是否为空值
	bool IsNone() { return _var == nullptr; }

	/**
	 * @brief 获得proto类型变量值
	 * 
	 * @tparam T proto变量类型
	 * @return std::shared_ptr<T> proto对象
	 */
	template<typename T>
	std::shared_ptr<T> Get() { return std::dynamic_pointer_cast<T>(_var); }

	const std::string& GetDesc() const { return _var_desc; }

	/// 获得变量类型
	const std::string& GetType() const { return _var_type; }

	/// 转换接送格式
	const std::string ToJson() const;

	/// 判断变量类型是否一致
	bool IsSameType(const BpVariable& v) { return ((_var_type == "any") || (_var_type == v._var_type)); }

	/// 值传递
	bool SetValue(const std::string& v);
	bool SetValue(const Json::Value& v);
	bool SetValue(const pb_msg_ptr_t& v);

	/// 引用传递
	void SetValueByRef(const pb_msg_ptr_t& v);

private:
	std::string _var_type;
	std::string _var_desc;
	pb_msg_ptr_t _var;
};

} // namespace bp
#endif