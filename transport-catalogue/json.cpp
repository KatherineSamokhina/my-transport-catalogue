#include "json.h"

using namespace std;

namespace json {

	namespace {

		Node LoadNode(istream& input);

		Node LoadArray(istream& input) {
			Array result;
			auto it = std::istreambuf_iterator<char>(input);
			auto end = std::istreambuf_iterator<char>();
			if (it == end) {
				throw ParsingError("Array parsing error");
			}
			for (char c; input >> c && c != ']';) {
				if (c != ',') {
					input.putback(c);
				}
				result.push_back(LoadNode(input));
			}

			return Node(move(result));
		}

		Node LoadNumber(istream& input) {
			std::string parsed_num;

			auto read_char = [&parsed_num, &input] {
				parsed_num += static_cast<char>(input.get());
				if (!input) {
					throw ParsingError("Failed to read number from stream"s);
				}
			};

			auto read_digits = [&input, read_char] {
				if (!std::isdigit(input.peek())) {
					throw ParsingError("A digit is expected"s);
				}
				while (std::isdigit(input.peek())) {
					read_char();
				}
			};

			if (input.peek() == '-') {
				read_char();
			}

			if (input.peek() == '0') {
				read_char();
			}
			else {
				read_digits();
			}

			bool is_int = true;

			if (input.peek() == '.') {
				read_char();
				read_digits();
				is_int = false;
			}

			if (int ch = input.peek(); ch == 'e' || ch == 'E') {
				read_char();
				if (ch = input.peek(); ch == '+' || ch == '-') {
					read_char();
				}
				read_digits();
				is_int = false;
			}

			try {
				if (is_int) {
					try {
						return Node(std::stoi(parsed_num));
					}
					catch (...) {

					}
				}
				return Node(std::stod(parsed_num));
			}
			catch (...) {
				throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
			}
		}

		Node LoadString(istream& input) {
			auto it = std::istreambuf_iterator<char>(input);
			auto end = std::istreambuf_iterator<char>();
			std::string s;
			while (true) {
				if (it == end) {
					throw ParsingError("String parsing error");
				}
				const char ch = *it;
				if (ch == '"') {
					++it;
					break;
				}
				else if (ch == '\\') {
					++it;
					if (it == end) {
						throw ParsingError("String parsing error");
					}
					const char escaped_char = *(it);

					switch (escaped_char) {
					case 'n':
						s.push_back('\n');
						break;
					case 't':
						s.push_back('\t');
						break;
					case 'r':
						s.push_back('\r');
						break;
					case '"':
						s.push_back('"');
						break;
					case '\\':
						s.push_back('\\');
						break;
					default:
						throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
					}
				}
				else if (ch == '\n' || ch == '\r') {
					throw ParsingError("Unexpected end of line"s);
				}
				else {
					s.push_back(ch);
				}
				++it;
			}

			return Node(std::move(s));
		}

		Node LoadDict(istream& input) {
			Dict result;
			auto it = std::istreambuf_iterator<char>(input);
			auto end = std::istreambuf_iterator<char>();
			if (it == end) {
				throw ParsingError("Map parsing error");
			}
			for (char c; input >> c && c != '}';) {
				if (c == ',') {
					input >> c;
				}

				string key = LoadString(input).AsString();
				input >> c;
				result.insert({ move(key), LoadNode(input) });
			}

			return Node(move(result));
		}

		Node LoadNull(istream& input) {
			char c;
			string word = ""s;
			while (input >> c && word != "null"s) {
				word += c;
			}
			if (word != "null"s) {
				throw ParsingError("Parsing null error");
			}
			input.putback(c);
			return Node();
		}
		Node LoadTrue(istream& input) {
			char c;
			string word = ""s;
			while (input >> c && word != "true"s) {
				word += c;
			}
			if (word != "true"s) {
				throw ParsingError("Parsing true error");
			}
			input.putback(c);
			return Node(true);
		}
		Node LoadFalse(istream& input) {
			char c;
			string word = ""s;
			while (input >> c && word != "false"s) {
				word += c;
			}
			if (word != "false"s) {
				throw ParsingError("Parsing false error");
			}
			input.putback(c);
			return Node(false);
		}

		Node LoadNode(istream& input) {
			char c;
			input >> c;

			if (c == '[') {
				return LoadArray(input);
			}
			else if (c == '{') {
				return LoadDict(input);
			}
			else if (c == '"') {
				return LoadString(input);
			}
			else if (c == 'n') {
				input.putback(c);
				return LoadNull(input);
			}
			else if (c == 't') {
				input.putback(c);
				return LoadTrue(input);
			}
			else if (c == 'f') {
				input.putback(c);
				return LoadFalse(input);
			}
			else {
				input.putback(c);
				return LoadNumber(input);
			}
		}

	}  // namespace

