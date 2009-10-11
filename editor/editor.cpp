/* Battle Tanks Game
 * Copyright (C) 2006-2008 Battle Tanks team
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

#include "editor.h"

#include "mrt/logger.h"
#include "mrt/directory.h"
#include "mrt/file.h"
#include "sdlx/sdlx.h"

#include "window.h"
#include "resource_manager.h"
#include "animation_model.h"
#include "game_monitor.h"

#include "object.h"
#include "world.h"
#include "finder.h"

#include "i18n.h"
#include "var.h"
#include "config.h"
#include "hud.h"
#include "rt_config.h"

#include "tmx/layer.h"
#include "tmx/map.h"
#include "tmx/generator.h"

#include "math/unary.h"

#include "open_map_dialog.h"
#include "tileset_dialog.h"
#include "layer_list_dialog.h"
#include "add_object_dialog.h"
#include "base_brush.h"
#include "layer_item.h"
#include "command.h"
#include "object_properties.h"
#include "tilebox_brush.h"
#include "morph_dialog.h"
#include "resize_dialog.h"

#include "game.h"
#include "math/binary.h"

void Editor::moveObjectHack(Object *object, const v2<int>& screen_pos) {
	World->move(object, screen_pos.x + map_pos.x, screen_pos.y + map_pos.y);
	World->purge(0);
}

void Editor::addCommand(Command &cmd) {
	undo_queue.push_back(cmd);
	redo_queue.clear();
}

Command &Editor::currentCommand() {
	if (undo_queue.empty())
		throw_ex(("undo queue is empty"));
	return undo_queue.back();
}

void Editor::undo() {
	if (undo_queue.empty())
		return;

	LOG_DEBUG(("undo"));
	undo_queue.back().undo();
	redo_queue.push_front(undo_queue.back());
	undo_queue.pop_back();
	//LOG_DEBUG(("undo. undo queue size = %u", (unsigned)undo_queue.size()));
}

void Editor::redo() {
	if (redo_queue.empty())
		return;

	LOG_DEBUG(("redo"));
	redo_queue.front().exec();
	undo_queue.push_back(redo_queue.front());
	redo_queue.pop_front();
	//LOG_DEBUG(("redo. undo queue size = %u", (unsigned)redo_queue.size()));
}


Editor::Editor() : 
	_map_picker(NULL), _hud(NULL), _brush(NULL), _layer_name_invisible(1.0f, false), _layer_invisible(1.0f, false), 
	_render_objects(true), _highlight_object(NULL), _dragging(false), _selecting(false) {
}

void Editor::loadMap(const std::string &map) {
TRY {
	_render_objects = true;
	_dragging = false;
	_selecting = false;
	
	LOG_DEBUG(("loading map %s", map.c_str()));
	GameMonitor->clear();
	World->clear();
	
	GameMonitor->loadMap(NULL, map);

	v2<int> map_size = Map->get_size();
	_tile_size = Map->getTileSize();
	map_pos.w = map_size.x;
	map_pos.h = map_size.y;
	map_pos.x = map_pos.y = 0;

	delete _brush;
	_brush = NULL;
	undo_queue.clear();
	redo_queue.clear();

	{
		Var bt("bool");
		bt.b = true;
		Config->setOverride("engine.mark-map-tiles", bt);
		Config->setOverride("engine.show-waypoints", bt);
	}

	_current_layer_z = 0;
	updateLayers();
	Map->getGenerator()->getPrimaryBoxes(_morph_boxes);
} CATCH("loadMap", throw;)
}

void Editor::onTick(const float dt) {
	static const Uint8 *keys = SDL_GetKeyState(0);
	bool active = (keys[SDLK_LSHIFT] != 0 || keys[SDLK_RSHIFT] != 0 || _layers_dialog->active()) && Map->loaded() && _tileset_dialog->hidden();
	_layers_dialog->hide(!active);

	tick(dt);
	Map->tick(dt);	

	if (_map_picker->changed()) {
		_map_picker->reset();
		_map_picker->getMap(_map_base, _map_file);
		loadMap(_map_file);
	}

	if (_tileset_dialog->changed()) {
		_morph_dialog->initMap();
		_tileset_dialog->reset();
		BaseBrush *b = _tileset_dialog->getBrush();
		delete _brush;
		_brush = b;
	}
	if (_tileset_dialog->tileset_added()) {
		_morph_dialog->initMap();
	}
	
	if (_layers_dialog->changed()) {
		_layers_dialog->reset();
		updateLayers();
	}
	
	if (_add_object->changed()) {
		_add_object->reset();
		std::string classname, animation;
		int z;
		if (_add_object->get(classname, animation, z)) {
			delete _brush; 
			_brush = NULL;
			TRY {
				_brush = new ObjectBrush(this, classname, animation, z);
			} CATCH("adding object brush", {});
		}
		/*
		if (_add_object->get(classname, animation, z)) {
			SDL_WarpMouse(mouse_pos.x, mouse_pos.y);
			LOG_DEBUG(("creation of %s:%s requested (z: %d)", classname.c_str(), animation.c_str(), z));

			const Animation *a = ResourceManager.get_const()->getAnimation(animation);
			v2<int> pos = (mouse_pos.convert<float>() + v2<float>(map_pos.x, map_pos.y) - v2<float>(a->tw, a->th) / 2).convert<int>();

			Command cmd(classname, animation, pos, z);
			addCommand(cmd);
			currentCommand().exec();
		}
		*/
	}
	
	if (_object_properties->changed()) {
		std::set<std::string> vars;
		_object_properties->get(vars);
		Command cmd(Command::ChangeObjectProperties, _object_properties->object, _object_properties->get_z(), Variants(vars));
		addCommand(cmd);
		currentCommand().exec();
		displayStatusMessage(_object_properties->object);

		_object_properties->reset();
		_object_properties->hide();
	}
	
	if (_morph_dialog->changed()) {
		_morph_dialog->reset();
		int idx = _morph_dialog->get();
		if (Map->loaded() && idx < (int)_morph_boxes.size()) {
			LOG_DEBUG(("morph brush"));
			delete _brush;
			_brush = new TileBoxBrush(_morph_boxes[idx].first, _morph_boxes[idx].second);
		}
	}

	{
		v2<float> dpos = map_dir;
		GET_CONFIG_VALUE("editor.scrolling-speed", int, speed, 500);
		dpos *= speed * dt;
		map_pos.x += (int)dpos.x;
		map_pos.y += (int)dpos.y;
	}
	
	World->purge(0);
	
	render(Window->get_surface(), dt);	
}


