#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>
#include <stdexcept>
#include <utility>

#include "json.h"

namespace json {
    using json::Node;

    class Builder {
    public:
        class ChildValueItemContext;
        class ChildKeyValueItemContext;
        class ChildDictItemContext;
        class ChildArrayItemContext;
        class ChildArrayItemValueContext;

        ChildValueItemContext Key(std::string key);
        Builder& Value(Node::Value value);

        ChildDictItemContext StartDict();
        Builder& EndDict();

        ChildArrayItemContext StartArray();
        Builder& EndArray();
        Node Build();

    private:
        Node root_;
        std::vector<Node*> nodes_stack_;
        bool no_content_ = true;

        std::variant<ChildDictItemContext, ChildArrayItemContext> StartCollection(json::Node node);

    public:

        class CommonContext {
        public:
            CommonContext(Builder& builder)
                : builder_(builder) {}

            Builder::ChildValueItemContext Key(std::string key);
            Builder& Value(Node::Value value);
            Builder::ChildDictItemContext StartDict();
            Builder& EndDict();
            Builder::ChildArrayItemContext StartArray();
            Builder& EndArray();

        protected:
            Builder& builder_;
        };

        class ChildKeyValueItemContext final : public CommonContext {
        public:
            ChildKeyValueItemContext(Builder& builder)
                : CommonContext(builder) {}

            Builder& Value(Node::Value value) = delete;
            Builder::ChildDictItemContext StartDict() = delete;
            Builder::ChildArrayItemContext StartArray() = delete;
            Builder& EndArray() = delete;

        };

        class ChildValueItemContext final : public CommonContext {
        public:
            ChildValueItemContext(Builder& builder)
                : CommonContext(builder) {}

            ChildKeyValueItemContext Value(Node::Value value);
            Builder::ChildValueItemContext Key(std::string key) = delete;
            Builder& EndDict() = delete;
            Builder& EndArray() = delete;

        };

        class ChildDictItemContext : public CommonContext {
        public:
            ChildDictItemContext(Builder& builder)
                : CommonContext(builder) {}

            Builder& Value(Node::Value value) = delete;
            Builder::ChildDictItemContext StartDict() = delete;
            Builder::ChildArrayItemContext StartArray() = delete;
            Builder& EndArray() = delete;

        };

        class ChildArrayItemContext : public CommonContext {
        public:
            ChildArrayItemContext(Builder& builder)
                : CommonContext(builder) {}
            ChildArrayItemValueContext Value(Node::Value value);
            Builder::ChildValueItemContext Key(std::string key) = delete;
            Builder& EndDict() = delete;

        };

        class ChildArrayItemValueContext : public CommonContext {
        public:
            ChildArrayItemValueContext(Builder& builder)
                : CommonContext(builder) {}

            ChildValueItemContext Key(std::string key) = delete;
            ChildArrayItemValueContext Value(Node::Value value);
            Builder& EndDict() = delete;

        };
    };

} // namespace json