// Copyright 2016-2016 the openage authors. See copying.md for legal info.

#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <tuple>

#include <QQuickItem>

#include "../coord/chunk.h"
#include "../engine.h"

#include "minimap_unit_marker_points.h"

class QSGGeometryNode;

namespace openage {
namespace gui {

class GameMainLink;
class GuiTexture;

struct background_update {
	bool is_fresh() const {
		return std::get<0>(this->background_tex).operator bool();
	}

	std::tuple<std::unique_ptr<uint32_t[]>, int, int> background_tex;
	std::tuple<coord::chunk, coord::chunk_t> bounding_square;
};

/**
 * QML item to render a minimap.
 *
 * Accepts another QML item and uses it as atlas: source of icons for each marker type.
 */
class Minimap : public QQuickItem {
	Q_OBJECT

	Q_PROPERTY(openage::gui::GameMainLink* game READ get_game WRITE set_game)
	Q_PROPERTY(int markerSize MEMBER marker_size)
	Q_PROPERTY(QRect viewFrameRect MEMBER view_frame_rect NOTIFY viewFrameRectChanged)
	Q_PROPERTY(QQuickItem* markerAtlas READ get_marker_atlas WRITE set_marker_atlas)
	Q_PROPERTY(QVariantList markersTypes READ get_markers_types WRITE set_markers_types)
	Q_PROPERTY(float markerAtlasSpacing READ get_marker_atlas_spacing WRITE set_marker_atlas_spacing)

public:
	explicit Minimap(QQuickItem *parent=nullptr);
	virtual ~Minimap();

	/**
	 * Update textures and geometry buffers.
	 * Running in the render thread while the GUI thread is blocked.
	 */
	virtual QSGNode* updatePaintNode(QSGNode *node, UpdatePaintNodeData*) override;

	/**
	 * Extract necessary data from the game logic.
	 * Running in the GUI thread.
	 */
	virtual void updatePolish() override;

	GameMainLink* get_game() const;
	void set_game(GameMainLink *game);

	/**
	 * A texture provider item with a square grid of icons for the markers.
	 * Define it in QML.
	 * Must be a texture provider (set 'layer.enabled: true' in its QML definition).
	 *
	 * For 5 marker types:
	 *  _______________________
	 * |  _____|  _____|  _____|
	 * | |     | |     | |     |
	 * | |  0  | |  1  | |  2  |
	 * |_|_____|_|_____|_|_____|
	 * |  _____|  _____|  _____|
	 * | |     | |     | |     |
	 * | |  3  | |  4  | |     |
	 * |_|_____|_|_____|_|_____|
	 * |  _____|  _____|  _____|
	 * | |     | |     | |     |
	 * | |     | |     | |     |
	 * |_|_____|_|_____|_|_____|
	 */
	QQuickItem* get_marker_atlas() const;
	void set_marker_atlas(QQuickItem *marker_atlas);

	/**
	 * Width and height of the empty space to the left and top of each cell (in tex coords).
	 */
	float get_marker_atlas_spacing() const;
	void set_marker_atlas_spacing(float marker_atlas_spacing);

	/**
	 * The gamedata::minimap_mode enum value for each icon in the marker atlas.
	 * The atlas is a square grid, indexed from left-top to bottom-right.
	 */
	QVariantList get_markers_types() const;
	void set_markers_types(const QVariantList &markers_types);

signals:
	void viewFrameRectChanged();

private:
	std::unique_ptr<QSGNode> create_tree();

	GameMainLink *game;
	bool game_dirty;

	std::vector<gamedata::palette_color> player_palette;

	/**
	 * Palette used in the minimap background generation.
	 */
	std::vector<gamedata::palette_color> palette;

	std::function<void()> update_marker_atlas;
	std::function<void(background_update&&, const coord_data&)> update_backround;
	std::function<void(const coord_data&)> update_view_frame_rect;
	std::function<void(const std::vector<ColoredTexturedPoint2D>&)> update_marker_points;

	std::unique_ptr<QSGNode> hidden_root;
	std::unique_ptr<QSGGeometryNode> hidden_foreground;

	QRectF last_bounding_rect;

	size_t terrain_usage_hash;
	background_update bg_update;
	bool need_freash_bg;

	/**
	 * Field of view of the camera in the minimap coordinates.
	 * Use it to position the QML item that represents the camera's field of view.
	 */
	QRect view_frame_rect;

	/**
	 * Size of the markers in pixels.
	 */
	int marker_size;

	QQuickItem *marker_atlas;
	std::vector<gamedata::minimap_modes> markers_types;
	bool marker_atlas_dirty;

	float marker_atlas_spacing;
	bool marker_atlas_spacing_dirty;

	std::vector<ColoredTexturedPoint2D> unit_marker_points;

	// TODO: maybe replace with something like std::optional
	std::tuple<coord_data, bool> engine_coord_data;
};

}} // namespace openage::gui
