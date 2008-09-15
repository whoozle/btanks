#ifndef BTANKS_MENU_TABLE_H__
#define BTANKS_MENU_TABLE_H__

#include "control.h"
#include "math/matrix.h"

class Grid : public Control {
public: 
	enum Align { None = 0, Left = 1, Right = 2, Center = 3, Top = 4, Bottom = 8, Middle = 12};

	Grid(const int w, const int h);
	void set_spacing(const int spacing) { _spacing = spacing; }
	~Grid();
	
	void set(const int row, const int col, Control *ctrl, const int align = None);
	void set_span(const int row, const int col, const int rowspan, const int colspan);

	virtual void render(sdlx::Surface &surface, const int x, const int y) const;
	virtual void get_size(int &w, int &h) const;
	
	virtual bool onKey(const SDL_keysym sym);
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);
	virtual bool onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel);

	void recalculate(const int w = 0, const int h = 0);

private: 
	struct ControlDescriptor {
		ControlDescriptor() : c(NULL), align(0), colspan(1), rowspan(1) {}
		Control *c;
		int align;
		int colspan, rowspan;
	};
	
	ControlDescriptor * find(int& x, int& y);
	
	typedef std::vector<ControlDescriptor> Row;
	typedef std::vector<Row> Matrix;
	Matrix _controls;
	
	std::vector<int> _split_w, _split_h;
	int _spacing;
};

#endif

