#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {
	using namespace std::literals;

	class Node;

	using Dict = std::map<std::string, Node>;
	using Array = std::vector<Node>;

	class ParsingError : public std::runtime_error {
	public:
		using runtime_error::runtime_error;
	};

	class Node {
	public:
		using Value = std::variant<std::nullptr_t, int, bool, double, std::string, Array, Dict>;

		Node() = default;
		Node(std::nullptr_t value);
		Node(int value);
		Node(bool value);
		Node(double value);
		Node(std::string value);
		Node(Array array);
		Node(Dict map);

		bool IsInt() const;
		bool IsDouble() const;
		bool IsPureDouble() const;
		bool IsBool() const;
		bool IsString() const;
		bool IsNull() const;
		bool IsArray() const;
		bool IsMap() const;

		int AsInt() const;
		bool AsBool() const;
		double AsDouble() const;
		const std::string& AsString() const;
		const Array& AsArray() const;
		const Dict& AsMap() const;

		const Value& GetValue() const {
			return value_;
		}

	private:
		Value value_;
	};

	inline bool operator ==(Node left, Node right) {
		return left.GetValue() == right.GetValue();
	}

	inline bool operator !=(Node left, Node right) {
		return !(left == right);
	}

	class Document {
	public:
		explicit Document(Node root);

		const Node& GetRoot() const;

	private:
		Node root_;
	};

	inline bool operator ==(Document left, Document right) {
		return left.GetRoot() == right.GetRoot();
	}

	inline bool operator !=(Document left, Document right) {
		return !(left == right);
	}

	Document Load(std::istream& input);

	struct PrintContext {
		std::ostream& out;
		int indent_step = 4;
		int indent = 0;

		void PrintIndent() const {
			for (int i = 0; i < indent; ++i) {
				out.put(' ');
			}
		}

		PrintContext Indented() const {
			return { out, indent_step, indent_step + indent };
		}
	};

	template <typename Value>
	inline void PrintValue(const Value& value, const PrintContext& ctx) {
		ctx.out << value;
	}

	void PrintValue(std::nullptr_t, const PrintContext& ctx);
	void PrintValue(const bool& value, const PrintContext& ctx);
	void PrintValue(const std::string& value, const PrintContext& ctx);
	void PrintValue(const Array& array, const PrintContext& ctx);
	void PrintValue(const Dict& dict, const PrintContext& ctx);

	void PrintNode(const Node& node, const PrintContext& ctx);

	void Print(const Document& doc, std::ostream& output);

}  // namespace json