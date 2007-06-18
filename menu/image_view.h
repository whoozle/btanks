#ifndef BTANKS_IMAGE_VIEW_H__
#define BTANKS_IMAGE_VIEW_H__

#include "export_btanks.h"
#include "container.h"
#include "math/v2.h"

class Box;

class BTANKSAPI ImageView : public Container {
public: 
	ImageView(int w, int h);
	void init(const sdlx::Surface *image);

	v2<float> position, destination;
	
	void setOverlay(const sdlx::Surface *overlay, const v2<int> &dpos);

	virtual void render(sdlx::Surface &surface, const int x, const int y);

	void tick(const float dt);

private: 
	int _w, _h;
	const sdlx::Surface * _image, *_overlay;
	v2<int> _overlay_dpos;
	Box * _box;
};

#endif

