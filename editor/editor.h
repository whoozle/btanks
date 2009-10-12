#ifndef BTANKS_EDITOR_H__
#define BTANKS_EDITOR_H__

/* Battle Tanks Game
 * Copyright (C) 2006-2009 Battle Tanks team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
*/

#include "sl08/sl08.h"
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

class Editor : public Container {
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

	void render(sdlx::Surface &surface, const float dt);

	//slots:	
	sl08::slot1<bool, float, Editor> on_tick_slot;
	bool onTick(float dt);

	sl08::slot2<bool, const SDL_keysym, const bool, Editor> on_key_slot;
	bool onKeySignal(const SDL_keysym sym, const bool pressed);
	
	sl08::slot4<bool, const int, const bool, const int, const int, Editor> on_mouse_slot;
	bool onMouseSignal(const int button, const bool pressed, const int x, const int y);
	
	sl08::slot5<bool, const int, const int, const int, const int, const int, Editor> on_mouse_motion_slot;
	bool onMouseMotionSignal(const int state, const int x, const int y, const int xrel, const int yrel);
	
	sl08::slot1<void, const SDL_Event &, Editor> on_event_slot;
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

	//slots: 
	sl08::slot1<void, const int, Editor> reset_slot;
	void resetLoadingBar(const int total);
	sl08::slot2<void, const int, const char *, Editor> notify_slot;
	void notifyLoadingBar(const int progress, const char *what);
	
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

