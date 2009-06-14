
#ifndef __SIMAGE_H__
#define	__SIMAGE_H__

class SImage {
private:
	int		width;
	int		height;
	BYTE*	data;

public:
	SImage();
	virtual ~SImage();
};

#endif //__SIMAGE_H__
