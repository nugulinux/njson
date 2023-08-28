/*
 * Copyright (c) 2023 SK Telecom Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <fstream>
#include <gtest/gtest.h>

#include "njson/njson.h"

#define MEMBER_JSON_STRING "{\"company\": \"skt\",\"building\": [{\"location\": \"seoul\",\"hq\": true},{\"location\": \"busan\",\"hq\": false}]}"
#define DEFAULT_JSON_STRING "{\"count\":2,\"people\":[{\"name\":\"jean\"},{\"name\":\"kim\"}]}"
#define EMPTY_JSON_STRING "{}"
#define BEAUTIFY_RAPIDJSON_STRING "{\n\
    \"count\": 2,\n\
    \"people\": [\n\
        {\n\
            \"name\": \"jean\"\n\
        },\n\
        {\n\
            \"name\": \"kim\"\n\
        }\n\
    ]\n\
}"

TEST(njsonTest, MakeSimpleObject)
{
    NJson::Value jvalue;
    unsigned int u_int = 400;
    float float_number = 3.14;

    jvalue["string"] = "text";
    jvalue["number"] = 10;
    jvalue["boolean"] = false;
    jvalue["double"] = 0.1;
    jvalue["u_int"] = u_int;
    jvalue["float"] = float_number;

    ASSERT_EQ(jvalue["string"].asString(), "text");
    ASSERT_EQ(jvalue["number"].asInt(), 10);
    ASSERT_EQ(jvalue["boolean"].asBool(), false);
    ASSERT_EQ(jvalue["double"].asDouble(), 0.1);
    ASSERT_EQ(jvalue["u_int"].asUInt(), u_int);
    ASSERT_EQ(jvalue["float"].asFloat(), float_number);

    // check string
    ASSERT_TRUE(jvalue["string"].isString());
    ASSERT_TRUE(!strcmp(jvalue["string"].asCString(), "text"));

    // check number
    ASSERT_TRUE(!jvalue["string"].isNumeric());
    ASSERT_TRUE(!jvalue["boolean"].isNumeric());
    ASSERT_TRUE(jvalue["number"].isNumeric());
    ASSERT_TRUE(jvalue["double"].isNumeric());
}

TEST(njsonTest, MakeSimpleArray)
{
    NJson::Value jvalue;
    NJson::Value jarray;

    for (int i = 0; i < 10; i++) {
        NJson::Value jitem;

        jitem["index"] = i;
        jvalue["array"].append(jitem);
    }

    jarray = jvalue["array"];
    ASSERT_EQ(jarray.size(), 10);

    for (NJson::ArrayIndex i = 0; i < jarray.size(); i++) {
        const NJson::Value& jitem = jarray[i];

        ASSERT_EQ(jitem["index"].asInt(), (int)i);
    }
}

TEST(njsonTest, MakeArrayByIndex)
{
    NJson::Value value;
    NJson::FastWriter writer;

    value["items"][0] = "item_1";
    value["items"][1] = "item_2";
    value["items"][2] = "item_3";
    value["orders"][0] = "first";
    value["orders"][1] = "second";
    value["internals"][0]["type"] = "basic";

    ASSERT_EQ(value["items"].size(), 3);
    ASSERT_EQ(value["orders"].size(), 2);
    ASSERT_EQ(value["items"][0].asString(), "item_1");
    ASSERT_EQ(value["orders"][1].asString(), "second");
    ASSERT_EQ(value["internals"][0]["type"].asString(), "basic");

    NJson::Value sub_value = value["internals"][0];
    ASSERT_EQ(sub_value["type"], "basic");
}

TEST(njsonTest, MakeMultipleObject)
{
    NJson::Value jvalue;
    NJson::Value jobject;

    jobject["id"] = "id";
    jobject["name"] = "jean";
    jvalue["person"] = jobject;

    ASSERT_EQ(jvalue["person"]["id"].asString(), "id");
    ASSERT_EQ(jvalue["person"]["name"].asString(), "jean");
}

TEST(njsonTest, DefaultParsing)
{
    NJson::Value root;
    NJson::Reader reader;

    ASSERT_TRUE(reader.parse(DEFAULT_JSON_STRING, root));

    ASSERT_EQ(root["count"].asInt(), 2);
    ASSERT_EQ(root["people"].size(), 2);
}

TEST(njsonTest, ParsingAndCheckMember)
{
    NJson::Value root;
    NJson::Reader reader;

    ASSERT_TRUE(reader.parse(MEMBER_JSON_STRING, root));

    ASSERT_TRUE(root.isMember("company"));
    ASSERT_TRUE(root.isMember("building"));
    ASSERT_TRUE(!root.isMember("location"));
    ASSERT_TRUE(!root.isMember("hq"));

    for (auto company : root["building"]) {
        ASSERT_TRUE(company.isMember("location"));
        ASSERT_TRUE(company.isMember("hq"));
    }

    // use const reference
    for (const auto& company : root["building"]) {
        ASSERT_TRUE(company.isMember("location"));
        ASSERT_TRUE(company.isMember("hq"));
    }

    // basic for loop
    for (NJson::ArrayIndex i = 0; i < root["building"].size(); i++) {
        auto company = root["building"][i];
        ASSERT_TRUE(company.isMember("location"));
        ASSERT_TRUE(company.isMember("hq"));
    }

    // use iterator
    for (NJson::Value::Iterator itr = root["building"].begin(); itr != root["building"].end(); itr++) {
        ASSERT_TRUE((*itr).isMember("location"));
        ASSERT_TRUE((*itr).isMember("hq"));
    }
}

TEST(njsonTest, ParsingNoExistNode)
{
    NJson::Value root;
    NJson::Reader reader;

    ASSERT_TRUE(reader.parse(EMPTY_JSON_STRING, root));

    ASSERT_TRUE(!root["none"].size());
    ASSERT_TRUE(!root["none"].asInt());
    ASSERT_TRUE(!root["none"].isObject());
    ASSERT_TRUE(!root["none"].isArray());
    ASSERT_EQ(root["none"].asString(), "");
    ASSERT_TRUE(!root["none"].asInt());
    ASSERT_TRUE(!root["none"].asDouble());
    ASSERT_TRUE(!root["none"].asBool());
}

TEST(njsonTest, ParsingFromStream)
{
    std::ifstream config("test.json", std::ifstream::binary);
    NJson::FastWriter writer;
    NJson::Value root;
    config >> root;

    ASSERT_TRUE(!root.empty());
    ASSERT_EQ(root["server"].asString(), "test_server");
    ASSERT_EQ(root["code"].asString(), "1234");
    ASSERT_EQ(root["info"]["number"].asString(), "abcd");
}

TEST(njsonTest, CheckKeyAndGetValue)
{
    NJson::Value root;
    NJson::Reader reader;

    ASSERT_TRUE(reader.parse(DEFAULT_JSON_STRING, root));

    ASSERT_TRUE(!root["count"].empty());
    ASSERT_TRUE(root["count"].isInt());
    ASSERT_EQ(root["count"].asInt(), 2);

    ASSERT_TRUE(!root["people"].empty());
    ASSERT_TRUE(root["people"].isArray());
    ASSERT_EQ(root["people"].size(), 2);

    NJson::Value jean;
    jean = root["people"][0];
    ASSERT_TRUE(!jean["name"].empty());
    ASSERT_EQ(jean["name"].asString(), "jean");

    NJson::Value kim;
    kim = root["people"][1];
    ASSERT_TRUE(!kim["name"].empty());
    ASSERT_EQ(kim["name"].asString(), "kim");
}

TEST(njsonTest, OverrideObjectValue)
{
    NJson::Value jvalue;

    jvalue["string"] = "org";
    ASSERT_EQ(jvalue["string"].asString(), "org");

    jvalue["string"] = "mod";
    ASSERT_EQ(jvalue["string"].asString(), "mod");
}

TEST(njsonTest, AppendObjectToJson)
{
    NJson::Value builder;
    NJson::Value person1, person2;
    NJson::Value root;
    NJson::Reader reader;
    NJson::FastWriter writer;
    NJson::Value leader;

    builder["count"] = 2;
    person1["name"] = "jean";
    person2["name"] = "kim";
    builder["people"].append(person1);
    builder["people"].append(person2);
    ASSERT_EQ(writer.write(builder), DEFAULT_JSON_STRING);

    ASSERT_TRUE(reader.parse(DEFAULT_JSON_STRING, root));
    ASSERT_EQ(writer.write(root), DEFAULT_JSON_STRING);

    leader["name"] = "jean";
    ASSERT_EQ(writer.write(leader), "{\"name\":\"jean\"}");

    // insert json value to builder
    builder["leader"] = leader;
    ASSERT_EQ(writer.write(builder), "{\"count\":2,\"people\":[{\"name\":\"jean\"},{\"name\":\"kim\"}],\"leader\":{\"name\":\"jean\"}}");

    // insert json value to json parser
    root["leader"] = leader;
    ASSERT_EQ(writer.write(root), "{\"count\":2,\"people\":[{\"name\":\"jean\"},{\"name\":\"kim\"}],\"leader\":{\"name\":\"jean\"}}");
}

TEST(njsonTest, MakeAndParseValue)
{
    NJson::Value jvalue;

    jvalue["string"] = "text";
    jvalue["number"] = 10;
    jvalue["bool"] = true;

    ASSERT_EQ(jvalue["string"].asString(), "text");
    ASSERT_EQ(jvalue["number"].asInt(), 10);
    ASSERT_EQ(jvalue["bool"].asBool(), true);
    ASSERT_TRUE(jvalue["bool"] == true);
}

TEST(njsonTest, HandleLargestInt)
{
    NJson::Value jvalue;
    long long_int = 2147483640;
    long long largest_int = 21474836470;

    jvalue["largest_int"] = largest_int;
    jvalue["casting_largest_int"] = (NJson::LargestInt)long_int;

    ASSERT_EQ(jvalue["largest_int"].asLargestInt(), largest_int);
    ASSERT_EQ(jvalue["casting_largest_int"].asLargestInt(), long_int);
}

TEST(njsonTest, AssignNullValue)
{
    NJson::Value jvalue;

    jvalue["null"] = NJson::nullValue;

    ASSERT_TRUE(jvalue["null"].isNull());
    ASSERT_TRUE(jvalue["null"].empty());
}

TEST(njsonTest, Stringify)
{
    NJson::Value root;
    NJson::Reader reader;
    NJson::FastWriter fast_writer;
    NJson::StyledWriter styled_writer;

    ASSERT_TRUE(reader.parse(DEFAULT_JSON_STRING, root));
    ASSERT_EQ(fast_writer.write(root), DEFAULT_JSON_STRING);
    ASSERT_EQ(styled_writer.write(root), BEAUTIFY_RAPIDJSON_STRING);
}

TEST(njsonTest, CopyParsedValue)
{
    NJson::Value* root = new NJson::Value();
    NJson::Value value;
    NJson::Reader reader;

    ASSERT_TRUE(reader.parse(DEFAULT_JSON_STRING, *root));

    value = (*root)["people"];

    delete root;

    ASSERT_EQ(value[0]["name"].asString(), "jean");
    ASSERT_EQ(value[1]["name"].asString(), "kim");
}

TEST(njsonTest, ConstructValue)
{
    NJson::Value str_value = NJson::Value("DATA_VALUE");
    NJson::Value array_value = NJson::arrayValue;
    NJson::Value array_item1, array_item2;
    NJson::FastWriter writer;

    ASSERT_EQ(writer.write(str_value), "\"DATA_VALUE\"");
    ASSERT_EQ(writer.write(array_value), "[]");

    array_item1["item"] = "array_1";
    array_item2["item"] = "array_2";
    array_value.append(array_item1).append(array_item2);

    ASSERT_TRUE(array_value.isArray());
    ASSERT_TRUE(!array_value.empty());
    ASSERT_EQ(array_value.size(), 2);

    // construct value by deep copy
    std::string original_value_str = writer.write(array_value);
    NJson::Value copied_value = array_value;
    array_value.clear();

    ASSERT_TRUE(array_value.empty());
    ASSERT_EQ(writer.write(array_value), "[]");
    ASSERT_TRUE(!copied_value.empty());
    ASSERT_EQ(copied_value.size(), 2);
    ASSERT_EQ(writer.write(copied_value), original_value_str);
}

TEST(njsonTest, SwapValue)
{
    const auto DATA = "{\"name\":\"Kim\",\"car\":[\"benz\",\"bmw\"]}";

    NJson::Value src_value;
    NJson::Value dest_value;
    NJson::FastWriter writer;

    src_value["name"] = "Kim";
    src_value["car"].append(NJson::Value("benz"));
    src_value["car"].append(NJson::Value("bmw"));

    ASSERT_EQ(writer.write(src_value), DATA);
    ASSERT_TRUE(dest_value.empty());

    dest_value.swap(src_value);

    ASSERT_EQ(writer.write(dest_value), DATA);
    ASSERT_TRUE(src_value.empty());
}

TEST(njsonTest, ClearValue)
{
    const auto OBJECT_DATA = "{\"name\":\"Kim\",\"age\":12}";
    const auto ARRAY_DATA = "[\"benz\",\"bmw\",\"audi\",\"honda\"]";
    const auto MIXED_DATA = "{\"name\":\"Kim\",\"car\":[\"benz\",\"bmw\"]}";

    for (const auto& data : { OBJECT_DATA, ARRAY_DATA, MIXED_DATA }) {
        NJson::Value value;
        NJson::Reader reader;

        ASSERT_TRUE(reader.parse(data, value));

        value.clear();

        ASSERT_TRUE(value.empty());
    }
}
