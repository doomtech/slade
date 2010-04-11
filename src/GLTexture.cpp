
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
GLTexture::GLTexture(bool allow_split) {
	this->loaded = false;
	this->allow_split = allow_split;
}

/* GLTexture::GLTexture
 * GLTexture class constructor
 *******************************************************************/
GLTexture::~GLTexture() {
	// Delete current textures if they exist
	if (loaded)
		clear();
}

bool GLTexture::loadData(const uint8_t* data, uint32_t width, uint32_t height, bool add) {
	// Check data was given
	if (!data)
		return false;

	// Delete current textures if they exist
	if (!add && loaded)
		clear();

	// Create texture struct
	gl_tex_t ntex;
	ntex.width = width;
	ntex.height = height;

	// Generate the texture id
	glGenTextures(1, &ntex.id);
	glBindTexture(GL_TEXTURE_2D, ntex.id);

	// Generate the texture
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	// Update variables
	loaded = true;
	this->width = width;
	this->height = height;
	tex.push_back(ntex);

	return true;
}

bool GLTexture::loadImage(SImage* image, Palette8bit* pal) {
	// Check image was given
	if (!image)
		return false;

	// Check image is valid
	if (!image->isValid())
		return false;

	// Clear current texture
	clear();

	// Check image dimensions
	if (OpenGL::validTexDimension(image->getWidth()) && OpenGL::validTexDimension(image->getHeight())) {
		// If the image dimensions are valid for OpenGL on this system, just load it as a single texture

		// Get RGBA image data
		MemChunk rgba;
		image->getRGBAData(rgba, pal);

		// Generate GL texture from rgba data
		return loadData(rgba.getData(), image->getWidth(), image->getHeight());
	}
	else {
		// Otherwise split the image into 128x128 chunks
		int top = 0;
		while (top < image->getHeight()) {
			int left = 0;
			while (left < image->getWidth()) {
				// Load 128x128 portion of image
				loadImagePortion(image, rect_t(left, top, left + 128, top + 128), pal, true);

				// Move right 128px
				left += 128;
			}

			// Move down 128px
			top += 128;
		}

		// Update variables
		width = image->getWidth();
		height = image->getHeight();

		return true;
	}
}

bool GLTexture::loadImagePortion(SImage* image, rect_t rect, Palette8bit* pal, bool add) {
	// Check image was given
	if (!image)
		return false;

	// Check image is valid
	if (!image->isValid())
		return false;

	// Check portion rect is valid
	if (rect.width() <= 0 || rect.height() <= 0)
		return false;

	// Get RGBA image data
	MemChunk rgba;
	image->getRGBAData(rgba, pal);

	// Init texture data
	MemChunk portion;
	portion.reSize(rect.width() * rect.height() * 4, false);
	portion.fillData(0);

	// Read portion of image if rect isn't completely outside the image
	if (!(rect.left() >= image->getWidth() || rect.right() < 0 || rect.top() >= image->getHeight() || rect.bottom() < 0)) {
		// Determine start of each row to read
		uint32_t row_start = 0;
		if (rect.left() > 0)
			row_start = rect.left();

		// Determine width of each row to read
		uint32_t row_width = rect.right() - row_start;
		if (rect.right() >= image->getWidth())
			row_width = image->getWidth() - row_start;

		// Determine difference between the left of the portion and the left of the image
		uint32_t skip = 0;
		if (rect.left() < 0)
			skip = (0 - rect.left()) * 4;

		// Create temp row buffer
		uint8_t* buf = new uint8_t[rect.width() * 4];

		// Go through each row
		for (int32_t row = rect.top(); row < rect.bottom(); row++) {
			// Clear row buffer
			memset(buf, 0, rect.width() * 4);

			// Check that the current row is within the image
			if (row >= 0 && row < image->getHeight()) {
				// Seek to current row in image data
				rgba.seek((row * image->getWidth() + row_start) * 4, SEEK_SET);

				// Copy the row data
				rgba.read(buf + skip, row_width * 4);
			}

			// Write the row
			portion.write(buf, rect.width() * 4);
		}

		// Free buffer
		delete[] buf;
	}

	// Generate texture from rgba data
	return loadData(portion.getData(), rect.width(), rect.height(), add);
}

bool GLTexture::clear() {
	// Delete texture(s)
	for (size_t a = 0; a < tex.size(); a++)
		glDeleteTextures(1, &tex[a].id);
	tex.clear();

	// Reset variables
	width = 0;
	height = 0;
	loaded = false;

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

	// Generate texture from rgba data
	loadData(data, block_size*2, block_size*2);

	// Clean up
	delete[] data;

	return true;
}

bool GLTexture::bind() {
	// Check texture is loaded
	if (!loaded)
		return false;

	// Bind the texture
	glBindTexture(GL_TEXTURE_2D, tex[0].id);

	return true;
}

bool GLTexture::draw2d(double x, double y) {
	// Can't draw if texture not loaded
	if (!loaded)
		return false;

	// If the texture isn't split, just draw it straight
	if (OpenGL::validTexDimension(width) && OpenGL::validTexDimension(height)) {
		// Bind the texture
		glBindTexture(GL_TEXTURE_2D, tex[0].id);

		// Draw
		glBegin(GL_QUADS);
		glTexCoord2d(0, 0);	glVertex2d(x, y);
		glTexCoord2d(0, 1);	glVertex2d(x, y+height);
		glTexCoord2d(1, 1);	glVertex2d(x+width, y+height);
		glTexCoord2d(1, 0); glVertex2d(x+width, y);
		glEnd();
	}

	// Otherwise draw the 128x128 chunks
	else {
		// Translate to position
		glPushMatrix();
		glTranslated(x, y, 0);

		size_t tex_index = 0;
		double top = 0;
		while (top < height) {
			double left = 0;
			while (left < width) {
				// Bind the texture
				glBindTexture(GL_TEXTURE_2D, tex[tex_index].id);

				// Draw
				glBegin(GL_QUADS);
				glTexCoord2d(0, 0);	glVertex2d(left, top);
				glTexCoord2d(0, 1);	glVertex2d(left, top+128);
				glTexCoord2d(1, 1);	glVertex2d(left+128, top+128);
				glTexCoord2d(1, 0); glVertex2d(left+128, top);
				glEnd();

				// Move right 128px
				left += 128;
				tex_index++;
			}

			// Move down 128px
			top += 128;
		}

		glPopMatrix();
	}

	return true;
}

bool GLTexture::draw2dTiled(uint32_t width, uint32_t height) {
	// Can't draw if texture not loaded
	if (!loaded)
		return false;

	// If the texture isn't split, just draw it straight
	if (OpenGL::validTexDimension(width) && OpenGL::validTexDimension(height)) {
		// Bind the texture
		glBindTexture(GL_TEXTURE_2D, tex[0].id);

		// Calculate texture coordinates
		double tex_x = (double)width / (double)this->width;
		double tex_y = (double)height / (double)this->height;

		// Draw
		glBegin(GL_QUADS);
		glTexCoord2d(0, 0);			glVertex2d(0, 0);
		glTexCoord2d(0, tex_y);		glVertex2d(0, height);
		glTexCoord2d(tex_x, tex_y);	glVertex2d(width, height);
		glTexCoord2d(tex_x, 0);		glVertex2d(width, 0);
		glEnd();
	}

	// Otherwise draw the 128x128 chunks
	else {
		uint32_t x = 0;
		while (x < width) {
			uint32_t y = 0;
			while (y < height) {
				// Draw texture
				draw2d(x, y);

				// Move down
				y += this->height;
			}

			// Move right
			x += this->width;
		}
	}

	return true;
}