	Node::Node(std::nullptr_t value)
		: value_(value) {
	}

	Node::Node(int value)
		: value_(value) {
	}

	Node::Node(bool value)
		: value_(value) {
	}

	Node::Node(double value)
		: value_(value) {
	}

	Node::Node(string value)
		: value_(std::move(value)) {
	}
	Node::Node(Array array)
		: value_(std::move(array)) {
	}

	Node::Node(Dict map)
		: value_(std::move(map)) {
	}



	bool Node::IsInt() const {
		return holds_alternative<int>(value_);
	}

	bool Node::IsDouble() const {
		return holds_alternative<int>(value_) || holds_alternative<double>(value_);
	}

	bool Node::IsPureDouble() const {
		return holds_alternative<double>(value_);
	}

	bool Node::IsBool() const {
		return holds_alternative<bool>(value_);
	}

	bool Node::IsString() const {
		return holds_alternative<string>(value_);
	}

	bool Node::IsNull() const {
		return holds_alternative<nullptr_t>(value_);
	}

	bool Node::IsArray() const {
		return holds_alternative<Array>(value_);
	}

	bool Node::IsMap() const {
		return holds_alternative<Dict>(value_);
	}

	int Node::AsInt() const {
		if (!IsInt()) {
			throw std::logic_error("logic error");
		}
		return get<int>(value_);
	}

	bool Node::AsBool() const
	{
		if (!IsBool()) {
			throw std::logic_error("logic error");
		}
		return get<bool>(value_);
	}

	double Node::AsDouble() const
	{
		if (!IsDouble()) {
			throw std::logic_error("logic error");
		}
		if (IsInt()) {
			return static_cast<double>(get<int>(value_));
		}
		return get<double>(value_);
	}

	const string& Node::AsString() const {
		if (!IsString()) {
			throw std::logic_error("logic error");
		}
		return get<string>(value_);
	}

	const Array& Node::AsArray() const {
		if (!IsArray()) {
			throw std::logic_error("logic error");
		}
		return get<Array>(value_);
	}

	const Dict& Node::AsMap() const {
		if (!IsMap()) {
			throw std::logic_error("logic error");
		}
		return get<Dict>(value_);
	}

	Document::Document(Node root)
		: root_(move(root)) {
	}

	const Node& Document::GetRoot() const {
		return root_;
	}

	Document Load(istream& input) {
		return Document{ LoadNode(input) };
	}

	void PrintValue(std::nullptr_t, const PrintContext& ctx) {
		ctx.out << "null"sv;
	}

	void PrintValue(const bool& value, const PrintContext& ctx) {
		ctx.out << std::boolalpha << value;
	}

	void PrintValue(const std::string& value, const PrintContext& ctx) {
		ctx.out << "\""sv;
		for (const char c : value) {
			if (c == '\r') {
				ctx.out << "\\r"sv;
			}
			else if (c == '\n') {
				ctx.out << "\\n"sv;
			}
			else if (c == '\\' || c == '\"') {
				ctx.out << "\\"sv;
				ctx.out << c;
			}
			else {
				ctx.out << c;
			}
		}
		ctx.out << "\""sv;
	}

	void PrintValue(const Array& array, const PrintContext& ctx) {
		ctx.out << "[\n"sv;
		for (size_t i = 0; i < array.size(); ++i) {
			if (i > 0) { ctx.out << ",\n"sv; }
			PrintNode(array[i], ctx);
		}
		ctx.out << "\n]"sv;
	}

	void PrintValue(const Dict& dict, const PrintContext& ctx) {
		ctx.out << "{\n"sv;
		int i = 0;
		for (const auto& [key, value] : dict) {
			if (i > 0) {
				ctx.out << ",\n"s;
			}
			++i;

			PrintNode(key, ctx);
			ctx.out << ":"sv;
			PrintNode(value, ctx);
		}
		ctx.out << "\n}"sv;
	}

	void PrintNode(const Node& node, const PrintContext& ctx) {
		std::visit(
			[&ctx](const auto& value) { PrintValue(value, ctx); },
			node.GetValue());
	}

	void Print(const Document& doc, std::ostream& output) {
		PrintNode(doc.GetRoot(), { output });
	}

}  // namespace json