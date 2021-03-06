#include <string>
#include <sstream>
#include <glog/logging.h>
#include "BpModuleLinux.hpp"

namespace bp {

BpModuleLinux::~BpModuleLinux() {
    if (_dl != nullptr) {
        dlclose(_dl);
        _dl = nullptr;
    }
}

bool BpModuleLinux::Init(const char* dll_path) {
    std::string cmd = "nm -g -n -P ";
    cmd += dll_path;
    if (GetDllSymbol(cmd.c_str())) {
        return false;
    }
    if (OpenDll(dll_path)) {
        return false;
    }
    return true;
}

// FIXME: 使用模糊搜索函数名,对于重载的函数可能会搞混
void* BpModuleLinux::GetFunc(const std::string& func_name) {
    int idx = -1;
    for (int i = 0; i < _symbols.size(); ++i) {
        if (_symbols[i].find(func_name) == std::string::npos) {
            continue;
        }
        idx = i;
        break;
    }
    if (idx == -1) {
        return nullptr;
    }
    LOG(INFO) << "load func \"" << func_name << "\" : " << _symbols[idx];
    return dlsym(_dl, _symbols[idx].c_str());
}

int BpModuleLinux::GetDllSymbol(const char* cmd) {
    FILE *fp = nullptr;
    if (cmd == nullptr) {
        return -1;
    }
    if ((fp = popen(cmd, "r")) == nullptr) {
        return -1;
    }
    _symbols.clear();
    char buf[1024];
    while (fgets(buf, sizeof(buf), fp)) {
        auto symbol = FilterSymbol(buf);
        if (symbol.empty()) {
            continue;
        }
        _symbols.emplace_back(symbol);
    }
    if (pclose(fp) == -1) {
        return -1;
    }
    return 0;
}

int BpModuleLinux::OpenDll(const std::string& dll_file) {
    const char* dll_path = dll_file.c_str();
    _dl = dlopen(dll_path, RTLD_NOW | RTLD_LOCAL);
    if(nullptr == _dl) {
        return -1;
    }
    return 0;
}

std::string BpModuleLinux::FilterSymbol(const std::string& line) {
    std::stringstream ss(line);
    std::string res;
    if (std::getline(ss, res, ' ')) {
        std::string t;
        std::getline(ss, t, ' ');
        if (t == "T" || t == "t") {
            return res;
        }
    }
    return "";
}

} // namespace bp