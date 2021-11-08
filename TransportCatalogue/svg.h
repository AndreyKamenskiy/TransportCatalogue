#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <sstream>
#include <optional>
#include <variant>


namespace svg {
    //https://github.com/ERupshis/TrasnportCatalogue/blob/main/svg.h
    //https://github.com/ERupshis/TrasnportCatalogue/blob/main/svg.cpp

    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }
        double x = 0;
        double y = 0;
    };

    struct Rgb {
        Rgb() = default;
        Rgb(uint8_t r, uint8_t g, uint8_t b)
            :red(r), green(g), blue(b)
        {
        }

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    struct Rgba {
        Rgba() = default;
        Rgba(uint8_t r, uint8_t g, uint8_t b, double a)
            :red(r), green(g), blue(b), opacity(a)
        {
        }
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };

    //����� ��� Color �� ��������� ������ �������� monostate, ��������� monostate ������ � ������ ����� variant.
    using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;

    // ������� � ������������ ����� ��������� �� �������������� inline,
    // �� ������� ���, ��� ��� ����� ����� �� ��� ������� ����������,
    // ������� ���������� ���� ���������.
    // � ��������� ������ ������ ������� ���������� ����� ������������ ���� ����� ���� ���������
    inline const Color NoneColor{ "none"};

    struct ColorPrinter {
        std::ostream& out;
        void operator()(std::monostate);
        void operator()(std::string str);
        void operator()(svg::Rgb col);
        void operator()(svg::Rgba col);
    };

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream& operator<<(std::ostream& os, const StrokeLineCap& lineCap);
    std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& lineJoin);
    std::ostream& operator<<(std::ostream& out, const Color& color);

    /*
        ��������� ��� ������� ����� �������, ���� � ����� �������
    */

    template <typename Owner>
    class PathProps {
    public:

        Owner& SetFillColor(Color color) {
            fillColor_ = color;
            return AsOwner();
        }

        Owner& SetStrokeColor(Color color) {
            strokeColor_ = color;
            return AsOwner();
        }

        Owner& SetStrokeWidth(double width) {
            strokeWidth_ = width;
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            lineCap_ = line_cap;
            return AsOwner();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            lineJoin_ = line_join;
            return AsOwner();
        }

    protected:
        virtual ~PathProps() = default;

        template <typename T>
        void FillData(std::ostream& out, std::string_view att_name, const std::optional<T>& att_value) const {
            if (att_value) {
                using namespace std::literals;
                out << ' ' << att_name << "=\""sv << *att_value << "\""sv;
            }
        }

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;
            FillData(out, "fill"sv, fillColor_);
            FillData(out, "stroke"sv, strokeColor_);
            FillData(out, "stroke-width"sv, strokeWidth_);
            FillData(out, "stroke-linecap"sv, lineCap_);
            FillData(out, "stroke-linejoin"sv, lineJoin_);
        }

    private:
        std::optional<StrokeLineCap> lineCap_;
        std::optional<StrokeLineJoin> lineJoin_;
        std::optional<double> strokeWidth_;
        std::optional<Color> strokeColor_;
        std::optional<Color> fillColor_;

        Owner& AsOwner() {
            return static_cast<Owner&>(*this);
        }
    };

    /*
     * ��������������� ���������, �������� �������� ��� ������ SVG-��������� � ���������.
     * ������ ������ �� ����� ������, ������� �������� � ��� ������� ��� ������ ��������
     */
    struct RenderContext {
        RenderContext(std::ostream& out_)
            : out(out_) {
        }

        RenderContext(std::ostream& out_, int indent_step, int indent = 0)
            : out(out_)
            , indent_step(indent_step)
            , indent(indent) {
        }

        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    /*
     * ����������� ������� ����� Object ������ ��� ���������������� ��������
     * ���������� ����� SVG-���������
     * ��������� ������� "��������� �����" ��� ������ ����������� ����
     */

    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    class ObjectContainer {
    public:
        template <typename Obj>
        void Add(Obj obj) {
            AddPtr(std::make_unique<Obj>(std::move(obj)));
        }

        // ��������� � svg-�������� ������-��������� svg::Object
        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

    };

    class Drawable {
    public:
        virtual void Draw(ObjectContainer& container) const = 0;
        virtual ~Drawable() = default;
    };

    /*
     * ����� Circle ���������� ������� <circle> ��� ����������� �����
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
     */
    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };

    /*
     * ����� Polyline ���������� ������� <polyline> ��� ����������� ������� �����
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
     */
    class Polyline final : public Object, public PathProps<Polyline> {
    public:
        // ��������� ��������� ������� � ������� �����
        Polyline& AddPoint(Point point);

    private:
        void RenderObject(const RenderContext& context) const override;

        std::vector<Point> points_;
    };

    /*
     * ����� Text ���������� ������� <text> ��� ����������� ������
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
     */
    class Text final : public Object, public PathProps<Text> {
    public:

        Text() {
            using namespace std::literals::string_literals;
            std::string font_weight_ = ""s;
            std::string font_family_ = ""s;
            std::string text_ = ""s;
        }

        // ����� ���������� ������� ����� (�������� x � y)
        Text& SetPosition(Point pos);

        // ����� �������� ������������ ������� ����� (�������� dx, dy)
        Text& SetOffset(Point offset);

        // ����� ������� ������ (������� font-size)
        Text& SetFontSize(uint32_t size);

        // ����� �������� ������ (������� font-family)
        Text& SetFontFamily(std::string font_family);

        // ����� ������� ������ (������� font-weight)
        Text& SetFontWeight(std::string font_weight);

        // ����� ��������� ���������� ������� (������������ ������ ���� text)
        Text& SetData(std::string data);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point position_ = { 0.0, 0.0 };
        Point offset_ = { 0.0, 0.0 };
        uint32_t font_size_ = 1;
        std::string font_weight_;
        std::string font_family_;
        std::string text_;
    };

    class Document : public ObjectContainer {
    public:
        // ��������� � svg-�������� ������-��������� svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj) override {
            objects_.emplace_back(std::move(obj));
        }

        // ������� � ostream svg-������������� ���������
        void Render(std::ostream& out) const;

        // ������ ������ � ������, ����������� ��� ���������� ������ Document
    private:
        std::vector<std::unique_ptr<Object>> objects_;
    };

}  // namespace svg
