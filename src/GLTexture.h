
#ifndef __GLTEXTURE_H__
#define	__GLTEXTURE_H__

class GLTexture {
private:
	int		width;
	int		height;
	GLuint	id;

public:
	GLTexture();
	~GLTexture();

	bool	loadData(const uint8_t* data, int width, int height);
};

#endif//__GLTEXTURE_H__
