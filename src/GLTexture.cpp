
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    GLTexture.cpp
 * Description: Simple class to handle an OpenGL texture
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *******************************************************************/


/*******************************************************************
 * INCLUDES
 *******************************************************************/
#include "Main.h"
#include "GLTexture.h"
#include "OpenGL.h"


/*******************************************************************
 * GLTEXTURE CLASS FUNCTIONS
 *******************************************************************/

/* GLTexture::GLTexture
 * GLTexture class constructor
 *******************************************************************/
GLTexture::GLTexture() {
	loaded = false;
}

/* GLTexture::GLTexture
 * GLTexture class constructor
 *******************************************************************/
GLTexture::~GLTexture() {
}

bool GLTexture::loadImage(SImage* image) {
	// Check image was given
	if (!image)
		return false;

	// Check image is valid
	if (!image->isValid())
		return false;

	// Delete current texture if it exists
	if (loaded)
		glDeleteTextures(1, &id);

	// Get RGBA image data
	MemChunk rgba;
	image->getRGBAData(rgba);

	// Generate the texture id
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	// Generate the texture
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, image->getWidth(), image->getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba.getData());

	// Update variables
	loaded = true;
	width = image->getWidth();
	height = image->getHeight();

	return true;
}

bool GLTexture::genChequeredTexture(uint8_t block_size, rgba_t col1, rgba_t col2) {
	// Check given block size and change if necessary
	for (uint8_t s = 1; s <= 64; s *= 2) {
		if (block_size <= s) {
			block_size = s;
			break;
		}
	}

	// Generate rgba data
	uint32_t data_size = (block_size*block_size*4) * 4;
	uint8_t* data = new uint8_t[data_size];
	uint32_t c = 0;
	for (uint32_t y = 0; y < block_size; y++) {		// Top half
		for (uint32_t x = 0; x < block_size; x++) {
			data[c++] = col1.r;
			data[c++] = col1.g;
			data[c++] = col1.b;
			data[c++] = 255;
		}
		for (uint32_t x = 0; x < block_size; x++) {
			data[c++] = col2.r;
			data[c++] = col2.g;
			data[c++] = col2.b;
			data[c++] = 255;
		}
	}
	for (uint32_t y = 0; y < block_size; y++) {		// Bottom half
		for (uint32_t x = 0; x < block_size; x++) {
			data[c++] = col2.r;
			data[c++] = col2.g;
			data[c++] = col2.b;
			data[c++] = 255;
		}
		for (uint32_t x = 0; x < block_size; x++) {
			data[c++] = col1.r;
			data[c++] = col1.g;
			data[c++] = col1.b;
			data[c++] = 255;
		}
	}

	// Delete current texture if it exists
	if (loaded)
		glDeleteTextures(1, &id);

	// Generate the texture id
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	// Generate the texture
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, block_size*2, block_size*2, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	// Update variables
	width = block_size*2;
	height = block_size*2;
	loaded = true;

	// Clean up
	delete[] data;

	return true;
}

bool GLTexture::bind() {
	// Check texture is loaded
	if (!loaded)
		return false;

	// Bind the texture
	glBindTexture(GL_TEXTURE_2D, id);

	return true;
}
