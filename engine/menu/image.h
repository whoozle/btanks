#ifndef BTANKS_MENU_IMAGE_H__
#define BTANKS_MENU_IMAGE_H__

#include "control.h"


class Image : public Control {
public: 
	Image(const sdlx::Surface *image = NULL);
	void set(const sdlx::Surface *image);

	virtual void render(sdlx::Surface &surface, const int x, const int y) const;
	virtual void get_size(int &w, int &h) const;

private: 
	const sdlx::Surface * image;
};


#endif

