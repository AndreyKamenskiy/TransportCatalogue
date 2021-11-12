#include <stdexcept>
#include <sstream>
#include "json.h"

using namespace std;

namespace json {

    namespace {

        Node LoadNode(istream& input);

        void skipSpacesAndSpec(istream& input) {
            while (true) {
                char ch = input.get();
                //find special symbols
                if (ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r') {
                    continue;
                }
                input.putback(ch); // вернем нормальный символ на место
                break;
            };
        }


        string LoadWord(istream& input) {
            string word{};
            while (true) {
                char ch = input.get();
                if (input.eof()) {
                    break;
                }
                if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
                    word.push_back(ch);
                }
                else {
                    input.putback(ch);
                    break;
                }
            }
            return word;
        }

        Number LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
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
                // После 0 в JSON не могут идти другие цифры
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
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
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return std::stoi(parsed_num);
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadArray(istream& input) {
            Array result;

            bool nodeLoaded = false;
            while (true) {
                skipSpacesAndSpec(input);
                char ch = input.get();
                if (input.eof()) {
                    //дошли ко конца потока, но не нашли ']'
                    throw ParsingError("Array parsing error: absent end of array"s);
                }
                if (ch == ']') {
                    if (!nodeLoaded && !result.empty()) {
                        //case [123,345,]
                        throw ParsingError("Array parsing error: empty Node"s);
                    }
                    break;
                }
                if (ch == ',') {
                    if (!nodeLoaded) {
                        //case [5,,7]
                        throw ParsingError("Array parsing error: empty Node"s);
                    }
                    nodeLoaded = false;
                }
                else {
                    input.putback(ch);
                }
                result.push_back(LoadNode(input));
                nodeLoaded = true;
            }

            return Node(move(result));
        }

        Node LoadString(istream& input) {
            string res;
            while (true) {
                char ch = input.get();
                if (input.eof()) {
                    //дошли ко конца потока, но не нашли "
                    throw ParsingError("String parsing error: absent end of line"s);
                }
                if (ch == '\\') {
                    ch = input.get();
                    if (input.eof()) {
                        //дошли ко конца потока, но не нашли "
                        throw ParsingError("String parsing error: absent end of line"s);
                    }
                    if (ch == 'n') {
                        res.push_back('\n');
                        continue;
                    }
                    else if (ch == 't') {
                        res.push_back('\t');
                        continue;
                    }
                    else if (ch == 'r') {
                        res.push_back('\r');
                        continue;
                    }
                    else if (ch == '"') {
                        res.push_back('"');
                        continue;
                    }
                    else if (ch == '\\') {
                        res.push_back('\\');
                        continue;
                    }
                    else if (ch == '/') {
                        res.push_back('/');
                        continue;
                    }
                }
                if (ch == '"') {
                    break;
                }
                res.push_back(ch);
            }
            return Node(move(res));
        }

        char getNextChar(istream& input) {
            skipSpacesAndSpec(input);
            char ch = input.get();
            if (input.eof()) {
                throw ParsingError("Parsing error: unexpected end of input"s);
            }
            return ch;
        }

        Node LoadDict(istream& input) {
            Dict result;
            char ch = getNextChar(input);
            if (ch == '}') {
                // case {}
                return Node(move(result));
            }
            else {
                input.putback(ch);
            }
            while (true) {
                //load Key
                if (getNextChar(input) != '"') {
                    throw ParsingError("Map parsing error: missing '\"'"s);
                }
                string key = LoadString(input).AsString();
                //search for ':'
                if (getNextChar(input) != ':') {
                    throw ParsingError("Map parsing error: missing ':'"s);
                }
                //load Node and add new pair to the Dict
                result.insert({ move(key), LoadNode(input) });
                // search for '}' or ','
                skipSpacesAndSpec(input);
                char ch = getNextChar(input);
                if (ch == '}') {
                    break;
                }
                else if (ch != ',') {
                    throw ParsingError("Map parsing error: expected ',' or '}' insteed '"s + ch + "'"s);
                }
            }
            return Node(move(result));
        }

        Node LoadNull(istream& input) {
            if (LoadWord(input) != "null"s) {
                throw ParsingError("null parsing error"s);
            }
            return Node{};
        }

        Node LoadBool(istream& input) {
            string word = LoadWord(input);
            if (word == "true"s) {
                return Node(true);
            }
            else if (word == "false"s) {
                return Node(false);
            }
            else {
                throw ParsingError("null parsing error"s);
            }
            return Node{};
        }

