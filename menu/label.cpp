#include "label.h"
#include "sdlx/font.h"

Label::Label(const sdlx::Font *font, const std::string &label) : _font(font), _label(label) {}


void Label::render(sdlx::Surface& surface, int x, int y) {
	_font->render(surface, x, y, _label);
}
