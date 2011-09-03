
#ifndef __MAP_TEXTURE_MANAGER_H__
#define __MAP_TEXTURE_MANAGER_H__

#include "GLTexture.h"
#include <wx/hashmap.h>

struct map_tex_t {
	GLTexture*	texture;
	map_tex_t() { texture = NULL; }
	~map_tex_t() { if (texture) delete texture; }
};

// Declare hash map class to hold textures
WX_DECLARE_STRING_HASH_MAP(map_tex_t, MapTexHashMap);

class Archive;
class MapTextureManager {
private:
	Archive*		archive;
	MapTexHashMap	textures;
	MapTexHashMap	flats;
	MapTexHashMap	thing_images;
	bool			thing_images_loaded;

public:
	MapTextureManager(Archive* archive = NULL);
	~MapTextureManager();

	void	setArchive(Archive* archive) { this->archive = archive; }

	GLTexture*	getTexture(string name);
	GLTexture*	getFlat(string name);
	GLTexture*	getThingImage(string name);
};

#endif//__MAP_TEXTURE_MANAGER_H__
