#include "svg.h"

namespace svg {

    using namespace std::literals;

    // ---------- Object ------------------

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Document ------------------

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;

        RenderContext ctx(out, 2, 2);
        for (const std::unique_ptr<Object>& curr : objects_) {
            curr->Render(ctx);
        }
        std::cout << "</svg>"sv << std::endl;
    }

    // ---------- Polyline ------------------

    Polyline& Polyline::AddPoint(Point point)
    {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        bool first = true;
        for (size_t i = 0; i < points_.size(); ++i) {
            if (first) {
                first = false;
            }
            else {
                out << ' ';
            }
            out << points_[i].x << ',' << points_[i].y;

        }
        out << "\" "sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Text ------------------

    Text& Text::SetPosition(Point pos)
    {
        position_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset)
    {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size)
    {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family)
    {
        font_family_ = std::move(font_family);
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight)
    {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    Text& Text::SetData(std::string data)
    {
        text_ = std::move(data);
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const
    {
        auto& out = context.out;
        out << "<text x=\""sv << position_.x << "\" y=\""sv << position_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
        out << "font-size=\""sv << font_size_ << "\""sv;
        if (!font_family_.empty()) {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }
        RenderAttrs(out);
        out << ">"sv;
        out << text_;
        out << "</text>"sv;
    }

    // ---------- toString functions ------------------

    std::ostream& operator<<(std::ostream& os, const StrokeLineCap& lineCap) {
        using namespace std::literals::string_literals;
        if (lineCap == StrokeLineCap::BUTT) {
            os << "butt"s;
        }
        else if (lineCap == StrokeLineCap::ROUND) {
            os << "round"s;
        }
        else if (lineCap == StrokeLineCap::SQUARE) {
            os << "square"s;
        }
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& lineJoin) {
        using namespace std::literals::string_literals;
        if (lineJoin == StrokeLineJoin::ARCS) {
            os << "arcs"s;
        }
        else if (lineJoin == StrokeLineJoin::BEVEL) {
            os << "bevel"s;
        }
        else if (lineJoin == StrokeLineJoin::MITER) {
            os << "miter"s;
        }
        else if (lineJoin == StrokeLineJoin::MITER_CLIP) {
            os << "miter-clip"s;
        }
        else if (lineJoin == StrokeLineJoin::ROUND) {
            os << "round"s;
        }
        return os;
    }

    std::ostream& operator<<(std::ostream& out, const Color& color) {
        std::visit(ColorPrinter{ out }, color);
        return out;
    }

    // ---------- ColorPrinter ------------------

    void ColorPrinter::operator()(std::monostate) {
        using namespace std::literals;
        out << "none"s;
    }

    void ColorPrinter::operator()(std::string str) {
        out << str;
    }

    void ColorPrinter::operator()(svg::Rgb col) {
        using namespace std::literals;
        out << "rgb("s << std::to_string(col.red) << ',' << std::to_string(col.green) << ',' << std::to_string(col.blue) << ')';
    }

    void ColorPrinter::operator()(svg::Rgba col) {
        using namespace std::literals;
        out << "rgba("s << std::to_string(col.red) << ',' << std::to_string(col.green) << ',' << std::to_string(col.blue) << ',' << col.opacity << ')';
    }

}  // namespace svg