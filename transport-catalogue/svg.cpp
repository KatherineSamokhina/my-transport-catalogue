#include "svg.h"

namespace svg {

    using namespace std;
    using namespace std::literals;

    // ---------- Rgb ------------------    

    Rgb::Rgb(uint8_t r, uint8_t g,
        uint8_t b)
        : red(r)
        , green(g)
        , blue(b) {}

    // ---------- Rgba ------------------  

    Rgba::Rgba(uint8_t r, uint8_t g,
        uint8_t b, double a)
        : Rgb(r, g, b)
        , opacity(a) {}

    // ---------- OstreamColor ------------------  

    void OstreamColor::operator()(monostate) const {
        out << NoneColor;
    }

    void OstreamColor::operator()(const string& color) const {
        out << color;
    }

    void OstreamColor::operator()(const Rgb& rgb) const {
        out << "rgb("sv
            << static_cast<int>(rgb.red) << ","sv
            << static_cast<int>(rgb.green) << ","sv
            << static_cast<int>(rgb.blue) << ")"sv;
    }

    void OstreamColor::operator()(const Rgba& rgba) const {
        out << "rgba("sv
            << static_cast<int>(rgba.red) << ","sv
            << static_cast<int>(rgba.green) << ","sv
            << static_cast<int>(rgba.blue) << ","sv
            << rgba.opacity << ")"sv;
    }

    // ---------- StrokeLineCap ------------------   

    ostream& operator<<(ostream& stream, const StrokeLineCap& stroke_line_cap) {
        switch (stroke_line_cap) {
        case StrokeLineCap::BUTT:
            stream << "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            stream << "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            stream << "square"sv;
            break;
        }
        return stream;
    }

    // ---------- StrokeLineJoin ------------------

    ostream& operator<<(ostream& stream, const StrokeLineJoin& stroke_line_join) {
        switch (stroke_line_join) {
        case StrokeLineJoin::ARCS:
            stream << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL:
            stream << "bevel"sv;
            break;
        case StrokeLineJoin::MITER:
            stream << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP:
            stream << "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND:
            stream << "round"sv;
            break;
        }
        return stream;
    }

    // ---------- Point ------------------    

    Point::Point(double x, double y)
        : x(x)
        , y(y) {}

    bool operator==(const Point& lhs, const Point& rhs) {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }

    bool operator!=(const Point& lhs, const Point& rhs) {
        return !(lhs == rhs);
    }

    // ---------- RenderContext ------------------    

    RenderContext::RenderContext(ostream& out)
        : out(out) {}

    RenderContext::RenderContext(ostream& out, int indent_step, int indent)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {}

    RenderContext RenderContext::Indented() const {
        return { out, indent_step, indent + indent_step };
    }

    void RenderContext::RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    // ---------- Object ------------------

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();
        RenderObject(context);
        context.out << endl;
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
        out << "<circle cx=\""sv << center_.x
            << "\" cy=\""sv << center_.y << "\" "sv
            << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Polyline ------------------

    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        for (auto it = points_.begin(); it != points_.end(); ++it) {
            out << it->x << ","sv << it->y;
            if (it + 1 != points_.end()) {
                out << " "sv;
            }
        }
        out << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Text ------------------  

    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(const string_view font_family) {
        font_family_ = static_cast<string>(font_family);
        return *this;
    }

    Text& Text::SetFontWeight(const string_view font_weight) {
        font_weight_ = static_cast<string>(font_weight);
        return *this;
    }

    Text& Text::SetData(const string_view data) {
        data_ = static_cast<string>(data);
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text"sv;
        RenderAttrs(out);
        out << " x=\""sv << pos_.x
            << "\" y=\""sv << pos_.y << "\" "sv
            << "dx=\""sv << offset_.x
            << "\" dy=\""sv << offset_.y << "\" "sv
            << "font-size=\""sv << font_size_ << "\""sv;
        if (font_family_) {
            out << " font-family=\""sv << *font_family_ << "\""sv;
        }
        if (font_weight_) {
            out << " font-weight=\""sv << *font_weight_ << "\""sv;
        }
        out << ">"sv;
        ScreenString(out);
        out << "</text>"sv;
    }

    TypeChar Text::GetTypeChar(char c) const {
        switch (c) {
        case '\"':
            return TypeChar::QUOT;
        case '\'':
            return TypeChar::APOS;
        case '<':
            return TypeChar::LT;
        case '>':
            return TypeChar::GT;
        case '&':
            return TypeChar::AMP;
        default:
            return TypeChar::OTHER;
        }
    }

    void Text::ScreenString(ostream& out) const {
        for (char c : data_) {
            switch (GetTypeChar(c)) {
            case TypeChar::QUOT:
                out << "&quot;"sv;
                break;
            case TypeChar::APOS:
                out << "&apos;"sv;
                break;
            case TypeChar::LT:
                out << "&lt;"sv;
                break;
            case TypeChar::GT:
                out << "&gt;"sv;
                break;
            case TypeChar::AMP:
                out << "&amp;"sv;
                break;
            case TypeChar::OTHER:
                out << c;
                break;
            }
        }
    }

    // ---------- Document ------------------ 

    void Document::AddPtr(std::shared_ptr<Object>&& obj) {
        objects_.emplace_back(move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;
        for (const auto& obj : objects_) {
            obj->Render(RenderContext(out, 2, 2));
        }
        out << "</svg>"sv;
    }
}