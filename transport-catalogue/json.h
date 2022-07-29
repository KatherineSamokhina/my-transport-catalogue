#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <stack>
#include <memory>

namespace json {

    class Node;

    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    class Node final : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {
    public:
        using variant::variant;
        using Value = variant;

        bool IsInt() const;
        int AsInt() const;

        bool IsPureDouble() const;
        bool IsDouble() const;
        double AsDouble() const;

        bool IsBool() const;
        bool AsBool() const;

        bool IsString() const;
        const std::string& AsString() const;

        bool IsDict() const;

        const Dict& AsDict() const;
        Dict& AsDict();

        bool IsNull() const;

        bool IsArray() const;

        const Array& AsArray() const;
        Array& AsArray();

        const Value& GetValue() const;

        bool operator==(const Node&) const;
        bool operator!=(const Node&) const;
    };

    class Document {
    public:
        explicit Document(Node root);
        const Node& GetRoot() const;

        bool operator==(const Document&) const;
        bool operator!=(const Document&) const;

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json