void Editor::render(sdlx::Surface &surface, const float dt) {
	sdlx::Rect window_size = surface.get_size();
	surface.fill(surface.map_rgb(255, 255, 255));

	const bool highlight_layer = !_layer_invisible.tick(dt);
	const bool show_layer_name = !_layer_name_invisible.tick(dt);
	if (Map->loaded()) {
		const sdlx::Rect window_size = Window->get_size();
		const v2<int> map_size = Map->get_size();
		
		if (map_pos.x < 0) 
			map_pos.x = 0;

		if (map_pos.y < 0)
			map_pos.y = 0;

		if (map_pos.x + window_size.w > map_size.x)
			map_pos.x = map_size.x - window_size.w;

		if (map_pos.y + window_size.h > map_size.y)
			map_pos.y = map_size.y - window_size.h;
	
		const bool render_objects = _render_objects && !Map->hasSoloLayers();
	
		if (render_objects)
			World->render(surface, map_pos, window_size, -10000, _current_layer_z);
		else 
			Map->render(surface, map_pos, window_size, -10000, _current_layer_z);
		
		if (highlight_layer) {
			int p = (int)(_layer_invisible.get() * 6);
			if (p & 1) {
				if (render_objects)
					World->render(surface, map_pos, window_size, _current_layer_z, _current_layer_z + 1);
				else
					Map->render(surface, map_pos, window_size, _current_layer_z, _current_layer_z + 1);
			}
		} else {
			if (render_objects)
				World->render(surface, map_pos, window_size, _current_layer_z, _current_layer_z + 1);
			else
				Map->render(surface, map_pos, window_size, _current_layer_z, _current_layer_z + 1);
		}
		
		if (_brush && _tileset_dialog->hidden()) {
			//LOG_DEBUG(("picked up brush %p", _brush));
			int mx, my;
			SDL_GetMouseState(&mx, &my);
			v2<int> window_pos(mx, my);
			v2<int> base(map_pos.x, map_pos.y);
			v2<int> tile_pos = (base + window_pos) / _tile_size * _tile_size - base;
			_brush->render(surface, window_pos, tile_pos);
		}
		if (render_objects)
			World->render(surface, map_pos, window_size, _current_layer_z + 1, 10000);
		else
			Map->render(surface, map_pos, window_size, _current_layer_z + 1, 10000);

		if (_selecting || _selection1.quick_distance(_selection2) >= 1024) {
			static sdlx::Surface selection;
			if (selection.isNull()) {
				selection.create_rgb(_tile_size.x, _tile_size.y, 32);
				selection.display_format_alpha();
				selection.fill(selection.map_rgba(0x11, 0xdd, 0x11, 0x80));
			}
			v2<int> base(map_pos.x, map_pos.y);
			v2<int> sel1 = _selection1, sel2 = _selection2;
			if (sel1.x > sel2.x)
				math::exchange(sel1.x, sel2.x);
			if (sel1.y > sel2.y)
				math::exchange(sel1.y, sel2.y);
			
			v2<int> t1 = v2<int>(sel1.x, sel1.y) / _tile_size * _tile_size - base;
			v2<int> t2 = v2<int>(sel2.x, sel2.y) / _tile_size * _tile_size - base;
			//LOG_DEBUG(("%d,%d -> %d,%d", t1.x, t1.y, t2.x, t2.y));
			for(int y = t1.y; y <= t2.y; y += _tile_size.y) 
				for(int x = t1.x; x <= t2.x; x += _tile_size.x) {
					surface.blit(selection, x, y);
				}
		}
	}
	
	if ((show_layer_name || highlight_layer) && !_layer_name.empty()) {
		//int w = _small_font->render(NULL, 0, 0, _layer_name);
		_small_font->render(surface, 8, 8, _layer_name);
	}
	
	if (_brush == NULL && _highlight_object) {
		static const sdlx::Surface *hl;
		if (hl == NULL) 
			hl = ResourceManager->load_surface("object.png");
		
		v2<float> pos;
		_highlight_object->get_center_position(pos);
		surface.blit(*hl, 
			(int)(pos.x - map_pos.x - hl->get_width() / 2), 
			(int)(pos.y - map_pos.y - hl->get_height() / 2) 
		);
	}
	
	Container::render(surface, 0, 0);
}

