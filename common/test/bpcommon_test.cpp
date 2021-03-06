#include <map>
#include <gtest/gtest.h>

#include "../bpcommon.hpp"
#include "bpbase.pb.h"
#include "bpmath.pb.h"

TEST(bpcommon, PbJsonConvert) {
    {
        ::bp::Int32 msg;
        // msg.set_var(100);
        std::string json;
        bp::JsonPbConvert::PbMsg2JsonStr(msg, json);
        std::cout << "json: " << json << std::endl;
        EXPECT_TRUE(bp::JsonPbConvert::JsonStr2PbMsg(json, msg));
        std::cout << "pb: \n{" << msg.DebugString() << "}" << std::endl;
    }

    {
        Json::Value v;
        Json::Value v2;
        v2["1"] = "hello";
        v["a"] = v2;
        auto dst = bp::BpCommon::Json2Str(v);
        std::cout << "fast write: " << dst << std::endl;
        Json::Value::Members mem = v.getMemberNames();
        for (auto iter = mem.begin(); iter != mem.end(); ++iter) {
            std::cout << *iter << std::endl;
            std::cout << v[*iter] << std::endl;
        }
    }

    {
        // test repeate data
        // TODO
        ::bp::Int32Ary msg;
        std::string json;
        bp::JsonPbConvert::PbMsg2JsonStr(msg, json);
        std::cout << "repeat ary: " << json << std::endl;
    }

    {
        std::string str = "9helll";
        EXPECT_FALSE(bp::BpCommon::IsName(str));
        str = "_helll";
        EXPECT_TRUE(bp::BpCommon::IsName(str));
        str = "9h elll";
        EXPECT_FALSE(bp::BpCommon::IsName(str));
        str = "hell_9helll_";
        EXPECT_TRUE(bp::BpCommon::IsName(str));
        str = "__maixxxx";
        EXPECT_TRUE(bp::BpCommon::IsName(str));
    }

    {
        std::map<int, int> m = {
            {3, 3},
            {9, 9},
            {1, 1},
        };
        for (const auto& p : m) {
            std::cout << p.first << std::endl;
        }
    }
}
