#ifndef VIDEOBUFFER_H
#define VIDEOBUFFER_H

#include <string>
#include "graphics.h"

class VideoBuffer
{
	//pixel video map, usually an unsigned int in 0x00RRGGBB format
	pixel* vid;
	int width;
	int height;
public:
	VideoBuffer(int width, int height);
	~VideoBuffer();

	void Clear();
	void ClearRect(int x, int y, int w, int h);
	void CopyVideoBuffer(pixel** vid, int x, int y);

	void DrawPixel(int x, int y, int r, int g, int b, int a);
	void DrawLine(int x1, int y1, int x2, int y2, int r, int g, int b, int a);
	void DrawRect(int x, int y, int w, int h, int r, int g, int b, int a);
	void FillRect(int x, int y, int w, int h, int r, int g, int b, int a);

	int DrawChar(int x, int y, unsigned char c, int r, int g, int b, int a);
	int DrawText(int x, int y, std::string s, int r, int g, int b, int a);

	void DrawImage(pixel *image, int x, int y, int w, int h);

	pixel* GetVid() { return vid; }
};

#endif
