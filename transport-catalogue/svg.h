#pragma once

#include <cstdint>
#include <deque>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace svg {

    using namespace std::literals;

    // ---------- Rgb ------------------

    struct Rgb {
    public:
        Rgb() = default;
        Rgb(uint8_t r, uint8_t g,
            uint8_t b);

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    // ---------- Rgba ------------------    

    struct Rgba : public Rgb {
        Rgba() = default;
        Rgba(uint8_t r, uint8_t g,
            uint8_t b, double a);

        double opacity = 1.0;
    };

    // ---------- OstreamColor ------------------    

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
    inline const std::string NoneColor("none"s);

    struct OstreamColor {
        void operator()(std::monostate) const;
        void operator()(const std::string& color) const;
        void operator()(const Rgb& rgb) const;
        void operator()(const Rgba& rgba) const;
        std::ostream& out;
    };

    // ---------- StrokeLineCap ------------------    

    enum class StrokeLineCap { BUTT, ROUND, SQUARE };
    std::ostream& operator<<(std::ostream& stream, const StrokeLineCap& stroke_line_cap);

    // ---------- StrokeLineJoin ------------------

    enum class StrokeLineJoin { ARCS, BEVEL, MITER, MITER_CLIP, ROUND };
    std::ostream& operator<<(std::ostream& stream, const StrokeLineJoin& stroke_line_join);

    // ---------- Point ------------------    

    struct Point {
        Point() = default;
        Point(double x, double y);

        double x = 0;
        double y = 0;
    };

    bool operator==(const Point& lhs, const Point& rhs);
    bool operator!=(const Point& lhs, const Point& rhs);

    // ---------- RenderContext ------------------    

    struct RenderContext {
        RenderContext(std::ostream& out);
        RenderContext(std::ostream& out, int indent_step, int indent = 0);

        RenderContext Indented() const;
        void RenderIndent() const;

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    // ---------- PathProps ------------------    

    template <typename Owner>
    class PathProps {
        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_line_cap_;
        std::optional<StrokeLineJoin> stroke_line_join_;

    public:
        Owner& SetFillColor(Color color);
        Owner& SetStrokeColor(Color color);
        Owner& SetStrokeWidth(double width);
        Owner& SetStrokeLineCap(StrokeLineCap line_cap);
        Owner& SetStrokeLineJoin(StrokeLineJoin line_join);

    protected:
        ~PathProps() = default;
        void RenderAttrs(std::ostream& out) const;

    private:
        Owner& AsOwner();
    };

    template <typename Owner>
    Owner& PathProps<Owner>::SetFillColor(Color color) {
        fill_color_ = std::move(color);
        return AsOwner();
    }

    template <typename Owner>
    Owner& PathProps<Owner>::SetStrokeColor(Color color) {
        stroke_color_ = std::move(color);
        return AsOwner();
    }

    template <typename Owner>
    Owner& PathProps<Owner>::SetStrokeWidth(double width) {
        stroke_width_ = std::move(width);
        return AsOwner();
    }

    template <typename Owner>
    Owner& PathProps<Owner>::SetStrokeLineCap(StrokeLineCap line_cap) {
        stroke_line_cap_ = std::move(line_cap);
        return AsOwner();
    }

    template <typename Owner>
    Owner& PathProps<Owner>::SetStrokeLineJoin(StrokeLineJoin line_join) {
        stroke_line_join_ = std::move(line_join);
        return AsOwner();
    }

    template <typename Owner>
    void PathProps<Owner>::RenderAttrs(std::ostream& out) const {
        if (fill_color_) {
            out << " fill=\""sv;
            std::visit(OstreamColor{ out }, *fill_color_);
            out << "\""sv;
        }

        if (stroke_color_) {
            out << " stroke=\""sv;
            std::visit(OstreamColor{ out }, *stroke_color_);
            out << "\""sv;
        }

        if (stroke_width_) {
            out << " stroke-width=\""sv
                << *stroke_width_ << "\""sv;
        }

        if (stroke_line_cap_) {
            out << " stroke-linecap=\""sv
                << *stroke_line_cap_ << "\""sv;
        }

        if (stroke_line_join_) {
            out << " stroke-linejoin=\""sv
                << *stroke_line_join_ << "\""sv;
        }
    }

    template <typename Owner>
    Owner& PathProps<Owner>::AsOwner() {
        return static_cast<Owner&>(*this);
    }

    // ---------- Object ------------------

    class Object {
    public:
        void Render(const RenderContext& context) const;
        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    // ---------- ObjectContainer ------------------    

    class ObjectContainer {
    public:
        template <typename Obj>
        void Add(Obj obj);
        virtual void AddPtr(std::shared_ptr<Object>&& obj) = 0;

    protected:
        virtual ~ObjectContainer() = default;
        std::deque<std::shared_ptr<Object>> objects_;
    };

    template <typename Obj>
    void
        ObjectContainer
        ::Add(Obj obj) {
        objects_.emplace_back(std::make_shared<Obj>(std::move(obj)));
    }

    // ---------- Drawable ------------------    

    class Drawable {
    public:
        virtual void Draw(ObjectContainer& container) const = 0;
        virtual  ~Drawable() = default;
    };

    // ---------- Circle ------------------    

    class Circle final : public Object, public PathProps<Circle> {
        Point center_;
        double radius_ = 1.0;

    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;
    };

    // ---------- Polyline ------------------    

    class Polyline final : public Object, public PathProps<Polyline> {
        std::vector<Point> points_;

    public:
        Polyline& AddPoint(Point point);

    private:
        void RenderObject(const RenderContext& context) const override;
    };

    // ---------- Text ------------------

    enum class TypeChar { QUOT, APOS, LT, GT, AMP, OTHER };

    class Text final : public Object, public PathProps<Text> {
        Point pos_;
        Point offset_;
        uint32_t font_size_ = 1;
        std::optional<std::string> font_family_;
        std::optional<std::string> font_weight_;
        std::string data_;

    public:
        Text& SetPosition(Point pos);
        Text& SetOffset(Point offset);
        Text& SetFontSize(uint32_t size);
        Text& SetFontFamily(const std::string_view font_family);
        Text& SetFontWeight(const std::string_view font_weight);
        Text& SetData(const std::string_view data);

    private:
        void RenderObject(const RenderContext& context) const override;
        TypeChar GetTypeChar(char c) const;
        void ScreenString(std::ostream& out) const;
    };

    // ---------- Document ------------------    

    class Document final : public ObjectContainer {
    public:
        void AddPtr(std::shared_ptr<Object>&& obj) override;
        void Render(std::ostream& out) const;
    };

}