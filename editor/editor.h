#ifndef BTANKS_EDITOR_H__
#define BTANKS_EDITOR_H__

#include <sigc++/sigc++.h>
#include "sdlx/sdlx.h"
#include "sdlx/rect.h"
#include "menu/container.h"
#include "alarm.h"
#include <string>
#include <deque>
#include "command.h"

class OpenMapDialog;
class TilesetDialog;
class Hud;
class LayerListDialog;
class BaseBrush; 
class AddObjectDialog;
class ObjectPropertiesDialog;
class ScrollList;
class MorphDialog;
class ResizeDialog;

namespace sdlx {
	class Font;
	class Surface;
}
class Object;

class Editor : public sigc::trackable, public Container {
public: 
	Editor();
	void init(int argc, char **argv);
	void run();
	void deinit();	
	
	void loadMap(const std::string &map);

	void addCommand(Command &cmd);
	Command &currentCommand();
	void undo();
	void redo();
	
	void moveObjectHack(Object *object, const v2<int>& screen_pos);
	
private: 
	virtual bool onKey(const SDL_keysym sym); //from ::Control
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);
	virtual bool onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel);

	void onTick(const float dt);
	void render(sdlx::Surface &surface, const float dt);
	
	bool onKeySignal(const SDL_keysym sym, const bool pressed);
	bool onMouseSignal(const int button, const bool pressed, const int x, const int y);
	bool onMouseMotionSignal(const int state, const int x, const int y, const int xrel, const int yrel);

	void onEvent(const SDL_Event &event);
	
	void displayStatusMessage(const Object *o);
	void updateLayers();

	OpenMapDialog *_map_picker;
	Hud *_hud;
	TilesetDialog *_tileset_dialog;
	LayerListDialog * _layers_dialog;
	AddObjectDialog * _add_object;
	ResizeDialog *_resize_map;

	std::deque<std::pair<std::string, std::string> > _morph_boxes;
	MorphDialog *_morph_dialog;
	
	sdlx::Rect map_pos;
	v2<float> map_dir;
	v2<int> _tile_size;
	std::string _map_base, _map_file;

	int _loading_bar_total, _loading_bar_now;
	void resetLoadingBar(const int total);
	void notifyLoadingBar(const int progress = 1);
	
	int _current_layer_z;
	BaseBrush *_brush;
	v2<int> _last_tile_painted;
	
	std::string _layer_name;
	Alarm _layer_name_invisible, _layer_invisible;
	const sdlx::Font *_small_font;

	std::deque<Command> undo_queue, redo_queue;

	bool _render_objects;
	const Object *_highlight_object;
	bool _dragging, _selecting;
	v2<int> _selection1, _selection2;

	v2<int> mouse_pos; 
	
	ObjectPropertiesDialog *_object_properties;
};

#endif

