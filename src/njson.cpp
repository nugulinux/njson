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

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include "njson/njson.h"

namespace NJson {
using NativeValue = rapidjson::Value;
using NativeValueIterator = rapidjson::Value::ValueIterator;
using NativeAllocator = rapidjson::MemoryPoolAllocator<>;

template <typename T>
std::string stringify(const NativeValue* native_value)
{
    if (!native_value)
        return "";

    rapidjson::StringBuffer buffer;

    T writer(buffer);
    native_value->Accept(writer);

    return buffer.GetString();
}

/*******************************************************************************
 * define helper structure
 ******************************************************************************/
struct Value::Iterator::IteratorArgs {
    const NativeValueIterator& native_iterator;
    NativeAllocator* allocator;
};

struct Value::Iterator::IteratorImpl {
    NativeValueIterator native_iterator;
    NativeAllocator* allocator;

    IteratorImpl(const NativeValueIterator& native_iterator, NativeAllocator* allocator)
        : native_iterator(native_iterator)
        , allocator(allocator)
    {
    }
};

struct Value::ValueArgs {
    NativeValue* native_value;
    NativeAllocator* allocator;
};

struct Value::ValueImpl {
    std::shared_ptr<NativeValue> raw_native_value;
    std::shared_ptr<NativeAllocator> raw_allocator;

    NativeValue* native_value;
    NativeAllocator* allocator;

    ValueImpl()
        : raw_native_value(std::make_shared<NativeValue>())
        , raw_allocator(std::make_shared<NativeAllocator>())
        , native_value(raw_native_value.get())
        , allocator(raw_allocator.get())
    {
    }

    ValueImpl(NativeValue* value, NativeAllocator* allocator)
        : native_value(value)
        , allocator(allocator)
    {
        if (!this->native_value) {
            raw_native_value = std::make_shared<NativeValue>();
            this->native_value = raw_native_value.get();
        }

        if (!this->allocator) {
            raw_allocator = std::make_shared<NativeAllocator>();
            this->allocator = raw_allocator.get();
        }
    }

    template <typename T>
    Value getValue(T key) const
    {
        return Value({ &((*native_value)[key]), allocator });
    }

