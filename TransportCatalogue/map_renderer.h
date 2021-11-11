#pragma once
#include <vector>
#include <algorithm>
#include "svg.h"
#include "domain.h"

namespace renderer {
using svg::Document;
using svg::Color;
using svg::Point;
using svg::Rgb;
using svg::Rgba;

/*
width и height — ключи, которые задают ширину и высоту в пикселях. Вещественное число в диапазоне от 0 до 100000.
padding — отступ краёв карты от границ SVG-документа. Вещественное число не меньше 0 и меньше min(width, height)/2.
line_width — толщина линий, которыми рисуются автобусные маршруты. Вещественное число в диапазоне от 0 до 100000.
stop_radius — радиус окружностей, которыми обозначаются остановки. Вещественное число в диапазоне от 0 до 100000.
bus_label_font_size — размер текста, которым написаны названия автобусных маршрутов. Целое число в диапазоне от 0 до 100000.
bus_label_offset — смещение надписи с названием маршрута относительно координат конечной остановки на карте. Массив из двух элементов типа double. Задаёт значения свойств dx и dy SVG-элемента <text>. Элементы массива — числа в диапазоне от –100000 до 100000.
stop_label_font_size — размер текста, которым отображаются названия остановок. Целое число в диапазоне от 0 до 100000.
stop_label_offset — смещение названия остановки относительно её координат на карте. Массив из двух элементов типа double. Задаёт значения свойств dx и dy SVG-элемента <text>. Числа в диапазоне от –100000 до 100000.
underlayer_color — цвет подложки под названиями остановок и маршрутов. Формат хранения цвета будет ниже.
underlayer_width — толщина подложки под названиями остановок и маршрутов. Задаёт значение атрибута stroke-width элемента <text>. Вещественное число в диапазоне от 0 до 100000.
color_palette — цветовая палитра. Непустой массив.
*/
struct RenderOptions {
	double width;
	double height;
	double padding;
	double line_width;
	double stop_radius;
	int bus_label_font_size;
	svg::Point bus_label_offset;
	int stop_label_font_size;
	svg::Point stop_label_offset;
	svg::Color underlayer_color;
	double underlayer_width;
	std::vector<svg::Color> color_palette;
};

class SphereProjector {
public:
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width,
        double max_height, double padding)
        : padding_(padding) {
        if (points_begin == points_end) {
            return;
        }

        const auto [left_it, right_it]
            = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
            return lhs.lng < rhs.lng;
                });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        const auto [bottom_it, top_it]
            = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
            return lhs.lat < rhs.lat;
                });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom) {
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom) {
            zoom_coeff_ = *height_zoom;
        }
    }

    SphereProjector() = default;

    svg::Point operator()(geo::Coordinates coords) const {
        return { (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_ };
    }

private:
    double padding_ = 0;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};


class MapRenderer {
public:

    //Конструктор рендера. ему передаются свойства
	MapRenderer(RenderOptions options);

    //На вход методу передается ссылка на массив остновок.
    //метод инициализрует преобразующий проэктор переводящий
    void InitProjector(const std::vector<domain::Coordinates>& points);

    //Создает полилинию рисующую маршрут.
    void RenderRoute(svg::Document& doc, const domain::Route* route, size_t count);

    //Создает надписи маршрут.
    void RenderRouteLables(svg::Document& doc, const domain::Route* route, size_t count);


private:
	RenderOptions options_;
    SphereProjector projector_;
};

}