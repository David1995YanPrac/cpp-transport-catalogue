#include "json.h"

using namespace std;

namespace json {

    namespace {

        using Number = std::variant<int, double>;

        Node LoadNode(istream& input);

        Node LoadNull(istream& input) {
            const string nameNull = "null";
            for (size_t i = 0; i < nameNull.size(); i++) {
                if (nameNull.at(i) == input.get()) continue;
                else throw ParsingError("Null parsing error");
            }
            return {};
        }

        // Считывает инф на входе строкового литерала JSON док после считывания открывающего символа ":
        std::string LoadString(std::istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Тут выбрасываем ParsingError если поток закончился до того, как встретили закрывающую кавычку
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Считали закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // Считали начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
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
                        // Считали неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Считываем символ и помещаем его в строку
                    s.push_back(ch);
                }
                ++it;
            }

            return s;
        }

        Node LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Тут считываем в parsed_num следующий символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Тут считываем одну или более цифр в parsed_num из input
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
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON другие цифры не могут идти
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробн часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим exp часть числа
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
                        return std::stoi(parsed_num);
                    }
                    catch (...) {
                        // В случае неудачи ниже пробуем преобразовать строку в double
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadBool(istream& input) {
            const string nameFalse = "false";
            const string nameTrue = "true";
            char c = input.get();
            bool value = (c == 't');
            std::string const* name = value ? &nameTrue : &nameFalse;
            for (size_t i = 1; i < name->size(); i++) {
                if (name->at(i) == input.get()) continue;
                else throw ParsingError("Bool parsing error");
            }
            return Node(value);
        }

        Node LoadArray(istream& input) {
            Array result;
            if (input.peek() == -1) throw ParsingError("Array parsing error");

            for (char c; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }

            return Node(std::move(result));
        }

        Node LoadDict(istream& input) {
            Dict result;
            if (input.peek() == -1) throw ParsingError("Array parsing error");

            for (char c; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input);
                input >> c;
                result.insert({ std::move(key), LoadNode(input) });
            }

            return Node(std::move(result));
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (c == 'n') {
                input.putback(c);
                return LoadNull(input);
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else if (c == 't' || c == 'f') {
                input.putback(c);
                return LoadBool(input);
            }
            else if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else {
                input.putback(c);
                return LoadNumber(input);
            }
        }

    }  // namespace

    // IS - AS
    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(*this);
    }
    bool Node::IsBool() const {
        return std::holds_alternative<bool>(*this);
    }
    bool Node::IsInt() const {
        return std::holds_alternative<int>(*this);
    }
    bool Node::IsDouble() const {
        return std::holds_alternative<double>(*this) || std::holds_alternative<int>(*this);
    }
    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(*this);
    }
    bool Node::IsString() const {
        return std::holds_alternative<std::string>(*this);
    }
    bool Node::IsArray() const {
        return std::holds_alternative<Array>(*this);
    }
    bool Node::IsMap() const {
        return std::holds_alternative<Dict>(*this);
    }

    const Node::Value& Node::GetValue() const {
        return *this;
    }

    bool Node::AsBool() const {
        if (auto* value = std::get_if<bool>(this))
            return *value;

        throw std::logic_error("Impossible to parse node as Boolean"s);
    }

    int Node::AsInt() const {
        if (auto* value = std::get_if<int>(this))
            return *value;
        throw std::logic_error("Impossible to parse node as Int "s);
    }

    double Node::AsDouble() const {
        if (auto* value = std::get_if<double>(this))
            return *value;

        if (auto* value = std::get_if<int>(this))
            return static_cast<double>(*value);

        throw std::logic_error("Impossible to parse node as Double "s);
    }

    const std::string& Node::AsString() const {
        if (auto* value = std::get_if<std::string>(this))
            return *value;
        throw std::logic_error("Impossible to parse node as String"s);
    }

    const Array& Node::AsArray() const {
        if (auto* value = std::get_if<Array>(this))
            return *value;
        throw std::logic_error("Impossible to parse node as Array"s);
    }

    const Dict& Node::AsMap() const {
        if (auto* value = std::get_if<Dict>(this))
            return *value;
        throw std::logic_error("Impossible to parse node as Dict"s);
    }

    bool Node::operator==(const Node& rhs) const {
        return *this == rhs.GetValue();
    }

    bool Node::operator!=(const Node& rhs) const {
        return !(*this == rhs.GetValue());
    }

    // Documents
    Document::Document(Node root)
        : root_(std::move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    bool Document::operator==(const Document& rhs) const {
        return root_ == rhs.root_;
    }

    bool Document::operator!=(const Document& rhs) const {
        return !(root_ == rhs.root_);
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void PrintValue(std::nullptr_t, const PrintContext& ctx) {
        ctx.out << "null"sv;
    }

    void PrintValue(std::string value, const PrintContext& ctx) {
        ctx.out << "\""sv;
        for (const char& c : value) {
            if (c == '\n') {
                ctx.out << "\\n"sv;
                continue;
            }
            if (c == '\r') {
                ctx.out << "\\r"sv;
                continue;
            }
            if (c == '\"') ctx.out << "\\"sv;
            if (c == '\t') {
                ctx.out << "\t"sv;
                continue;
            }
            if (c == '\\') ctx.out << "\\"sv;
            ctx.out << c;
        }
        ctx.out << "\""sv;
    }

    void PrintValue(bool value, const PrintContext& ctx) {
        ctx.out << std::boolalpha << value;
    }

    void PrintValue(Array array, const PrintContext& ctx) {
        ctx.out << "[\n"sv;
        auto inner_ctx = ctx.Indented();
        bool first = true;
        for (const auto& elem : array) {
            if (first) first = false;
            else ctx.out << ",\n"s;
            inner_ctx.PrintIndent();
            PrintNode(elem, inner_ctx);
        }
        ctx.out << "\n"s;
        ctx.PrintIndent();
        ctx.out << "]"sv;
    }

    void PrintValue(Dict dict, const PrintContext& ctx) {
        ctx.out << "{\n"sv;
        auto inner_ctx = ctx.Indented();
        bool first = true;
        for (auto& [key, node] : dict) {
            if (first) first = false;
            else ctx.out << ",\n"s;
            inner_ctx.PrintIndent();
            PrintValue(key, ctx);
            ctx.out << ": ";
            PrintNode(node, inner_ctx);
        }
        ctx.out << "\n"s;
        ctx.PrintIndent();
        ctx.out << "}"sv;
    }

    void PrintNode(const Node& node, const PrintContext& ctx) {
        std::visit(
            [&ctx](const auto& value) { PrintValue(value, ctx); },
            node.GetValue());
    }

    void Print(const Document& doc, std::ostream& output) {
        PrintContext ctx{ output };
        PrintNode(doc.GetRoot(), ctx);
    }

}  // namespace json