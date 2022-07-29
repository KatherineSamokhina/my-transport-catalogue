#include <stdexcept>
#include <utility>
#include <variant>

#include "json_builder.h"

namespace json {

    using namespace std::literals;

    BuildConstructor::BuildConstructor(Builder& builder)
        : builder_(builder)
    {}

    BuildContextFirst::BuildContextFirst(Builder& builder)
        : BuildConstructor(builder)
    {}

    DictContext& BuildContextFirst::StartDict() {
        return builder_.StartDict();
    }

    ArrayContext& BuildContextFirst::StartArray() {
        return builder_.StartArray();
    }

    BuildContextSecond::BuildContextSecond(Builder& builder)
        : BuildConstructor(builder)
    {}

    KeyContext& BuildContextSecond::Key(std::string key) {
        return builder_.Key(key);
    }

    Builder& BuildContextSecond::EndDict() {
        return builder_.EndDict();
    }

    KeyContext::KeyContext(Builder& builder)
        : BuildContextFirst(builder)
    {}

    ValueKeyContext& KeyContext::Value(Node::Value value) {
        return builder_.Value(value);
    }

    ValueKeyContext::ValueKeyContext(Builder& builder)
        : BuildContextSecond(builder)
    {}

    ValueArrayContext::ValueArrayContext(Builder& builder)
        : BuildContextFirst(builder)
    {}

    ValueArrayContext& ValueArrayContext::Value(Node::Value value) {
        return builder_.Value(value);
    }

    Builder& ValueArrayContext::EndArray() {
        return builder_.EndArray();
    }

    DictContext::DictContext(Builder& builder)
        : BuildContextSecond(builder)
    {}

    ArrayContext::ArrayContext(Builder& builder)
        : ValueArrayContext(builder)
    {}

    Builder::Builder()
        : KeyContext(*this)
        , ValueKeyContext(*this)
        , DictContext(*this)
        , ArrayContext(*this)
    {}

    KeyContext& Builder::Key(std::string key) {
        if (UnableUseKey()) {
            throw std::logic_error("Key can't be applied"s);
        }
        nodes_stack_.push_back(std::make_unique<Node>(key));
        return *this;
    }

    Builder& Builder::Value(Node::Value value) {
        if (UnableUseValue()) {
            throw std::logic_error("Value can't be applied"s);
        }
        PushNode(value);
        return AddNode(*nodes_stack_.back().release());
    }

    DictContext& Builder::StartDict() {
        if (UnableUseStartDict()) {
            throw std::logic_error("StartDict can't be applied"s);
        }
        nodes_stack_.push_back(std::make_unique<Node>(Dict()));
        return *this;
    }

    Builder& Builder::EndDict() {
        if (UnableUseEndDict()) {
            throw std::logic_error("EndDict can't be applied"s);
        }
        return AddNode(*nodes_stack_.back().release());
    }

    ArrayContext& Builder::StartArray() {
        if (UnableUseStartArray()) {
            throw std::logic_error("StartArray can't be applied"s);
        }
        nodes_stack_.push_back(std::make_unique<Node>(Array()));
        return *this;
    }

    Builder& Builder::EndArray() {
        if (UnableUseEndArray()) {
            throw std::logic_error("EndArray can't be applied"s);
        }
        return AddNode(*nodes_stack_.back().release());
    }

    Node Builder::Build() const {
        if (UnableUseBuild()) {
            throw std::logic_error("Builder can't be applied"s);
        }
        return node_;
    }

    bool Builder::UnableAdd() const {
        return !(nodes_stack_.empty()
            || nodes_stack_.back()->IsArray()
            || nodes_stack_.back()->IsString());
    }

    bool Builder::IsMakeObj() const {
        return !node_.IsNull();
    }

    bool Builder::UnableUseKey() const {
        return IsMakeObj()
            || nodes_stack_.empty()
            || !nodes_stack_.back()->IsDict();
    }

    bool Builder::UnableUseValue() const {
        return IsMakeObj()
            || UnableAdd();
    }

    bool Builder::UnableUseStartDict() const {
        return UnableUseValue();
    }

    bool Builder::UnableUseEndDict() const {
        return IsMakeObj()
            || nodes_stack_.empty()
            || !nodes_stack_.back()->IsDict();
    }

    bool Builder::UnableUseStartArray() const {
        return UnableUseValue();
    }

    bool Builder::UnableUseEndArray() const {
        return IsMakeObj()
            || nodes_stack_.empty()
            || !nodes_stack_.back()->IsArray();
    }

    bool Builder::UnableUseBuild() const {
        return !IsMakeObj();
    }

    Builder& Builder::AddNode(const Node& node) {
        nodes_stack_.pop_back();
        if (nodes_stack_.empty()) {
            node_ = node;
        }
        else if (nodes_stack_.back()->IsArray()) {
            nodes_stack_.back()->AsArray().push_back(node);
        }
        else {
            const Node& key = *nodes_stack_.back().release();
            nodes_stack_.pop_back();
            nodes_stack_.back()->AsDict().emplace(key.AsString(), node);
        }
        return *this;
    }

    void Builder::PushNode(Node::Value value) {
        visit([this](auto&& val) {
            nodes_stack_.push_back(std::make_unique<Node>(val));
            }, value);
    }
}