#include "mrt/lang.h"

void Editor::init(int argc, char *argv[]) {
	std::string config_path = mrt::Directory::get_app_dir("Battle Tanks", "btanks") + "/";
	Config->load(config_path + "bt.xml");
	
	std::string lang; //default
	
	for(int i = 1; i < argc; ++i) {
		if (strncmp(argv[i], "--lang=", 7) == 0) { lang = argv[i] + 7; }
	}
	
	if (lang.empty()) {
		if (Config->has("engine.language")) {
			Config->get("engine.language", lang, std::string());
		}

		if (lang.empty())
			lang = mrt::get_lang_code();
	}
	
	I18n->load(lang);
	Window->init(argc, argv);
	sdlx::Rect window_size = Window->get_size();

	LOG_DEBUG(("initializing hud..."));
	_hud = new Hud(window_size.w, window_size.h);

	LOG_DEBUG(("installing callbacks..."));

	on_key_slot.assign(this, &Editor::onKeySignal, Window->key_signal);
	on_mouse_slot.assign(this, &Editor::onMouseSignal, Window->mouse_signal);
	on_mouse_motion_slot.assign(this, &Editor::onMouseMotionSignal, Window->mouse_motion_signal);
	on_event_slot.assign(this, &Editor::onEvent, Window->event_signal);

	reset_slot.assign(this, &Editor::resetLoadingBar, Map->reset_progress);
	notify_slot.assign(this, &Editor::notifyLoadingBar, Map->notify_progress);

	reset_slot.assign(this, &Editor::resetLoadingBar, ResourceManager->reset_progress);
	notify_slot.assign(this, &Editor::notifyLoadingBar, ResourceManager->notify_progress);

	on_tick_slot.assign(this, &Editor::onTick, Window->tick_signal);

	std::vector<std::pair<std::string, std::string> > files;
	Finder->findAll(files, "resources.xml");
	
	ResourceManager->init(files);
	
	Control * c = _map_picker = new OpenMapDialog;
	int cw, ch;
	c->get_size(cw, ch);
	add((window_size.w - cw) / 2, (window_size.h - ch) / 2, c);

	c = _tileset_dialog = new TilesetDialog(window_size.w, window_size.h);
	add(0, 0, c);
	c->hide();
	
	c = _layers_dialog = new LayerListDialog(window_size.w, window_size.h);
	c->get_size(cw, ch);
	add((window_size.w - cw), 0, c);
	c->hide();
	
	c = _add_object = new AddObjectDialog(512, 384);
	c->get_size(cw, ch);
	add((window_size.w - cw) / 2, (window_size.h - ch) / 2, c);
	c->hide();

	c = _morph_dialog = new MorphDialog(512, 384);
	c->get_size(cw, ch);
	add((window_size.w - cw) / 2, (window_size.h - ch) / 2, c);
	c->hide();
	
	RTConfig->editor_mode = true;
	
	_small_font = ResourceManager->loadFont("small", true);
	
	_object_properties = new ObjectPropertiesDialog(240);
	add(0, 0, _object_properties);
	_object_properties->hide();
	
	_resize_map = new ResizeDialog();
	_resize_map->get_size(cw, ch);
	add((window_size.w - cw) / 2, (window_size.h - ch) / 2, _resize_map);
	_resize_map->hide();
}

