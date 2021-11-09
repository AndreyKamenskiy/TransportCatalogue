#pragma once
#include <vector>
#include "svg.h"

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

class MapRenderer {
public:
	MapRenderer(RenderOptions options);

private:
	RenderOptions options_;

};

}