        Node LoadNode(istream& input) {
            skipSpacesAndSpec(input);
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
            else if (c == 't' || c == 'f') {
                input.putback(c);
                return LoadBool(input);
            }
            else
            {
                input.putback(c);
                Number variantNumber = LoadNumber(input);
                if (std::holds_alternative<int>(variantNumber)) {
                    return Node(get<int>(variantNumber));
                }
                return Node(get<double>(variantNumber));
            }
        }

    }  // namespace

    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(*this);
    }

    bool Node::IsDouble() const
    {
        return IsPureDouble() || IsInt();
    }

    bool Node::IsPureDouble() const
    {
        return std::holds_alternative<double>(*this);
    }

    bool Node::IsInt() const
    {
        return std::holds_alternative<int>(*this);
    }

    bool Node::IsString() const
    {
        return std::holds_alternative<string>(*this);
    }

    bool Node::IsBool() const
    {
        return std::holds_alternative<bool>(*this);
    }

    bool Node::IsArray() const
    {
        return std::holds_alternative<Array>(*this);
    }

    bool Node::IsMap() const
    {
        return std::holds_alternative<Dict>(*this);
    }

    const Array& Node::AsArray() const {
        if (!std::holds_alternative<Array>(*this)) {
            using namespace std::literals;
            throw std::logic_error("Node is not Array type"s);
        }
        return std::get<Array>(*this);
    }

    const Dict& Node::AsMap() const {
        if (!std::holds_alternative<Dict>(*this)) {
            using namespace std::literals;
            throw std::logic_error("Node is not Dict type"s);
        }
        return std::get<Dict>(*this);
    }

    int Node::AsInt() const {
        if (!std::holds_alternative<int>(*this)) {
            using namespace std::literals;
            throw std::logic_error("Node is not int type"s);
        }
        return std::get<int>(*this);
    }

    double Node::AsDouble() const
    {
        if (IsPureDouble()) {
            return get<double>(*this);
        }
        else if (IsInt()) {
            return static_cast<double>(get<int>(*this));
        }
        using namespace std::literals;
        throw std::logic_error("Node is not a double type"s);
    }

    const string& Node::AsString() const {
        if (!std::holds_alternative<std::string>(*this)) {
            using namespace std::literals;
            throw std::logic_error("Node is not Array type"s);
        }
        return std::get<std::string>(*this);
    }

    bool Node::AsBool() const
    {
        if (!std::holds_alternative<bool>(*this)) {
            using namespace std::literals;
            throw std::logic_error("Node is not bool type"s);
        }
        return std::get<bool>(*this);
    }

    const Value& Node::GetValue() const
    {
        return *this;
    }


    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    bool operator==(const Node& lhs, const Node& rhs)
    {
        if (lhs.IsDouble() && rhs.IsDouble()) {
            return (std::abs(lhs.AsDouble() - rhs.AsDouble())) < 0.00001;
        }
        return lhs.GetValue() == rhs.GetValue();
    }

    bool operator!=(const Node& lhs, const Node& rhs)
    {
        return lhs.GetValue() != rhs.GetValue();
    }

    bool operator==(const Document& lhs, const Document& rhs)
    {
        return lhs.GetRoot() == rhs.GetRoot();
    }

    bool operator!=(const Document& lhs, const Document& rhs)
    {
        return lhs.GetRoot() != rhs.GetRoot();
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    ///////////////////// NodePrinter //////////////////////////////////// 

    NodePrinter::NodePrinter(std::ostream& output, int depth = 1) 
        : output_(output), depth_(depth)
    {
    }

    void NodePrinter::operator()(std::nullptr_t)
    {
        using namespace std::literals;
        output_ << "null"s;
    }

    void NodePrinter::operator()(std::string value)
    {
        output_ << "\""s;
        for (char ch : value) {
            if (ch == '"') {
                output_ << '\\';
            }
            if (ch == '/') {
                output_ << '\\';
            }
            if (ch == '\\') {
                output_ << '\\';
            }
            if (ch == '\n') {
                output_ << "\\n"s;
                continue;
            }
            if (ch == '\t') {
                output_ << "\\t"s;
                continue;
            }
            if (ch == '\r') {
                output_ << "\\r"s;
                continue;
            }

            output_ << ch;
        }
        output_ << '"';
    }

    void NodePrinter::operator()(double value)
    {
        output_ << value;
    }

    void NodePrinter::operator()(int value)
    {
        output_ << value;
    }

    void NodePrinter::operator()(bool value)
    {
        output_ << (value ? "true"s : "false"s);
    }

    void NodePrinter::operator()(Array arr)
    {
        std::string tabs = "\n";
        for (size_t i = 0; i < depth_ - 1; i++)
        {
            tabs += tabulation_;
        }
        output_ << "["s << tabs << tabulation_;
        bool first = true;
        for (const Node& node : arr) {
            if (first) {
                first = false;
            }
            else {
                output_ << ","s << tabs << tabulation_;
            }
            visit(NodePrinter{ output_, depth_ + 1 }, node.GetValue());
        }

        output_ << tabs <<']';
    }

    void NodePrinter::operator()(Dict map)
    {
        std::string tabs = "\n";
        for (size_t i = 0; i < depth_ - 1; i++)
        {
            tabs += tabulation_;
        }
        output_ << "{"s << tabs << tabulation_;
        bool first = true;
        for (auto& [key, node] : map) {
            if (first) {
                first = false;
            }
            else {
                output_ << ","s << tabs<< tabulation_;
            }
            output_ << '"' << key << "\": "s;
            visit(NodePrinter{ output_, depth_ + 1 }, node.GetValue());
        }

        output_ << tabs << '}';
    }

    void Print(const Document& doc, std::ostream& output) {
        visit(NodePrinter{ output }, doc.GetRoot().GetValue());
    }

}  // namespace json