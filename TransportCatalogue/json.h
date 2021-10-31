#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

class Node;
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

using Value = std::variant<std::nullptr_t, std::string, Array, Dict, int, double, bool>;
using Number = std::variant<int, double>;

class Node : private Value{
    friend bool operator== (const Node& lhs, const Node& rhs);
    friend bool operator!= (const Node& lhs, const Node& rhs);
public:

    using variant::variant;
    using InputValue = variant;

    Node(json::Node::InputValue& value) {
        this->swap(value);
    }

    bool IsNull() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsInt() const;
    bool IsString() const;
    bool IsBool() const;
    bool IsArray() const;
    bool IsMap() const;

    const Array& AsArray() const;
    const Dict& AsMap() const;
    int AsInt() const;
    double AsDouble() const;
    const std::string& AsString() const;
    bool AsBool() const;

    const Value& GetValue() const;
};

class Document {
    friend bool operator== (const Document& lhs, const Document& rhs);
    friend bool operator!= (const Document& lhs, const Document& rhs);
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

private:
    Node root_;
};

Document Load(std::istream& input);

class NodePrinter {
public:
    NodePrinter(std::ostream& output, int depth);    
    void operator()(std::nullptr_t);
    void operator()(std::string value);
    void operator()(double value);
    void operator()(int value);
    void operator()(bool value);
    void operator()(Array arr);
    void operator()(Dict map);
private:
    const std::string tabulation_ = "\t";
    std::ostream& output_;
    int depth_;
};

void Print(const Document& doc, std::ostream& output);

}  // namespace json