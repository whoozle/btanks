#include "start_server_menu.h"
#include "sdlx/surface.h"
#include "mrt/logger.h"
#include "mrt/exception.h"
#include "map_picker.h"

StartServerMenu::StartServerMenu(const int w, const int h) : _w(w), _h(h) {
	UpperBox * upper_box = NULL;
	TRY {
		upper_box = new UpperBox; 
		upper_box->init(500, 80, true);

		sdlx::Rect r((w - upper_box->w) / 2, 32, upper_box->w, upper_box->h);
		add(r, upper_box);
		upper_box = NULL;
	} CATCH("StartServerMenu", {delete upper_box; throw; });
	add(sdlx::Rect(0, 128, w, h - 128), new MapPicker(w, h - 128));
}