void Editor::run() {
	LOG_DEBUG(("entering main loop"));
	Window->run();
	LOG_DEBUG(("exiting main loop"));
}

void Editor::deinit() {

	delete _hud;
	_hud = NULL;

	ResourceManager->clear();
	Window->deinit();

	TRY {
		Config->save();
	} CATCH("saving config", {});
}

bool Editor::onKey(const SDL_keysym sym) {
	if (Container::onKey(sym))
		return true;
	
	int dir = 0;
	switch(sym.sym) {
	case SDLK_x: 
	case SDLK_c: 
		if ((sym.mod & KMOD_CTRL) == 0)
			break;
		{
			if (_brush != NULL || _selecting || _selection1.quick_distance(_selection2) < 1024)
				return true;
			
			Layer * l = Map->getLayer(_current_layer_z);
			if (!l->isVisible())
				return true;
			
			v2<int> t1 = v2<int>(_selection1.x, _selection1.y) / _tile_size;
			v2<int> t2 = v2<int>(_selection2.x, _selection2.y) / _tile_size;
			//LOG_DEBUG(("%d,%d -> %d,%d", t1.x, t1.y, t2.x, t2.y));
			
			std::vector<int> tiles;
			bool empty = true;
			for(int y = t1.y; y <= t2.y; ++y) 
				for(int x = t1.x; x <= t2.x; ++x) {
					int tid = l->get(x, y);
					//LOG_DEBUG(("tid: %d", tid));
					tiles.push_back(tid);
					if (tid)
						empty = false;
				}
			if (empty)
				return true;

			_brush = new Brush(_tile_size, tiles);
			_brush->size = t2 - t1 + 1;
						
			bool cut = sym.sym == SDLK_x;
			_selection1.clear();
			_selection2.clear();
			LOG_DEBUG(("copypastecut: %s", cut?"cut":"copy"));
			if (cut) {
				{
					Command cmd(l);
					addCommand(cmd);				
				}
				for(int y = t1.y; y <= t2.y; ++y) 
					for(int x = t1.x; x <= t2.x; ++x) {
						currentCommand().setTile(x, y, 0);
				}
				currentCommand().exec();
			}
		}
		return true;
	
	case SDLK_q: 
		if (sym.mod & KMOD_ALT) {
			Window->stop();
			return true;
		} 
		break;
	case SDLK_ESCAPE: 
		if (_brush) {
			delete _brush;
			_brush = NULL;
		}
		return true;

	case SDLK_INSERT:
	case SDLK_i: 
		if (Map->loaded()) {
			SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
			_add_object->hide(false);
		}
		return true;
	
	case SDLK_z: 
		if (!Map->loaded())
			return true;

		if ((sym.mod & KMOD_CTRL) != 0) {
			if (sym.mod & KMOD_SHIFT) 
				redo();
			else 
				undo();
			return true;
		} 
		break;
	case SDLK_r: 
		if (!Map->loaded())
			return true;

		if ((sym.mod & KMOD_ALT) != 0) {
			_resize_map->show();
			return true;
		} 

		if ((sym.mod & KMOD_CTRL) != 0) {
			redo();
			return true;
		} 
		break;
	case SDLK_b: {
		if (!Map->loaded())
			return true;

		BaseBrush *b = _tileset_dialog->getBrush();
		if (b->size.is0()) {
			delete b;
			return true;
		}
		delete _brush;
		_brush = b;
		return true;
		}
	case SDLK_g: {
		int mx, my;
		SDL_GetMouseState(&mx, &my);
		v2<int> window_pos(mx, my);
		v2<int> base(map_pos.x, map_pos.y);
		v2<int> pixel_pos = base + window_pos;
		v2<int> tile_pos = pixel_pos / _tile_size;
		//mrt::format_string("%d,%d", tile_pos.x, tile_pos.y).c_str());
		_layer_name = mrt::format_string("grid_position: %d,%d, pixel position: %d,%d", tile_pos.x, tile_pos.y, pixel_pos.x, pixel_pos.y);
		_layer_name_invisible.reset();

		return true;
	}
	case SDLK_f: {
		//LOG_DEBUG(("f"));
		if (!Map->loaded())
			return true;
		
		Layer * l = Map->getLayer(_current_layer_z);
		if (!l->isVisible())
			return true;
		
		Brush *p_brush = dynamic_cast<Brush *>(_brush);
		if (p_brush) {
			//LOG_DEBUG(("tile pos: %d %d", x / _tile_size.x, y / _tile_size.y));
			//_last_tile_painted = v2<int>((map_pos.x + x)/ _tile_size.x, (map_pos.y + y) / _tile_size.y);
			int x, y;
			SDL_GetMouseState(&x, &y);

			v2<int> pos((map_pos.x + x)/ _tile_size.x, (map_pos.y + y) / _tile_size.y);
			{
				Command cmd(l);
				addCommand(cmd);
			}
			FillerBrush brush(*p_brush, Map->get_size() / Map->getTileSize());
			brush.exec(currentCommand(), pos.x, pos.y );
			currentCommand().exec();
		} else 
			LOG_DEBUG(("_brush is not Brush. skip fill"));
		return true;
	}
	case SDLK_e: 
		if (!Map->loaded())
			return true;

		LOG_DEBUG(("eraser"));
		delete _brush;
		_brush = new Eraser(_tile_size);
		return true;
	case SDLK_m: 
		if (!Map->loaded())
			return true;
		
		_morph_dialog->hide(!_morph_dialog->hidden());
		return true;
	case SDLK_o: 
		if (sym.mod & KMOD_ALT) {
			_map_picker->hide(false);
		} else {
			_render_objects = !_render_objects;
		}
		return true;
	case SDLK_s:
		if (sym.mod & KMOD_ALT && Map->loaded()) {
			std::string result;
			Map->generateXML(result);
			//LOG_DEBUG(("map returned:\n%s", result.c_str()));
			try {
				mrt::Directory dir;
				dir.create(_map_base + "/maps", true);
			} catch(...) {}
			mrt::File f;
			std::string fname = _map_base + "/maps/" + _map_file + ".tmx";
			f.open(fname, "wb");
			LOG_DEBUG(("saving map to %s...", fname.c_str()));
			f.write_all(result);
			f.close();
			return true;
		} break;
	case SDLK_TAB: 
		if (!Map->loaded())
			return true;

		_tileset_dialog->hide(false);
		return true;
		//if (!_tileset_dialog->hidden() || !_map_picker->hidden() || !Map->loaded())
		//	return false;
		//_layers_dialog->hide(false);
	case SDLK_LSHIFT: 
	case SDLK_RSHIFT: 
		_layer_invisible.reset();
		return true;
	case SDLK_DELETE: 
		if (_highlight_object) {
			Command cmd(Command::DeleteObject, _highlight_object);
			addCommand(cmd);
			currentCommand().exec();
			
			_highlight_object = NULL;
		}
		return true;
		
	case SDLK_RIGHTBRACKET: 
		dir = -2;
	case SDLK_LEFTBRACKET: 
		++dir;
		if (_highlight_object == NULL)
			return false;
		{
			int dirs = _highlight_object->get_directions_number();
			if (dirs != 8 && dirs != 16)
				return false;
			
			Command cmd(Command::RotateObject, _highlight_object, dir);
			addCommand(cmd);
			currentCommand().exec();
			displayStatusMessage(_highlight_object);			
		}
		return true;
	case SDLK_KP_MINUS:
	case SDLK_MINUS: 
		dir = -20;
	case SDLK_PLUS:
	case SDLK_KP_PLUS:
		dir += 10;
		if (_highlight_object == NULL)
			return false;
		{
			Command cmd(Command::ChangeObjectProperties, _highlight_object, _highlight_object->get_z() + dir, Variants(_highlight_object->get_variants()));
			addCommand(cmd);
			currentCommand().exec();
			displayStatusMessage(_highlight_object);
		}
		return true;
		
	default: 
		return false;
	}
	return false;	
}

