#ifndef BTANKS_IMAGE_VIEW_H__
#define BTANKS_IMAGE_VIEW_H__

#include "export_btanks.h"
#include "control.h"
#include "math/v2.h"

class BTANKSAPI ImageView : public Control {
public: 
	ImageView(int w, int h);
	void init(const sdlx::Surface *image);

	v2<float> position, destination;

	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual void getSize(int &w, int &h) const;

	void tick(const float dt);

private: 
	int _w, _h;
	const sdlx::Surface * _image;
};

#endif

