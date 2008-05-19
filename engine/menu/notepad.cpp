#include "notepad.h"
#include "resource_manager.h"
#include "sdlx/surface.h"
#include "sdlx/font.h"
#include "i18n.h"

Notepad::Notepad(const int w, const std::string &font) : width(0),
tabbg(ResourceManager->loadSurface("menu/background_tab.png")), 
font(ResourceManager->loadFont(font, true)), current_page(0) {
	tab_w = tabbg->getWidth() / 5;
	tab_x1 = 2 * tab_w;
	tab_x2 = tabbg->getWidth() - tab_x1;
	tab_left.x = 0;
	tab_left.y = 0;
	tab_left.w = tab_x1;

	tab_right.x = tab_x2;
	tab_right.y = 0;
	tab_right.w = tab_x1;
	
	tab_bg.x = tab_x1;
	tab_bg.y = 0;
	tab_bg.w = tab_w;

	tab_bg.h = tab_right.h = tab_left.h = tabbg->getHeight();
}

void Notepad::add(const std::string &area, const std::string &label) {
	Page page;
	page.label = I18n->get(area, label);
	pages.push_back(page);
	recalculate_sizes();
}

void Notepad::recalculate_sizes() {
	width = 0;
	for(size_t i = 0; i < pages.size(); ++i) {
		Page &page = pages[i];

		width += tab_x1;
		
		page.tab_rect.x = width;
		page.tab_rect.y = 0;
		page.tab_rect.w = ((font->render(NULL, 0, 0, page.label) - 1) / tab_w + 1) * tab_w;
		page.tab_rect.h = tabbg->getHeight();
		
		width += page.tab_rect.w;
	}
	width += tab_x1;
}

void Notepad::getSize(int &w, int &h) const {
	w = width;
	h = font->getHeight();
}

bool Notepad::onMouse(const int button, const bool pressed, const int x, const int y) {
	for(size_t i = 0; i < pages.size(); ++i) {
		const Page &page = pages[i];
		if (page.tab_rect.in(x, y)) {
			current_page = i;
			invalidate();
			return true;
		}
	}
	return false;
}

void Notepad::render(sdlx::Surface &surface, const int x, const int y) const {
	int xp = x;
	int yp = y + tabbg->getHeight() / 2 - font->getHeight() / 2;
	for(size_t i = 0; i < pages.size(); ++i) {
		const Page &page = pages[i];
		if (i == current_page) {
			surface.copyFrom(*tabbg, tab_left, xp, y);
		}
		xp += tab_left.w;
		if (i == current_page) {
			for(int b = 0; b < page.tab_rect.w / tab_w; ++b) {
				surface.copyFrom(*tabbg, tab_bg, xp + b * tab_bg.w, y);
			}
		}
		font->render(surface, xp, yp, page.label);
		xp += page.tab_rect.w;
		if (i == current_page) {
			surface.copyFrom(*tabbg, tab_right, xp, y);
		}
	}	
}