void Editor::displayStatusMessage(const Object *object) {
	std::string prop;
	try {
		GameItem &item = GameMonitor->find(_highlight_object);
		prop = " (" + item.property + ")";
	} catch(...) {}
	_layer_name = mrt::format_string("%s:%s%s, z: %d, dir: %d%s", 
		object->registered_name.c_str(), object->animation.c_str(), object->get_variants().dump().c_str(), object->get_z(), object->get_direction(), prop.c_str());
	_layer_name_invisible.reset();
}

bool Editor::onMouse(const int button, const bool pressed, const int x, const int y) {
	if (Container::onMouse(button, pressed, x, y))
		return true;

	if (!Map->loaded()) {
		return false;
	}
	
	static const Uint8 *keys = SDL_GetKeyState(0);
	if (keys[SDLK_LSHIFT] != 0) {
		
		if (button == SDL_BUTTON_WHEELUP) {
			if (pressed)
				_layers_dialog->up();
		} else if (button == SDL_BUTTON_WHEELDOWN) {
			if (pressed)
				_layers_dialog->down();
		}
		
		return true;
	} else if (keys[SDLK_SPACE] != 0) {
		if (button == SDL_BUTTON_RIGHT) {
			if (!pressed)
				return true;
		
			const sdlx::Rect window_size = Window->get_size();
	
			map_pos.x += x - window_size.w / 2;
			map_pos.y += y - window_size.h / 2;
		
		}
		return true;
	} else {
		if (button == SDL_BUTTON_WHEELUP) {
			if (pressed)
				map_pos.y -= 128;
			return true;
		} else if (button == SDL_BUTTON_WHEELDOWN) {
			if (pressed)
				map_pos.y += 128;
			return true;
		}
	
	
	}

	if (pressed && button == SDL_BUTTON_LEFT && _brush) {
		if (_layers_dialog->empty()) {
			_layer_name = "add new layer with shift-n";
			_layer_name_invisible.reset();	
			return true;
		}
		//LOG_DEBUG(("click"));
		ObjectBrush *object_brush = dynamic_cast<ObjectBrush *>(_brush);
		if (object_brush != NULL) {
			Command cmd(object_brush->classname, object_brush->animation, 
				v2<int>(map_pos.x + x - _brush->size.x / 2, map_pos.y + y - _brush->size.y / 2), object_brush->z);
			addCommand(cmd);
			currentCommand().exec();
			return true;
		}
		Layer * l = Map->getLayer(_current_layer_z);
		if (l->isVisible()) {
			//LOG_DEBUG(("tile pos: %d %d", x / _tile_size.x, y / _tile_size.y));
			_last_tile_painted = v2<int>((map_pos.x + x)/ _tile_size.x, (map_pos.y + y) / _tile_size.y);
			Command cmd(l);
			addCommand(cmd);
			_brush->exec(currentCommand(), _last_tile_painted.x, _last_tile_painted.y );
		}
		return true;
	} 
	
	if (button == SDL_BUTTON_LEFT && pressed && _highlight_object) {
		_dragging = true;
		LOG_DEBUG(("dragging object..."));
		Command cmd(Command::MoveObject, _highlight_object);
		addCommand(cmd);
		
		Object *object = currentCommand().getObject();
		assert(object != NULL);
		v2<int> pos;
		object->get_position(pos);
		currentCommand().save(pos.x, pos.y);
		return true;
	}
	
	if (button == SDL_BUTTON_LEFT && !pressed && _dragging) {
		_dragging = false;
		LOG_DEBUG(("dropping object..."));
		v2<int> pos;
		currentCommand().getObject()->get_position(pos);
		currentCommand().move(pos.x, pos.y);
		currentCommand().exec();
		return true;
	}
	
	if (button == SDL_BUTTON_RIGHT && !pressed && _highlight_object) {
		_object_properties->set_base(x, y);
		std::set<std::string> variants;
		_add_object->get_variants(variants, _highlight_object->registered_name);
		_object_properties->show(_highlight_object, variants);
		return true;
	}
	
	if (button == SDL_BUTTON_RIGHT && pressed && !_selecting && _highlight_object == NULL) {
		_selecting = true;
		LOG_DEBUG(("start selection"));
		_selection1 = _selection2 = v2<int>(x + map_pos.x, y + map_pos.y);
		return true;
	}
	
	if (button == SDL_BUTTON_RIGHT && !pressed && _selecting) {
		_selecting = false;
		if (_selection1.x > _selection2.x) 
			math::exchange(_selection1.x, _selection2.x);
		if (_selection1.y > _selection2.y) 
			math::exchange(_selection1.y, _selection2.y);
		v2<int> size = _selection2 - _selection1 + 1;
		LOG_DEBUG(("selection: %d,%d %dx%d", _selection1.x, _selection1.y, size.x, size.y));
		return true;
	}
	
	return false;
}


