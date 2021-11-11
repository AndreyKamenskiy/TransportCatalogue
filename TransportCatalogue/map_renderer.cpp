#include "map_renderer.h"
#include "domain.h"
#include "svg.h"

renderer::MapRenderer::MapRenderer(RenderOptions options): options_(std::move(options))
{
}

void renderer::MapRenderer::InitProjector(const std::vector<domain::Coordinates>& points)
{
	projector_ = SphereProjector{ points.begin(), points.end(),
		options_.width, options_.height, options_.padding};
}

void renderer::MapRenderer::RenderRoute(svg::Document& doc, const domain::Route* route, size_t count)
{
	svg::Polyline polyline;
	for (const domain::Stop* currentStop: route->stops) {
		polyline.AddPoint(projector_(currentStop->coordinates));
	}
	size_t colorIndex = count % options_.color_palette.size();

	polyline.SetFillColor(svg::NoneColor)
		.SetStrokeColor(options_.color_palette[colorIndex])
		.SetStrokeWidth(options_.line_width)
		.SetStrokeLineCap( svg::StrokeLineCap::ROUND)
		.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
	doc.Add(polyline);
}

void renderer::MapRenderer::RenderRouteLables(svg::Document& doc, const domain::Route* route, size_t count)
{
	using namespace std::string_literals;
	const std::string FONT_FAMILY = "Verdana"s;
	const std::string FONT_WEIGHT = "bold"s;
	
	svg::Text text;
	svg::Text background;
	size_t colorIndex = count % options_.color_palette.size();

	auto basePoint = projector_(route->firstFinalStop.value()->coordinates);
	auto offset = options_.bus_label_offset;
	auto name = static_cast<std::string>(route->name);
	background.SetPosition(basePoint)
		.SetOffset(offset)
		.SetFontSize(options_.bus_label_font_size)
		.SetFontFamily(FONT_FAMILY)
		.SetFontWeight(FONT_WEIGHT)
		.SetData(name)
		.SetFillColor(options_.underlayer_color)
		.SetStrokeColor(options_.underlayer_color)
		.SetStrokeWidth(options_.underlayer_width)
		.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
		.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
	text.SetPosition(basePoint)
		.SetOffset(offset)
		.SetFontSize(options_.bus_label_font_size)
		.SetFontFamily(FONT_FAMILY)
		.SetFontWeight(FONT_WEIGHT)
		.SetData(name)
		.SetFillColor(options_.color_palette[colorIndex]);
	doc.Add(background);
	doc.Add(text);
	if (route->secondFinalStop.has_value()) {
		basePoint = projector_(route->secondFinalStop.value()->coordinates);
		background.SetPosition(basePoint)
			.SetOffset(offset)
			.SetFontSize(options_.bus_label_font_size)
			.SetFontFamily(FONT_FAMILY)
			.SetFontWeight(FONT_WEIGHT)
			.SetData(name)
			.SetFillColor(options_.underlayer_color)
			.SetStrokeColor(options_.underlayer_color)
			.SetStrokeWidth(options_.underlayer_width)
			.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
			.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		text.SetPosition(basePoint)
			.SetOffset(offset)
			.SetFontSize(options_.bus_label_font_size)
			.SetFontFamily(FONT_FAMILY)
			.SetFontWeight(FONT_WEIGHT)
			.SetData(name)
			.SetFillColor(options_.color_palette[colorIndex]);
		doc.Add(background);
		doc.Add(text);
	}
}

void renderer::MapRenderer::RenderStopPoint(svg::Document& doc, const domain::Stop* stop)
{
	using namespace std::string_literals;
	svg::Circle circle;

	circle.SetCenter(projector_(stop->coordinates))
		.SetFillColor("white"s)
		.SetRadius(options_.stop_radius);
	doc.Add(circle);
}

void renderer::MapRenderer::RenderStopLable(svg::Document& doc, const domain::Stop* stop)
{
	using namespace std::string_literals;
	const std::string FONT_FAMILY = "Verdana"s;
	const svg::Color TEXT_FILL_COLOR{ "black"s };
	svg::Text text;
	svg::Text background;
	auto basePoint = projector_(stop->coordinates);
	auto offset = options_.stop_label_offset;
	auto name = static_cast<std::string>(stop->name);
	background.SetPosition(basePoint)
		.SetOffset(offset)
		.SetFontSize(options_.stop_label_font_size)
		.SetFontFamily(FONT_FAMILY)
		.SetData(name)
		.SetFillColor(options_.underlayer_color)
		.SetStrokeColor(options_.underlayer_color)
		.SetStrokeWidth(options_.underlayer_width)
		.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
		.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
	text.SetPosition(basePoint)
		.SetOffset(offset)
		.SetFontSize(options_.stop_label_font_size)
		.SetFontFamily(FONT_FAMILY)
		.SetData(name)
		.SetFillColor(TEXT_FILL_COLOR);
	doc.Add(background);
	doc.Add(text);
}
