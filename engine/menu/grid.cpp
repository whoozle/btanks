#include "grid.h"

Grid::Grid(const int w, const int h) {
	_controls.resize(h);
	for(int i = 0; i < h; ++i) {
		_controls[i].resize(w);
	}
	_split_w.resize(w);
	_split_h.resize(h);
}

Grid::~Grid() {
	for(size_t i = 0; i < _controls.size(); ++i) {
		for(size_t j = 0; j < _controls[i].size(); ++j) {
			delete _controls[i][j].c;
		}
	}

}

void Grid::set(const int r, const int c, Control *ctrl, const int align) {
	if (r < 0 || r >= (int)_controls.size())
		throw_ex(("set(%d, %d) is out of range", r, c));
	Row &row = _controls[r];
	if (c < 0 || c >= (int) row.size())
		throw_ex(("set(%d, %d) is out of range", r, c));

	ControlDescriptor &d = row[c];
	delete d.c;
	d.c = ctrl;
	d.align = align;
	
	recalculate();
}

void Grid::render(sdlx::Surface &surface, const int x, const int y) const {
	int yp = y;
	for(size_t i = 0; i < _controls.size(); ++i) {
		int xp = x;
		const Row &row = _controls[i];
		for(size_t j = 0; j < row.size(); ++j) {
			const ControlDescriptor &d = row[j];
			if (d.c == NULL)
				continue;
			d.c->render(surface, xp, yp);
			xp += _split_w[j];
		}
		yp += _split_h[i];
	}
}

void Grid::recalculate() {
	for(size_t i = 0; i < _split_w.size(); ++i) {
		_split_w[i] = 0;
	}
	for(size_t i = 0; i < _split_h.size(); ++i) {
		_split_h[i] = 0;
	}
	
	for(size_t i = 0; i < _controls.size(); ++i) {
		const Row &row = _controls[i];
		for(size_t j = 0; j < row.size(); ++j) {
			Control *c = row[j].c;
			if (c == NULL)
				continue;
			int cw, ch;
			c->getSize(cw, ch);
			if (cw > _split_w[j]) {
				_split_w[j] = cw;
			}
			if (ch > _split_h[i]) {
				_split_h[i] = ch;
			}
		}
	}
}

void Grid::getSize(int &w, int &h) const {
	w = h = 0;

	for(size_t i = 0; i < _split_w.size(); ++i) 
		w += _split_w[i];

	for(size_t j = 0; j < _split_h.size(); ++j) 
		h += _split_h[j];
}
	
bool Grid::onKey(const SDL_keysym sym) {
	for(size_t i = 0; i < _controls.size(); ++i) {
		Row &row = _controls[i];
		for(size_t j = 0; j < row.size(); ++j) {
			if (row[j].c != NULL &&	row[j].c->onKey(sym))
				return true;
		}
	}
	return false;
}

bool Grid::onMouse(const int button, const bool pressed, const int x, const int y) {
	return false;
}

bool Grid::onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel) {
	return false;
}
