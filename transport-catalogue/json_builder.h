#pragma once

#include <memory>
#include <vector>
#include <string>

#include "json.h"

namespace json {

    class BuildConstructor;
    class BuildContextFirst;
    class BuildContextSecond;
    class KeyContext;
    class ValueKeyContext;
    class ValueArrayContext;
    class DictContext;
    class ArrayContext;
    class Builder;

    class BuildConstructor {
    public:
        explicit BuildConstructor(Builder& builder);
    protected:
        Builder& builder_;
    };

    class BuildContextFirst : public BuildConstructor {
    public:
        explicit BuildContextFirst(Builder& builder);
        DictContext& StartDict();
        ArrayContext& StartArray();
    };

    class BuildContextSecond : public BuildConstructor {
    public:
        explicit BuildContextSecond(Builder& builder);
        KeyContext& Key(std::string key);
        Builder& EndDict();
    };

    class KeyContext : public BuildContextFirst {
    public:
        explicit KeyContext(Builder& builder);
        ValueKeyContext& Value(Node::Value value);
    };

    class ValueKeyContext : public BuildContextSecond {
    public:
        explicit ValueKeyContext(Builder& builder);
    };

    class ValueArrayContext : public BuildContextFirst {
    public:
        explicit ValueArrayContext(Builder& builder);
        ValueArrayContext& Value(Node::Value value);
        Builder& EndArray();
    };

    class DictContext : public BuildContextSecond {
    public:
        explicit DictContext(Builder& builder);
    };

    class ArrayContext : public ValueArrayContext {
    public:
        explicit ArrayContext(Builder& builder);
    };

    class Builder final : virtual public KeyContext, virtual public ValueKeyContext, virtual public DictContext, virtual public ArrayContext {
        Node node_ = nullptr;
        std::vector<std::unique_ptr<Node>> nodes_stack_;

    public:
        Builder();
        KeyContext& Key(std::string key);
        Builder& Value(Node::Value value);
        DictContext& StartDict();
        Builder& EndDict();
        ArrayContext& StartArray();
        Builder& EndArray();
        Node Build() const;

    private:
        bool UnableAdd() const;
        bool IsMakeObj() const;
        bool UnableUseKey() const;
        bool UnableUseValue() const;
        bool UnableUseStartDict() const;
        bool UnableUseEndDict() const;
        bool UnableUseStartArray() const;
        bool UnableUseEndArray() const;
        bool UnableUseBuild() const;
        Builder& AddNode(const Node& node);
        void PushNode(Node::Value value);
    };
}