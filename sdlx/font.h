#ifndef __SDLX_FONT_H__
#define __SDLX_FONT_H__

#include <string>

namespace sdlx {

class Surface;
class Font {
public:
	enum Type { AZ09 };
	Font();
	~Font();
	
	void load(const std::string &file, const Type type);
	const int getHeight() const;
	const int render(sdlx::Surface &window, const int x, const int y, const std::string &str) const;
	void clear();

private:
	Font(const Font &);
	const Font& operator=(const Font &);
	Type _type;
	sdlx::Surface *_surface;
};

}

#endif