    Value getMemberByName(const std::string& name, bool is_const = false) const
    {
        if (native_value->IsNull())
            native_value->SetObject();

        if (!native_value->HasMember(name.c_str())) {
            if (is_const)
                return Value();
            else
                native_value->AddMember(NativeValue(name.c_str(), *allocator), NativeValue(), *allocator);
        }

        return getValue(name.c_str());
    }
};

/*******************************************************************************
 * define Value::Iterator
 ******************************************************************************/
Value::Iterator::Iterator(const IteratorArgs& args)
    : iterator_pimpl(std::make_shared<IteratorImpl>(args.native_iterator, args.allocator))
{
}

bool Value::Iterator::operator==(const Iterator& other) const
{
    return iterator_pimpl->native_iterator == other.iterator_pimpl->native_iterator;
}

bool Value::Iterator::operator!=(const Iterator& other) const
{
    return iterator_pimpl->native_iterator != other.iterator_pimpl->native_iterator;
}

Value::Iterator& Value::Iterator::operator++()
{
    ++(iterator_pimpl->native_iterator);

    return *this;
}

Value::Iterator& Value::Iterator::operator++(int)
{
    (iterator_pimpl->native_iterator)++;

    return *this;
}

Value::Iterator& Value::Iterator::operator--()
{
    --(iterator_pimpl->native_iterator);

    return *this;
}

Value::Iterator& Value::Iterator::operator--(int)
{
    (iterator_pimpl->native_iterator)--;

    return *this;
}

Value Value::Iterator::operator*()
{
    return Value({ iterator_pimpl->native_iterator, iterator_pimpl->allocator });
}

/*******************************************************************************
 * define Value
 ******************************************************************************/
Value::Value()
    : pimpl(std::make_shared<ValueImpl>())
{
}

Value::Value(const ValueArgs& args)
    : pimpl(std::make_shared<ValueImpl>(args.native_value, args.allocator))
{
}

Value::Value(const Value& other)
    : Value()
{
    *this = other;
}

Value::Value(ValueType type)
    : Value()
{
    pimpl->native_value->SetArray();
}

Value::Value(const std::string& str)
    : Value()
{
    *this = str.c_str();
}

Value Value::operator[](const std::string& name)
{
    return pimpl->getMemberByName(name);
}

const Value Value::operator[](const std::string& name) const
{
    return pimpl->getMemberByName(name, true);
}

Value Value::operator[](ArrayIndex index)
{
    return pimpl->getValue(index);
}

const Value Value::operator[](ArrayIndex index) const
{
    return pimpl->getValue(index);
}

Value& Value::operator=(const Value& value)
{
    pimpl->native_value->CopyFrom(*value.pimpl->native_value, *pimpl->allocator);

    return *this;
}

Value& Value::operator=(ValueType type)
{
    switch (type) {
    case ValueType::nullValue:
        pimpl->native_value->SetNull();
        break;
    case ValueType::arrayValue:
        pimpl->native_value->SetArray();
        break;
    };

    return *this;
}

Value& Value::operator=(const char* value)
{
    if (value)
        pimpl->native_value->SetString(value, *pimpl->allocator);

    return *this;
}

Value& Value::operator=(int value)
{
    pimpl->native_value->SetInt(value);

    return *this;
}

Value& Value::operator=(unsigned int value)
{
    pimpl->native_value->SetUint(value);

    return *this;
}

Value& Value::operator=(long long value)
{
    pimpl->native_value->SetInt64(value);

    return *this;
}

Value& Value::operator=(bool value)
{
    pimpl->native_value->SetBool(value);

    return *this;
}

Value& Value::operator=(float value)
{
    pimpl->native_value->SetFloat(value);

    return *this;
}

Value& Value::operator=(double value)
{
    pimpl->native_value->SetDouble(value);

    return *this;
}

Value& Value::append(const Value& other)
{
    if (!pimpl->native_value->IsArray())
        pimpl->native_value->SetArray();

    pimpl->native_value->PushBack(NativeValue(*other.pimpl->native_value, *pimpl->allocator), *pimpl->allocator);

    return *this;
}

ArrayIndex Value::size() const
{
    if (pimpl->native_value->IsArray())
        return pimpl->native_value->Size();

    return 0;
}

bool Value::empty() const
{
    if (pimpl->native_value->IsArray())
        return pimpl->native_value->Empty();
    else if (pimpl->native_value->IsObject())
        return pimpl->native_value->ObjectEmpty();

    return isNull();
}

bool Value::isNull() const
{
    return pimpl->native_value->IsNull();
}

bool Value::isMember(const std::string& name) const
{
    return pimpl->native_value->HasMember(name.c_str());
}

bool Value::isObject() const
{
    return pimpl->native_value->IsObject();
}

bool Value::isArray() const
{
    return pimpl->native_value->IsArray();
}

bool Value::isString() const
{
    return pimpl->native_value->IsString();
}

bool Value::isInt() const
{
    return pimpl->native_value->IsInt();
}

bool Value::isNumeric() const
{
    return pimpl->native_value->IsNumber();
}

const char* Value::asCString() const
{
    if (pimpl->native_value->IsString())
        return pimpl->native_value->GetString();

    return nullptr;
}

std::string Value::asString() const
{
    if (auto raw_string = asCString())
        return raw_string;

    return "";
}

int Value::asInt() const
{
    if (pimpl->native_value->IsInt())
        return pimpl->native_value->GetInt();

    return 0;
}

unsigned int Value::asUInt() const
{
    if (pimpl->native_value->IsUint())
        return pimpl->native_value->GetUint();

    return 0;
}

long long Value::asLargestInt() const
{
    if (pimpl->native_value->IsInt64())
        return pimpl->native_value->GetInt64();

    return 0;
}

bool Value::asBool() const
{
    if (pimpl->native_value->IsBool())
        return pimpl->native_value->GetBool();

    return false;
}

float Value::asFloat() const
{
    if (pimpl->native_value->IsFloat())
        return pimpl->native_value->GetFloat();

    return 0;
}

double Value::asDouble() const
{
    if (pimpl->native_value->IsDouble())
        return pimpl->native_value->GetDouble();

    return 0;
}

Value::Iterator Value::begin() const
{
    return Iterator({ pimpl->native_value->Begin(), pimpl->allocator });
}

Value::Iterator Value::end() const
{
    return Iterator({ pimpl->native_value->End(), pimpl->allocator });
}

void Value::swap(Value& other)
{
    other.pimpl->native_value->Swap(*pimpl->native_value);
}

void Value::clear()
{
    if (pimpl->native_value->IsArray())
        pimpl->native_value->Clear();
    else if (pimpl->native_value->IsObject())
        pimpl->native_value->RemoveAllMembers();
}

/*******************************************************************************
 * define Reader, StyledWriter, FastWriter
 ******************************************************************************/
bool Reader::parse(const std::string& data, Value& node)
{
    rapidjson::Document document;

    document.Parse(data.c_str());
    if (!document.HasParseError()) {
        node = Value::ValueArgs { &document };

        return true;
    }

    return false;
}

std::string StyledWriter::write(const Value& value)
{
    return stringify<rapidjson::PrettyWriter<rapidjson::StringBuffer>>(value.pimpl->native_value);
}

std::string FastWriter::write(const Value& value)
{
    return stringify<rapidjson::Writer<rapidjson::StringBuffer>>(value.pimpl->native_value);
}
} // NJson
