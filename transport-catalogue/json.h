#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

using namespace std::string_view_literals;

namespace json {

    class Node;
    // Для сохранить Dict и Array без изменения
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // Для выбрасывания ошибки при оишбках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node final : private std::variant<std::nullptr_t, bool, int, double, std::string, Dict, Array> {
    public:
        // Сделать доступными все конструкторы родительского варианта класса.
        using variant::variant;
        using Value = variant;

    public:
        bool IsNull() const;
        bool IsBool() const;
        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsString() const;
        bool IsArray() const;
        bool IsMap() const;

        const Value& GetValue() const;
        bool AsBool() const;
        int AsInt() const;
        double AsDouble() const;
        const std::string& AsString() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

        bool operator==(const Node& rhs) const;
        bool operator!=(const Node& rhs) const;
    };

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

        bool operator==(const Document& rhs) const;
        bool operator!=(const Document& rhs) const;

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    // Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
    struct PrintContext {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }
        // Возвращает новый контекст вывода с увеличенным смещением
        PrintContext Indented() const {
            return { out, indent_step, indent_step + indent };
        }
    };

    void PrintValue(std::nullptr_t, const PrintContext& ctx);
    void PrintValue(std::string value, const PrintContext& ctx);
    void PrintValue(bool value, const PrintContext& ctx);
    void PrintValue(Array array, const PrintContext& ctx);
    void PrintValue(Dict dict, const PrintContext& ctx);
    
    // Шаблон для вывода double и int
    template <typename Value>
    void PrintValue(const Value& value, const PrintContext& ctx) {
        ctx.out << value;
    }

    void PrintNode(const Node& node, const PrintContext& ctx);
    void Print(const Document& doc, std::ostream& output);

}  // namespace json