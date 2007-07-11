#ifdef WIN32
#	define WINDOWS_LEAN_AND_MEAN
#	include <windows.h>
#else 
#	include <locale.h>
#endif

#include "mrt/locale.h"
#include "logger.h"

const std::string mrt::getLanguageCode() {
#ifdef WIN32
	LANGID lang_id = GetUserDefaultLangID();
	LOG_DEBUG(("GetUserDefaultLangID() returned %08x", (unsigned)lang_id));

	switch(lang_id & 0x3ff) {

	case 0x01: return "ar";
	case 0x02: return "bg";
	case 0x03: return "ca";

	case 0x04: return "zh";
	case 0x05: return "cs";
	case 0x06: return "da";
	case 0x07: return "de";
	
	case 0x08: return "el";
	case 0x09: return "en";
	case 0x0a: return "es";
	case 0x0b: return "fi";

	case 0x0c: return "fr";
	case 0x0d: return "he";
	case 0x0e: return "hu";
	case 0x0f: return "is";

	case 0x10: return "it";
	case 0x11: return "ja";
	case 0x12: return "ko";
	case 0x13: return "nl";
	
	case 0x14: return "nn";
	case 0x15: return "pl";
	case 0x16: return "pt";
	//case 0x17: return ""; //ancient forgotten language

	case 0x18: return "ro";
	case 0x19: return "ru";
	case 0x1a: return "sr";
	case 0x1b: return "sk";

	case 0x1c: return "sq";
	case 0x1d: return "sv";
	case 0x1e: return "th";
	case 0x1f: return "tr";

	case 0x20: return "ur";
	case 0x21: return "id";
	case 0x22: return "uk";
	case 0x23: return "be";

	case 0x24: return "sl";
	case 0x25: return "et";
	case 0x26: return "lv";
	case 0x27: return "lt";

	//case 0x28: return ""; //another forgotten language
	case 0x29: return "fa";
	case 0x2a: return "vi";
	case 0x2b: return "hy";

	case 0x2c: return "az";
	case 0x2d: return "eu";
	case 0x2f: return "mk";

	case 0x36: return "af";
	case 0x37: return "ka";
	case 0x38: return "fo";
	case 0x39: return "hi";
	case 0x3e: return "ms";
	case 0x3f: return "kk";

	case 0x40: return "ky";
	case 0x41: return "sw";
	//case 0x42: return "";
	case 0x43: return "uz";

	case 0x44: return "tt";
	case 0x45: return "bn";
	case 0x46: return "pa";
	case 0x47: return "gu";

	case 0x48: return "or";
	case 0x49: return "ta";
	case 0x4a: return "te";
	case 0x4b: return "kn";

	case 0x4c: return "ml";
	case 0x4d: return "as";
	case 0x4e: return "mr";
	case 0x4f: return "sa";

	case 0x50: return "mn";

	case 0x56: return "gl";
	//case 0x57: return ""; //konkani ! no 2letters iso code

	//case 0x58: return ""; //manipuri
	case 0x59: return "sd";
	case 0x5a: return "sy"; //no code.

	case 0x60: return "ks";
	case 0x61: return "ne";
	case 0x65: return "dv";
	}
#else 
	//non win-32
	const char * lang = setlocale(LC_MESSAGES, NULL);
	LOG_DEBUG(("setlocale(LC_MESSAGES, NULL) returned %s", lang));
	if (lang == NULL)
		return std::string();
#endif
	return std::string();
}
