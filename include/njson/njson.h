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

#ifndef __NJSON_H__
#define __NJSON_H__

#include <fstream>
#include <iostream>
#include <memory>
#include <string>

namespace NJson {

using ArrayIndex = unsigned int;
using LargestInt = long long;

enum ValueType {
    nullValue = 0,
    arrayValue
};

class Value {
public:
    using ArrayIndex = NJson::ArrayIndex;

    class Iterator {
    public:
        bool operator==(const Iterator& other) const;
        bool operator!=(const Iterator& other) const;
        Iterator& operator++();
        Iterator& operator++(int);
        Iterator& operator--();
        Iterator& operator--(int);
        Value operator*();

    private:
        friend class Value;
        struct IteratorArgs;
        struct IteratorImpl;

        Iterator(const IteratorArgs& args);

        std::shared_ptr<IteratorImpl> iterator_pimpl;
    };

public:
    Value();
    Value(const Value& other);
    Value(ValueType type);
    Value(const std::string& str);
    Value(const char* str);
    Value(bool value);

    bool operator==(const Value& other) const;
    Value operator[](const std::string& name);
    const Value operator[](const std::string& name) const;
    Value operator[](ArrayIndex index);
    const Value operator[](ArrayIndex index) const;
    Value& operator=(const Value& value);
    Value& operator=(ValueType type);
    Value& operator=(const char* value);
    Value& operator=(int value);
    Value& operator=(unsigned int value);
    Value& operator=(long long value);
    Value& operator=(bool value);
    Value& operator=(float value);
    Value& operator=(double value);

    Value& append(const Value& other);
    ArrayIndex size() const;
    bool empty() const;
    bool isNull() const;
    bool isMember(const std::string& name) const;
    bool isObject() const;
    bool isArray() const;
    bool isString() const;
    bool isInt() const;
    bool isNumeric() const;
    bool isBool() const;
    const char* asCString() const;
    std::string asString() const;
    int asInt() const;
    unsigned int asUInt() const;
    long long asLargestInt() const;
    bool asBool() const;
    float asFloat() const;
    double asDouble() const;

    Value::Iterator begin() const;
    Value::Iterator end() const;
    void swap(Value& other);
    void clear();

private:
    friend class StyledWriter;
    friend class FastWriter;
    friend class Reader;
    friend std::istream& operator>>(std::istream& input_stream, Value& value);

    struct ValueArgs;
    struct ValueImpl;

    Value(const ValueArgs& args);

    std::shared_ptr<ValueImpl> pimpl;
};

class Reader {
public:
    bool parse(const std::string& data, Value& node);
};

class StyledWriter {
public:
    std::string write(const Value& value);
};

class FastWriter {
public:
    std::string write(const Value& value);
};

std::istream& operator>>(std::istream& input_stream, Value& value);

};

#endif // __NJSON_H__
