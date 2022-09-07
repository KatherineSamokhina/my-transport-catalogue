#include "svg.h"

namespace svg {

    using namespace std::literals;

    std::ostream& operator<<(std::ostream& out, const Point& point) {
        out << point.x << "," << point.y;
        return out;
    }

    std::ostream& operator<<(std::ostream& out, StrokeLineCap linecap) {
        switch (linecap)
        {
        case StrokeLineCap::BUTT:
            out << "butt";
            break;
        case StrokeLineCap::ROUND:
            out << "round";
            break;
        case StrokeLineCap::SQUARE:
            out << "square";
            break;
        default:
            break;
        }
        return out;
    }
    std::ostream& operator<<(std::ostream& out, StrokeLineJoin linejoin) {
        switch (linejoin)
        {
        case StrokeLineJoin::ARCS:
            out << "arcs";
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel";
            break;
        case StrokeLineJoin::MITER:
            out << "miter";
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip";
            break;
        case StrokeLineJoin::ROUND:
            out << "round";
            break;
        default:
            break;
        }
        return out;
    }

    // Выполняет линейную интерполяцию значения от from до to в зависимости от параметра t
    uint8_t Lerp(uint8_t from, uint8_t to, double t) {
        return static_cast<uint8_t>(std::round((to - from) * t + from));
    }

    // Выполняет линейную интерполяцию Rgb цвета от from до to в зависимости от параметра t
    svg::Rgb Lerp(svg::Rgb from, svg::Rgb to, double t) {
        return { Lerp(from.red, to.red, t), Lerp(from.green, to.green, t), Lerp(from.blue, to.blue, t) };
    }

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
        out << "r=\""sv << radius_ << "\""sv;
        // Выводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // -------- Poluline --------------

    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(std::move(point));
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\"";
        bool is_first_point = true;
        for (const Point& point : points_) {
            if (!is_first_point) {
                out << " ";
            }
            out << point;
            is_first_point = false;
        }
        out << "\"";
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- text ------------------------
    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text x=\""sv << position_.x << "\" y=\""sv << position_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\""sv;
        out << " font-size=\""sv << size_ << "\""sv;
        if (!font_family_.empty()) {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }
        RenderAttrs(context.out);
        out << ">"sv << data_ << "</text>"sv;
    }

    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& Text::SetPosition(Point pos) {
        position_ = std::move(pos);
        return *this;
    }

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    // Задаёт размеры шрифта (атрибут font-size)
    Text& Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    // Задаёт название шрифта (атрибут font-family)
    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = std::move(font_family);
        return *this;
    }

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& Text::SetData(std::string data) {
        data_ = std::move(TransformText(std::move(data)));
        return *this;
    }

    std::string Text::TransformText(std::string text) {

        size_t pos = text.find_first_of("\"'<>&");
        while (pos != text.npos) {

            if (text[pos] == '"') {
                text.erase(pos, 1);
                text.insert(pos, "&quot;");
                pos += 6;
            }
            else if (text[pos] == '\'') {
                text.erase(pos, 1);
                text.insert(pos, "&apos;");
                pos += 6;
            }
            else if (text[pos] == '<') {
                text.erase(pos, 1);
                text.insert(pos, "&lt;");
                pos += 4;
            }
            else if (text[pos] == '>') {
                text.erase(pos, 1);
                text.insert(pos, "&gt;");
                pos += 4;
            }
            else if (text[pos] == '&') {
                text.erase(pos, 1);
                text.insert(pos, "&amp;");
                pos += 5;
            }
            else {
                continue;
            }
            pos = text.find_first_of("\"'<>&", pos);
        }
        return text;
    }
    // ------------ Document ----------


    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        RenderContext context(out, 1, 2);
        for (const auto& object : objects_) {


            object.get()->Render(context);
        }
        out << "</svg>"sv;
    }


    // ----------OstreamColorPtinter-------------
    void OstreamColorPrinter::operator()(std::monostate) {
        out << "none";
    }
    void OstreamColorPrinter::operator()(const std::string& color) {
        out << color;
    }
    void OstreamColorPrinter::operator()(const Rgb color) {
        out << "rgb(" << +color.red << "," << +color.green << "," << +color.blue << ")";
    }
    void OstreamColorPrinter::operator()(const Rgba color) {
        out << "rgba(" << +color.red << "," << +color.green << "," << +color.blue << "," << color.opacity << ")";
    }

    std::ostream& operator<<(std::ostream& out, const Color& color) {
        std::visit(OstreamColorPrinter{ out }, color);
        return out;
    }

}  // namespace svg