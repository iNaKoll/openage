// Copyright 2016-2016 the openage authors. See copying.md for legal info.

#pragma once

#include <QSGSimpleMaterialShader>
#include <QSGTextureProvider>

namespace openage {
namespace gui {

struct Icon
{
	QQuickItem *marker_atlas;
	float columns;
	float spacing;
};

/**
 * Draws ColoredTexturedPoint2D arrays.
 *
 * Uses icons found in the square-grid atlas by the ColoredTexturedPoint2D::cell_id.
 * Applies color from ColoredTexturedPoint2D regardless of the color in the atlas.
 */
class TexturedPointsShader : public QSGSimpleMaterialShader<Icon>
{
	QSG_DECLARE_SIMPLE_SHADER(TexturedPointsShader, Icon)

public:
	TexturedPointsShader()
		:
		QSGSimpleMaterialShader<Icon>{},
		id_tex{},
		id_columns{},
		id_spacing{} {
	}

	virtual const char* vertexShader() const override {
		return "#version 120\n"
			"attribute highp vec4 vertex;\n"
			"attribute highp float cell_id;\n"
			"attribute lowp vec4 color;\n"
			"uniform highp mat4 qt_Matrix;\n"
			"varying vec4 v_color;\n"
			"varying float v_cell_id;\n"
			"void main() {\n"
			"    gl_Position = qt_Matrix * vertex;\n"
			"    v_color = color;\n"
			"    v_cell_id = cell_id;\n"
			"}";
	}

	virtual const char* fragmentShader() const override {
		return "#version 120\n"
			"uniform lowp float qt_Opacity;\n"
			"uniform lowp sampler2D tex;\n"
			"uniform highp float columns;\n"
			"uniform highp float spacing;\n"
			"varying vec4 v_color;\n"
			"varying float v_cell_id;\n"
			"void main() {\n"
			"    vec2 border = vec2(spacing);\n"
			"    vec2 in_grid_coord = vec2(mod(v_cell_id, columns), floor(v_cell_id / columns)) + border;\n"
			"    vec2 subtex_coord = (in_grid_coord + gl_PointCoord * (vec2(1.0) - border)) / columns;\n"
			"    if (texture2D(tex, subtex_coord).a < 0.1)\n"
			"        discard;\n"
			"    else;\n"
			"        gl_FragColor = vec4(v_color.rgb, v_color.a * texture2D(tex, subtex_coord).a) * qt_Opacity;\n"
			"}";
	}

	virtual QList<QByteArray> attributes() const override {
		return QList<QByteArray>{} << "vertex" << "cell_id" << "color";
	}

	virtual void updateState(const Icon *icon, const Icon*) override {
		assert(icon->marker_atlas);
		assert(icon->marker_atlas->textureProvider());
		assert(icon->marker_atlas->textureProvider()->texture());
		icon->marker_atlas->textureProvider()->texture()->bind();
		this->program()->setUniformValue(this->id_columns, icon->columns);
		this->program()->setUniformValue(this->id_spacing, icon->spacing);
	}

	virtual void resolveUniforms() override {
		this->id_tex = this->program()->uniformLocation("tex");
		this->program()->setUniformValue(id_tex, 0);
		this->id_columns = this->program()->uniformLocation("columns");
		this->id_spacing = this->program()->uniformLocation("spacing");
	}

private:
	int id_tex;
	int id_columns;
	int id_spacing;
};

}} // namespace openage::gui
