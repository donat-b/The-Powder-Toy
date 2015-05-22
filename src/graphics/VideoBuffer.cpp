#ifdef NEWINTERFACE
#include "VideoBuffer.h"
#define INCLUDE_FONTDATA
#include "font.h"

VideoBuffer::VideoBuffer(int width, int height):
	width(width),
	height(height)
{
	vid = new pixel[width*height];
	Clear();
}

VideoBuffer::~VideoBuffer()
{
	delete[] vid;
}

void VideoBuffer::Clear()
{
	std::fill(vid, vid+(width*height), 0);
}

void VideoBuffer::ClearRect(int x, int y, int w, int h)
{
	if (x < 0)
	{
		w += x;
		x = 0;
	}
	if (y < 0)
	{
		h += y;
		y = 0;
	}
	if (x+w >= width)
		w = width-x;
	if (y+h >= height)
		h = height-y;
	if (w<0 || h<0)
		return;

	for (int i = 0; i < h; i++)
		memset(vid+(x+width*(y+i)), 0, sizeof(pixel)*w);
}

void VideoBuffer::CopyVideoBuffer(pixel** vidPaste, int x, int y)
{
	for (int i = 0; i < height; i++)
	{
		memcpy(*vidPaste+x+((y+i)*(XRES+BARSIZE)), vid+(width*i), sizeof(pixel)*width);
	}
}

//This function does NO bounds checking
TPT_INLINE void VideoBuffer::DrawPixel(int x, int y, int r, int g, int b, int a)
{
	//if (x < 0 || y < 0 || x >= width || y >= height || a == 0)
	//	return;
	if (a == 0)
		return;
	if (a != 255)
	{
		pixel t = vid[y*width+x];
		r = (a*r + (255-a)*PIXR(t)) >> 8;
		g = (a*g + (255-a)*PIXG(t)) >> 8;
		b = (a*b + (255-a)*PIXB(t)) >> 8;
	}
	vid[y*width+x] = PIXRGB(r,g,b);
}

void VideoBuffer::DrawLine(int x1, int y1, int x2, int y2, int r, int g, int b, int a)
{
	int dx, dy, sx, sy, e, x, y;
	bool reverseXY = false;

	dx = abs(x1-x2);
	dy = abs(y1-y2);
	sx = (x1<x2) ? 1 : -1;
	sy = (y1<y2) ? 1 : -1;
	x = x1;
	y = y1;

	if (dy > dx)
	{
		dx = dx + dy;
		dy = dx - dy;
		dx = dx - dy;
		reverseXY = true;
	}

	e = (dy<<2) - dx;
	for (int i = 0; i <= dx; i++)
	{
		if (x >= 0 && y >= 0 && x < width && y < height)
			DrawPixel(x, y, r, g, b, a);
		if (e >= 0)
		{
			if (reverseXY)
				x = x + sx;
			else
				y = y + sy;
			e = e - (dx<<2);
		}
		if (reverseXY)
			y = y + sy;
		else
			x = x + sx;
		e = e + (dy<<2);
	}
}

void VideoBuffer::DrawRect(int x, int y, int w, int h, int r, int g, int b, int a)
{
	if (x < 0)
	{
		w += x;
		x = 0;
	}
	if (y < 0)
	{
		h += y;
		y = 0;
	}
	for (int i = 0; i < w && x+i < width; i++)
	{
		if (y < height)
			DrawPixel(x+i, y, r, g, b, a);
		if (y+h < height)
			DrawPixel(x+i, y+h-1, r, g, b, a);
	}
	for (int i = 1; i < h-1 && y+i < height; i++)
	{
		if (x < width)
			DrawPixel(x, y+i, r, g, b, a);
		if (x+w < width)
			DrawPixel(x+w-1, y+i, r, g, b, a);
	}
}

//draws a rectangle and fills it in as well.
void VideoBuffer::FillRect(int x, int y, int w, int h, int r, int g, int b, int a)
{
	if (x < 0)
	{
		w += x;
		x = 0;
	}
	if (y < 0)
	{
		h += y;
		y = 0;
	}
	if (x+w >= width)
		w = width-x;
	if (y+h >= height)
		h = height-y;

	for (int j = 0; j < h; j++)
		for (int i = 0; i < w; i++)
			DrawPixel(x+i, y+j, r, g, b, a);
}

TPT_INLINE int VideoBuffer::DrawChar(int x, int y, unsigned char c, int r, int g, int b, int a)
{
	int w, bn = 0, ba = 0;
	char *rp = (char*)font_data + font_ptrs[c];
	w = *(rp++);
	for (int j = 0; j < FONT_H; j++)
		for (int i = 0; i < w; i++)
		{
			if (!bn)
			{
				ba = *(rp++);
				bn = 8;
			}
			if (x+i >= 0 && y+j >= 0 && x+i < width && y+j < height && a != 0)
				DrawPixel(x+i, y+j, r, g, b, ((ba&3)*a)/3);
			ba >>= 2;
			bn -= 2;
		}
	return x + w;
}

int VideoBuffer::DrawText(int x, int y, const char *s, int r, int g, int b, int a)
{
	int sx = x;
	bool highlight = false;
	int oR = r, oG = g, oB = b;
	for (; *s; s++)
	{
		if (*s == '\n' || *s == '\r')
		{
			x = sx;
			y += FONT_H+2;
			if (highlight && (s[1] == '\n' || s[1] == '\r' || (s[1] == '\x01' && (s[2] == '\n' || s[2] == '\r'))))
			{
				FillRect(x, y-2, font_data[font_ptrs[' ']], FONT_H+2, 0, 0, 255, 127);
			}
		}
		else if (*s == '\x0F')
		{
			if(!s[1] || !s[2] || !s[3]) break;
			oR = r;
			oG = g;
			oB = b;
			r = (unsigned char)s[1];
			g = (unsigned char)s[2];
			b = (unsigned char)s[3];
			s += 3;
		}
		else if (*s == '\x0E')
		{
			r = oR;
			g = oG;
			b = oB;
		}
		else if (*s == '\x01')
		{
			highlight = !highlight;
		}
		else if (*s == '\x02')
		{
			DrawLine(x, y-1, x, y+FONT_H-2, 255, 255, 255, 255);
		}
		else if (*s == '\b')
		{
			switch (s[1])
			{
			case 'w':
				r = g = b = 255;
				break;
			case 'g':
				r = g = b = 192;
				break;
			case 'o':
				r = 255;
				g = 216;
				b = 32;
				break;
			case 'r':
				r = 255;
				g = b = 0;
				break;
			case 'l':
				r = 255;
				g = b = 75;
				break;
			case 'b':
				r = g = 0;
				b = 255;
				break;
			case 't':
				b = 255;
				g = 170;
				r = 32;
				break;
			case 'p':
				b = 100;
				g = 10;
				r = 100;
				break;
			}
			s++;
		}
		else
		{
			int oldX = x;
			x = DrawChar(x, y, *(unsigned char *)s, r, g, b, a);
			if (highlight)
			{
				FillRect(oldX, y-2, font_data[font_ptrs[(int)(*(unsigned char *)s)]], FONT_H+2, 0, 0, 255, 127);
			}
		}
	}
	return x;
}

void VideoBuffer::DrawImage(pixel *img, int x, int y, int w, int h)
{
	for (int j = 0; j < h; j++)
		for (int i = 0; i < w; i++)
		{
			int r = PIXR(*img);
			int g = PIXG(*img);
			int b = PIXB(*img);
			DrawPixel(x+i, y+j, r, g, b, 255);
			img++;
		}
}

#endif
