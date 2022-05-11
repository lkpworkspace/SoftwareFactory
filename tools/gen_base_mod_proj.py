#!/usr/bin/python3

import sys, getopt
import os, os.path
import re
import shutil

def dumpUsage():
    print("Usage: python3 /opt/SoftwareFactory/tools/gen_base_mod_proj.py --type=MOD_TYPE --name=MOD_NAME --dir=MOD_DIR")
    print("Options:")
    print("  --type   MOD_TYPE            Base module type: [ mod | mod_with_ui | mod_panel ]")
    print("  --name   MOD_NAME            Base module name, for example: usermod")
    print("  --dir    MOD_DIR             Base module project dirtory, for example: /home/yourname/")
    print("")
    print("Sample: python3 /opt/SoftwareFactory/tools/gen_base_mod_proj.py --type=\"mod\" --name=\"usermod\" --dir=\"/home/yourname/\"")
    print("")

def checkParams(opts):
    """
    检查模块名是否符合命名规则
    检查目录是否存在
    """
    res = {}
    for opt, arg in opts:
        if opt in ('--name'):
            if re.match('^[a-zA-Z_][a-zA-Z0-9_]*$', arg):
                res['name'] = arg
            else:
                return res
        elif opt in ('--dir'):
            res['dir'] = arg;
        elif opt in ('--type'):
            res['type'] = arg
        else:
            print("Unknown option " + arg)
    res['dir'] = res['dir'] + res['name'] + '/'
    return res

def replaceParams(params, line):
    return re.sub("@template_name@", params["name"], line)

def replaceFile(params, file):
    file_content = ""
    rf = open(file, 'r')
    while True:
        line = rf.readline()
        if line == '' or line is None:
            break
        # replaceParams(params, line)
        file_content = file_content + replaceParams(params, line)
    # print(file_content)
    rf.close()

    wf = open(file, 'w')
    wf.write(file_content)
    wf.close()

if __name__ == "__main__":
    try:
        opts, args = getopt.getopt(sys.argv[1:],
            "h",
            ["type=", "name=", "dir="])
    except getopt.GetOptError:
        dumpUsage()
        sys.exit(1)
    
    opt_cnt = 3
    if len(opts) != opt_cnt:
        dumpUsage()
        sys.exit(1)
    
    print("opts %s" % opts)
    params_dict = checkParams(opts)
    if len(params_dict) != opt_cnt:
        dumpUsage()
        sys.exit(1)
    
    proj_src_dir = ""
    proj_rename_dict = {}
    proj_modify_var_list = []
    # 设置要修改的文件
    proj_type = params_dict['type']
    if proj_type == "mod":
        proj_src_dir = "/opt/SoftwareFactory/templates/base_mod"
        proj_rename_dict[params_dict["dir"] + "com/template_mod.cpp"] = params_dict["dir"] + "com/" + params_dict["name"] + ".cpp"
        proj_rename_dict[params_dict["dir"] + "conf/template_mod.json"] = params_dict["dir"] + "conf/" + params_dict["name"] + ".json"
        proj_modify_var_list.append(params_dict["dir"] + "CMakeLists.txt")
        proj_modify_var_list.append(params_dict["dir"] + "conf/" + params_dict["name"] + ".json")
    elif proj_type == "mod_with_ui":
        proj_src_dir = "/opt/SoftwareFactory/templates/base_mod_with_ui"
        proj_rename_dict[params_dict["dir"] + "com/template_mod_with_ui.cpp"] = params_dict["dir"] + "com/" + params_dict["name"] + ".cpp"
        proj_rename_dict[params_dict["dir"] + "conf/template_mod_with_ui.json"] = params_dict["dir"] + "conf/" + params_dict["name"] + ".json"
        proj_modify_var_list.append(params_dict["dir"] + "CMakeLists.txt")
        proj_modify_var_list.append(params_dict["dir"] + "conf/" + params_dict["name"] + ".json")
    elif proj_type == "mod_panel":
        proj_src_dir = "/opt/SoftwareFactory/templates/base_panel"
        proj_rename_dict[params_dict["dir"] + "template_panel.cpp"] = params_dict["dir"] + params_dict["name"] + ".cpp"
        proj_rename_dict[params_dict["dir"] + "template_panel.json"] = params_dict["dir"] + params_dict["name"] + ".json"
        proj_modify_var_list.append(params_dict["dir"] + "CMakeLists.txt")
        proj_modify_var_list.append(params_dict["dir"] + params_dict["name"] + ".json")
        proj_modify_var_list.append(params_dict["dir"] + params_dict["name"] + ".cpp")
    else:
        dumpUsage()
        sys.exit(1)

    # 拷贝到指定目录
    shutil.copytree(proj_src_dir, params_dict["dir"], True)

    # 重命名模板工程文件
    for k in proj_rename_dict:
        os.rename(k, proj_rename_dict[k])

    # 替换文件中变量
    for v in proj_modify_var_list:
        replaceFile(params_dict, v)

    print("Success!!!")
    