bool Editor::onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel) {
	if (Container::onMouseMotion(state, x, y, xrel, yrel))
		return true;
	const bool lmb = (state & SDL_BUTTON_LMASK) != 0;

	if (_render_objects && _brush == NULL && state == 0 && _add_object->hidden()) {
		//LOG_DEBUG(("mouse %d %d", x, y));
		_highlight_object = World->getObjectByXY((int)map_pos.x + x, (int)map_pos.y + y);
		if (_highlight_object) {
			displayStatusMessage(_highlight_object);
		}
	}
		
	static const Uint8 *keys = SDL_GetKeyState(0);

	if (lmb && _brush != NULL && !_brush->size.is0() && keys[SDLK_SPACE] == 0 && !undo_queue.empty()) { //no space
		v2<int> tile_pos ((map_pos.x + x)/ _tile_size.x, (map_pos.y + y) / _tile_size.y);
		v2<int> dpos = tile_pos - _last_tile_painted;
		v2<int> shift (math::abs(dpos.x), math::abs(dpos.y));
		if (shift.x > shift.y)
			shift.y = 0;
		else
			shift.x = 0;
		
		if (shift.x >= _brush->size.x || shift.y >= _brush->size.y) {
			dpos.x = math::sign(dpos.x);
			dpos.y = math::sign(dpos.y);
			do {
				_last_tile_painted += shift * dpos;
				//LOG_DEBUG(("last tile: %d %d", _last_tile_painted.x, _last_tile_painted.y ));
				_brush->exec(currentCommand(), _last_tile_painted.x, _last_tile_painted.y );
				shift -= _brush->size;
			} while(shift.x >= 0 && shift.y >= 0);
			//LOG_DEBUG(("draw line: tile_pos: %d %d", tile_pos.x, tile_pos.y));
		}
	}
	if (_dragging) {
		v2<int> pos;
		currentCommand().getObject()->get_position(pos);
		pos.x += xrel; pos.y += yrel;

		World->move(currentCommand().getObject(), pos.x, pos.y);
	}
	if (_selecting) {
		_selection2 = v2<int>(map_pos.x + x, map_pos.y + y);
	}
	
	if (keys[SDLK_SPACE] == 0 || !lmb || !Map->loaded())
		return false;

	map_pos.x -= xrel;
	map_pos.y -= yrel;

	return true;
}

