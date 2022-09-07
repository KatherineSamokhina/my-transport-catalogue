#include "json_builder.h"

namespace json {

    Builder::ChildValueItemContext Builder::CommonContext::Key(std::string key) {
        return builder_.Key(key);
    }

    Builder& Builder::CommonContext::Value(Node::Value value) {
        return builder_.Value(value);
    }

    Builder::ChildDictItemContext Builder::CommonContext::StartDict() {
        return builder_.StartDict();
    }

    Builder& Builder::CommonContext::EndDict() {
        return builder_.EndDict();
    }

    Builder::ChildArrayItemContext Builder::CommonContext::StartArray() {
        return builder_.StartArray();
    }

    Builder& Builder::CommonContext::EndArray() {
        return builder_.EndArray();
    }

    Builder::ChildKeyValueItemContext Builder::ChildValueItemContext::Value(Node::Value value) {
        return builder_.Value(value);
    }

    Builder::ChildArrayItemValueContext Builder::ChildArrayItemContext::Value(Node::Value value) {
        return builder_.Value(value);
    }

    Builder::ChildArrayItemValueContext Builder::ChildArrayItemValueContext::Value(Node::Value value) {
        return builder_.Value(value);
    }

    Builder::ChildValueItemContext Builder::Key(std::string key) {
        if (nodes_stack_.empty()) {
            throw std::logic_error("error");
        }
        if (!nodes_stack_.empty() && !nodes_stack_.back()->IsDict()) {
            throw std::logic_error("Attempting to add key, but no json::Dict has opened!");
        }
        auto [inserted_iterator, is_inserted] = nodes_stack_.back()->AsDict().emplace(key, json::Node{ nullptr });
        nodes_stack_.push_back(&(inserted_iterator->second));
        return ChildValueItemContext{ *this };
    }

    Builder& Builder::Value(Node::Value value) {
        if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict()) {
            throw std::logic_error("error");
        }

        if (!no_content_ && nodes_stack_.empty()) {
            throw std::logic_error("error");
        }

        Node current_node;

        if (std::holds_alternative<std::nullptr_t>(value)) {
            current_node = nullptr;
        }
        else if (std::holds_alternative<int>(value)) {
            current_node = std::get<int>(value);
        }
        else if (std::holds_alternative<double>(value)) {
            current_node = std::get<double>(value);
        }
        else if (std::holds_alternative<std::string>(value)) {
            current_node = std::get<std::string>(value);
        }
        else if (std::holds_alternative<json::Array>(value)) {
            current_node = std::get<json::Array>(value);
        }
        else if (std::holds_alternative<json::Dict>(value)) {
            current_node = std::get<json::Dict>(value);
        }
        else {
            throw std::logic_error("error!");
        }

        if (nodes_stack_.empty()) {
            root_ = std::move(current_node);
            no_content_ = false;
        }
        else if (nodes_stack_.back()->IsNull()) {
            *nodes_stack_.back() = std::move(current_node);
            nodes_stack_.pop_back();

        }
        else if (nodes_stack_.back()->IsArray()) {
            nodes_stack_.back()->AsArray().push_back(std::move(current_node));

        }
        else {
            throw std::logic_error("error");
        }
        return *this;
    }

    Builder::ChildDictItemContext Builder::StartDict() {
        return std::get<Builder::ChildDictItemContext>(StartCollection(json::Node{ json::Dict{} }));
    }

    Builder::ChildArrayItemContext Builder::StartArray() {
        return std::get<Builder::ChildArrayItemContext>(StartCollection(json::Node{ json::Array{} }));
    }

    std::variant<Builder::ChildDictItemContext, Builder::ChildArrayItemContext> Builder::StartCollection(json::Node node) {
        if (nodes_stack_.empty() && no_content_) {
            root_ = std::move(node);
            nodes_stack_.emplace_back(&root_);
        }
        else if (!nodes_stack_.empty() && nodes_stack_.back()->IsNull()) {
            *nodes_stack_.back() = std::move(node);
        }
        else if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
            Node inserted_node = nodes_stack_.back()->AsArray().emplace_back(node);
            nodes_stack_.push_back(&inserted_node);

        }
        else {
            throw std::logic_error("error");
        }
        if (node.IsArray()) {
            return ChildArrayItemContext{ *this };
        }
        else if (node.IsDict()) {
            return ChildDictItemContext{ *this };
        }
        else {
            throw std::logic_error("bad node in return Builder::StartCollection().");
        }
    }

    Builder& Builder::EndDict() {
        if (!nodes_stack_.empty() && !nodes_stack_.back()->IsDict()) {
            throw std::logic_error("error");
        }
        nodes_stack_.pop_back();
        no_content_ = false;
        return *this;
    }

    Builder& Builder::EndArray() {
        if (!nodes_stack_.empty() && !nodes_stack_.back()->IsArray()) {
            throw std::logic_error("error");
        }
        nodes_stack_.pop_back();
        no_content_ = false;
        return *this;
    }

    Node Builder::Build() {
        if (!nodes_stack_.empty()) {
            throw std::logic_error("error");
        }
        if (no_content_) {
            throw std::logic_error("error");
        }
        return root_;
    }
} // namespace json