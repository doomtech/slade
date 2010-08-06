
#ifndef __ENTRYDATAFORMAT_H__
#define __ENTRYDATAFORMAT_H__

class EntryDataFormat {
private:
	// Struct to specify a range for a byte (min <= valid >= max)
	// If max == min, only 1 valid value
	struct byte_vrange_t {
		uint8_t	min;
		uint8_t	max;

		byte_vrange_t() { min = 0; max = 255; }
	};

	// Struct to specify valid values for a byte at pos
	struct byte_pattern_t {
		unsigned				pos;
		vector<byte_vrange_t>	valid_values;

		bool match(uint8_t value) {
			for (unsigned a = 0; a < valid_values.size(); a++) {
				if (value < valid_values[a].min || value > valid_values[a].max)
					return false;
			}

			return true;
		}
	};

	// Detection
	unsigned				size_min;
	vector<byte_pattern_t>	patterns;
	// Also needed:
	// Some way to check more complex values (eg. multiply byte 0 and 1, result must be in a certain range)

public:
	EntryDataFormat(string id);
	virtual ~EntryDataFormat();

	virtual bool	isThisFormat(MemChunk& mc);
	void			copyToFormat(EntryDataFormat& target);

	static void				initBuiltinFormats();
	static bool				readDataFormatDefinition(MemChunk& mc);
	static EntryDataFormat*	getFormat(string id);
	static EntryDataFormat*	anyFormat();
};

#endif//__ENTRYDATAFORMAT_H__
