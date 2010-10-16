
#include "Main.h"
#include "SFont.h"
#include "SImage.h"
#include "ArchiveManager.h"


SFont SFont::font_vga;
SFont SFont::font_slade;


SFontChar::SFontChar() {
	// Init variables
	width = 0;
	height = 0;
}

SFontChar::~SFontChar() {
}



SFont::SFont() {
	// Init character map
	for (unsigned a = 0; a < 256; a++)
		characters[a] = NULL;
}

SFont::~SFont() {
}

bool SFont::loadFont0(MemChunk& mc) {
	return false;
}

bool SFont::loadFont1(MemChunk& mc) {
	return false;
}

bool SFont::loadFont2(MemChunk& mc) {
	return false;
}

bool SFont::loadFontM(MemChunk& mc) {
	// Check data
	if (mc.getSize() == 0 || mc.getSize() % 256)
		return false;

	// Init variables
	unsigned width = 256;
	unsigned height = 256;
	SImage image;

	// Determine character size
	unsigned charwidth = 8;
	unsigned charheight = mc.getSize() >> 8;
	line_height = charheight + 1;

	// Setup image
	image.resize(width, height);

	// Draw characters to image
	unsigned xoff = 0;
	unsigned yoff = 0;
	unsigned i = 0;
	bool test = true;
	for (unsigned a = 0; a < 256; a++) {
		// Setup character info
		SFontChar* c = new SFontChar();
		c->tex_bounds.set(xoff, yoff, xoff+charwidth, yoff+charheight);
		c->width = charwidth;
		c->height = charheight;
		characters[a] = c;

		// Write rows to image
		uint8_t val = 255;
		for (unsigned row = 0; row < charheight; row++) {
			for (unsigned p = 0; p < charwidth; ++p) {
				val = ((mc[i]>>(7-p)) & 1) * 255;
				image.setPixel(xoff + p, yoff + row, rgba_t(val, val, val, val));
			}
			i++;
		}

		// Go to next character
		xoff += charwidth;
		if (xoff >= 256) {
			xoff = 0;
			yoff += charheight;
		}

		test = !test;
	}

	// Load the generated image to the font texture
	texture.loadImage(&image);

	return true;

	/*
	// Check data
	if (!gfx_data || size % 256)
		return false;

	// Setup variables
	offset_x = offset_y = 0;
	has_palette = false;
	format = PALMASK;

	size_t charwidth = 8;
	size_t charheight = size>>8;
	width = charwidth;
	height = charheight << 8;

	if (width * height != size * 8)
		return false;

	// reset data
	clearData();
	data = new uint8_t[width*height];
	memset(data, 0xFF, width*height);
	mask = new uint8_t[width*height];
	memset(mask, 0x00, width*height);

	// Technically each character is its own image, though.
	numimages = 1;
	imgindex = 0;

	//Each pixel is described as a single bit, either on or off
	for (size_t i = 0; i < (unsigned)size; ++i) {
		for (size_t p = 0; p < 8; ++p)
			mask[(i*8)+p] = ((gfx_data[i]>>(7-p)) & 1) * 255;
	}
	return true;
	*/

	return false;
}

bool SFont::loadBMF(MemChunk& mc) {
	return false;
}

void SFont::drawCharacter(char c, rgba_t colour) {
	// Bind texture
	if (!texture.bind())
		return;

	// Set colour
	colour.set_gl();

	// Get character to draw
	SFontChar* ch = characters[c];
	if (!ch)
		return;

	// Draw it
	frect_t tex_rect;
	tex_rect.tl.set((double)ch->tex_bounds.x1() / (double)texture.getWidth(), (double)ch->tex_bounds.y1() / (double)texture.getHeight());
	tex_rect.br.set((double)ch->tex_bounds.x2() / (double)texture.getWidth(), (double)ch->tex_bounds.y2() / (double)texture.getHeight());
	glBegin(GL_QUADS);
	glTexCoord2d(tex_rect.x1(), tex_rect.y1());		glVertex2d(0, 0);
	glTexCoord2d(tex_rect.x1(), tex_rect.y2());		glVertex2d(0, ch->height);
	glTexCoord2d(tex_rect.x2(), tex_rect.y2());		glVertex2d(ch->width, ch->height);
	glTexCoord2d(tex_rect.x2(), tex_rect.y1());		glVertex2d(ch->width, 0);
	glEnd();
}

void SFont::drawString(string str, rgba_t colour, uint8_t align) {
	// Bind texture
	if (!texture.bind())
		return;

	// Set colour
	colour.set_gl();

	// Determine string length (for alignment)
	int total_width = 0;
	for (unsigned a = 0; a < str.size(); a++) {
		// Get character
		SFontChar* ch = characters[chr(str)[a]];

		// Increment total width
		if (ch)
			total_width += ch->width + 1;
		else
			total_width++;
	}

	// Translate for alignment
	glPushMatrix();
	if (align == SF_ALIGN_RIGHT)
		glTranslated(-total_width, 0, 0);
	else if (align == SF_ALIGN_CENTER)
		glTranslated(-(total_width*0.5), 0, 0);

	// Draw the string
	unsigned xoff = 0;
	for (unsigned a = 0; a < str.size(); a++) {
		// Get character
		SFontChar* ch = characters[chr(str)[a]];
		if (!ch) {
			xoff++;
			continue;
		}

		// Draw it
		frect_t tex_rect;
		tex_rect.tl.set((double)ch->tex_bounds.x1() / (double)texture.getWidth(), (double)ch->tex_bounds.y1() / (double)texture.getHeight());
		tex_rect.br.set((double)ch->tex_bounds.x2() / (double)texture.getWidth(), (double)ch->tex_bounds.y2() / (double)texture.getHeight());
		glBegin(GL_QUADS);
		glTexCoord2d(tex_rect.x1(), tex_rect.y1());		glVertex2d(xoff, 0);
		glTexCoord2d(tex_rect.x1(), tex_rect.y2());		glVertex2d(xoff, ch->height);
		glTexCoord2d(tex_rect.x2(), tex_rect.y2());		glVertex2d(xoff+ch->width, ch->height);
		glTexCoord2d(tex_rect.x2(), tex_rect.y1());		glVertex2d(xoff+ch->width, 0);
		glEnd();

		// Increment x to next character (1 pixel spacing)
		xoff += ch->width + 1;
	}

	glPopMatrix();
}


SFont& SFont::sladeFont() {
	if (!font_slade.texture.isLoaded()) {
		// Load slade font
	}

	return font_slade;
}

SFont& SFont::vgaFont() {
	if (!font_vga.texture.isLoaded()) {
		// Get vga font entry
		ArchiveEntry* entry_vgafont = theArchiveManager->programResourceArchive()->entryAtPath("vga-rom-font.16");

		// Load font
		if (entry_vgafont)
			font_vga.loadFontM(entry_vgafont->getMCData());
	}

	return font_vga;
}