void Editor::resetLoadingBar(const int total) {
	_loading_bar_now = 0;
	_loading_bar_total = total;
}

void Editor::notifyLoadingBar(const int progress, const char *what) {
	GET_CONFIG_VALUE("hud.disable-loading-screen", bool, disable_bar, false);
	if (disable_bar)
		return;
	
	float old_progress = 1.0 * _loading_bar_now / _loading_bar_total;
	_loading_bar_now += progress;
	
	if (_hud->renderLoadingBar(Window->get_surface(), old_progress, 1.0 * _loading_bar_now / _loading_bar_total, NULL, false)) {
		Window->flip();
		Window->get_surface().fill(Window->get_surface().map_rgb(255, 255, 255));
	}
}

void Editor::updateLayers() {
	if (!_layers_dialog->empty()) {
		const LayerItem *li = _layers_dialog->getCurrentItem();
		_current_layer_z = li->z;
		_layer_name = li->layer->name;
		_layer_name_invisible.reset();
		_layer_invisible.reset();
	}
}


bool Editor::onKeySignal(const SDL_keysym sym, const bool pressed) {
	if (pressed && onKey(sym))
		return true;
	return false;
}

bool Editor::onMouseSignal(const int button, const bool pressed, const int x, const int y) {
	return onMouse(button, pressed, x, y);
}

