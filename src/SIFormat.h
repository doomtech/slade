
#ifndef __SIFORMAT_H__
#define __SIFORMAT_H__

#include "SImage.h"

class SIFormat {
protected:
	string	id;
	string	name;
	string	extension;

	// Stuff to access protected image data
	uint8_t*		imageData(SImage& image) { return image.data; }
	uint8_t*		imageMask(SImage& image) { return image.mask; }
	Palette8bit&	imagePalette(SImage& image) { return image.palette; }

	virtual bool	readImage(SImage& image, MemChunk& data, int index) = 0;
	virtual bool	writeImage(SImage& image, MemChunk& data, Palette8bit* pal, int index) { return false; }

public:
	// Image info struct
	struct imginfo_t {
		int		width;
		int		height;
		int		colformat;
		string	format;
		int		numimages;
		int		offset_x;
		int		offset_y;

		imginfo_t() {
			width = height = offset_x = offset_y = 0;
			colformat = RGBA;
			numimages = 1;
		}
	};

	// Conversion options stuff
	enum {
		MASK_NONE = 0,
		MASK_COLOUR,
		MASK_ALPHA,
		MASK_BRIGHTNESS,
	};
	struct convert_options_t {
		Palette8bit*	pal_current;
		Palette8bit*	pal_target;
		int				mask_source;
		rgba_t			mask_colour;
		uint8_t			alpha_threshold;
		bool			transparency;
		int				col_format;

		convert_options_t() {
			pal_current = pal_target = NULL;
			mask_source = 0;
			transparency = true;
			col_format = -1;
		}
	};

	SIFormat(string id);
	~SIFormat();

	string	getId() { return id; }
	string	getName() { return name; }
	string	getExtension() { return extension; }

	virtual bool	isThisFormat(MemChunk& mc) = 0;

	// Reading
	virtual imginfo_t	getInfo(MemChunk& mc, int index = 0) = 0;

	bool loadImage(SImage& image, MemChunk& data, int index = 0) {
		// Check format
		if (!isThisFormat(data))
			return false;

		// Attempt to read image data
		bool ok = readImage(image, data, index);

		// Set image properties if successful
		if (ok) {
			image.format = this;
			image.imgindex = index;
		}
		else
			image.clear();

		// Testing
		wxLogMessage("Loaded image format %s successfully", CHR(name));

		return ok;
	}

	// Writing
	enum {
		NOTWRITABLE,	// Format cannot be written
		WRITABLE,		// Format can be written
		CONVERTIBLE,	// Format can be written but a conversion is required
	};
	virtual int		canWrite(SImage& image) { return NOTWRITABLE; }
	virtual bool	canWriteType(SIType type) { return false; }
	virtual bool	convertWritable(SImage& image, convert_options_t opt) { return false; }

	bool saveImage(SImage& image, MemChunk& out, Palette8bit* pal = NULL, int index = 0) {
		// Attempt to write image data
		bool ok = writeImage(image, out, pal, index);

		// Set format if successful
		if (ok)
			image.format = this;

		return ok;
	}

	static void			initFormats();
	static SIFormat*	getFormat(string name);
	static SIFormat*	determineFormat(MemChunk& mc);
	static SIFormat*	unknownFormat();
	static SIFormat*	rawFormat();
	static SIFormat*	flatFormat();
	static void			getAllFormats(vector<SIFormat*>& list);
};

#endif//__SIFORMAT_H__
