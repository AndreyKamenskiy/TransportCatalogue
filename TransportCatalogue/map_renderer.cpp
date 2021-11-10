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

svg::Polyline renderer::MapRenderer::RenderRoute(const domain::Route* route, size_t count)
{
	svg::Polyline polyline;
	for (const domain::Stop* currentStop: route->stops) {
		polyline.AddPoint(projector_(currentStop->coordinates));
	}
	size_t colorIndex = count % options_.color_palette.size();

	polyline.SetFillColor(svg::NoneColor)
		.SetStrokeColor(options_.color_palette[colorIndex])
		.SetStrokeWidth(options_.line_width)
		.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
		.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
	return polyline;
}
