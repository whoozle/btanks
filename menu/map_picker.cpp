#include "map_picker.h"
#include "scroll_list.h"
#include "mrt/exception.h"

MapPicker::MapPicker(const int w, const int h) {
	sdlx::Rect list_pos(0, 0, (w - 128)/3, h / 2);
	list_pos.x = 64;
	list_pos.y = 64;
	ScrollList *list = NULL;
	TRY {
		list = new ScrollList(list_pos.w, list_pos.h);
		for(char i = 'A'; i <= 'Z'; ++i)
			list->add(mrt::formatString("item %c", i));
		add(list_pos, list);
	} CATCH("MapPicker::ctor", {delete list; throw; });
}