bool Editor::onMouseMotionSignal(const int state, const int x, const int y, const int xrel, const int yrel) {
	return onMouseMotion(state, x, y, xrel, yrel);
}

void Editor::onEvent(const SDL_Event &event) {
	if (event.type == SDL_QUIT)
		Window->stop();
}


#ifdef _WINDOWS
#	include "sdlx/SDL_main.h"
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#endif

#include "sdlx/system.h"

#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char *argv[]) {
try {
	LOG_DEBUG(("bt editor"));
	Game->loadPlugins();

	Editor editor;
	editor.init(argc, argv);
	editor.run();
	editor.deinit();
#ifdef _WINDOWS
	} catch(const std::exception &e) {
		sdlx::System::deinit();
		LOG_ERROR(("main:%s", e.what()));
		MessageBox(NULL, e.what(), "Error", MB_OK | MB_ICONERROR | MB_TASKMODAL);
		return 1;
	}
#else 
	} CATCH("main", { 
		sdlx::System::deinit();
		return 1;
	})
#endif
	
	sdlx::System::deinit();
	return 0;
}


#include "mrt/directory.h"
#include <string>

#ifdef _WINDOWS
extern "C" {
#ifdef _WIN32_WCE
int WINAPI SDLWinMain(HINSTANCE hInst, HINSTANCE hPrev, LPWSTR szCmdLine, int sw);
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPWSTR szCmdLine, int sw)
#else
int WINAPI SDLWinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw);
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw)
#endif
{
	std::string logpath = mrt::format_string("SDL_LOG_PATH=%s", mrt::Directory::get_app_dir("Battle Tanks", "btanks").c_str());
	_putenv(_strdup(logpath.c_str()));
	return SDLWinMain(hInst, hPrev, szCmdLine, sw);
}
}
#endif
