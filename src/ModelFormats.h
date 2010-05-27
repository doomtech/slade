#ifndef MODELFORMATS_H
#define MODELFORMATS_H

class DMDModelDataFormat : public EntryDataFormat {
public:
	DMDModelDataFormat() : EntryDataFormat("mesh_dmd") {};
	~DMDModelDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 4) {
			// Check for DMDM header
			if (mc[0] == 'D' && mc[1] == 'M' && mc[2] == 'D' && mc[3] == 'M')
				return true;
		}
		return false;
	}
};

class MDLModelDataFormat : public EntryDataFormat {
public:
	MDLModelDataFormat() : EntryDataFormat("mesh_mdl") {};
	~MDLModelDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 4) {
			// Check for IDPO header
			if (mc[0] == 'I' && mc[1] == 'D' && mc[2] == 'P' && mc[3] == 'O')
				return true;
		}
		return false;
	}
};

class MD2ModelDataFormat : public EntryDataFormat {
public:
	MD2ModelDataFormat() : EntryDataFormat("mesh_md2") {};
	~MD2ModelDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 4) {
			// Check for IDP2 header
			if (mc[0] == 'I' && mc[1] == 'D' && mc[2] == 'P' && mc[3] == '2')
				return true;
		}
		return false;
	}
};

class MD3ModelDataFormat : public EntryDataFormat {
public:
	MD3ModelDataFormat() : EntryDataFormat("mesh_md3") {};
	~MD3ModelDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 4) {
			// Check for IDP3 header
			if (mc[0] == 'I' && mc[1] == 'D' && mc[2] == 'P' && mc[3] == '3')
				return true;
		}
		return false;
	}
};

#endif //MODELFORMATS_H
