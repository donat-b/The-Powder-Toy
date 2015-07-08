#include <cstdlib>
#include <cstring>
#include <cmath>
#include "bzlib.h"
#include "Pixel.h"

char * generate_gradient(pixel * colours, float * points, int pointcount, int size)
{
	char * newdata = (char*)malloc(size * 3);
	memset(newdata, 0, size*3);
	//Sort the Colours and Points
	for (int i = (pointcount - 1); i > 0; i--)
	{
		for (int j = 1; j <= i; j++)
		{
			if (points[j-1] > points[j])
			{
				float temp = points[j-1];
				points[j-1] = points[j];
				points[j] = temp;

				pixel ptemp = colours[j-1];
				colours[j-1] = colours[j];
				colours[j] = ptemp;
			}
		}
	}
	int i = 0;
	int j = 1;
	float poss = points[i];
	float pose = points[j];
	for (int cp = 0; cp < size; cp++)
	{
		float cpos = (float)cp / (float)size, ccpos, cccpos;
		if (cpos > pose && j+1 < pointcount)
		{
			poss = points[++i];
			pose = points[++j];
		}
		ccpos = cpos - poss;
		cccpos = ccpos / (pose - poss);
		if (cccpos > 1.0f)
			cccpos = 1.0f;
		newdata[(cp*3)] = (char)(PIXR(colours[i])*(1.0f-cccpos) + PIXR(colours[j])*(cccpos));
		newdata[(cp*3)+1] = (char)(PIXG(colours[i])*(1.0f-cccpos) + PIXG(colours[j])*(cccpos));
		newdata[(cp*3)+2] = (char)(PIXB(colours[i])*(1.0f-cccpos) + PIXB(colours[j])*(cccpos));
	}
	return newdata;
}

void * ptif_pack(pixel *src, int w, int h, int *result_size)
{
	int datalen = (w*h)*3;
	unsigned char *red_chan = (unsigned char*)calloc(1, w*h);
	unsigned char *green_chan = (unsigned char*)calloc(1, w*h);
	unsigned char *blue_chan = (unsigned char*)calloc(1, w*h);
	unsigned char *data = (unsigned char*)malloc(((w*h)*3)+8);
	unsigned char *result = (unsigned char*)malloc(((w*h)*3)+8);

	for (int cx = 0; cx<w; cx++)
	{
		for (int cy = 0; cy<h; cy++)
		{
			red_chan[w*(cy)+(cx)] = PIXR(src[w*(cy)+(cx)]);
			green_chan[w*(cy)+(cx)] = PIXG(src[w*(cy)+(cx)]);
			blue_chan[w*(cy)+(cx)] = PIXB(src[w*(cy)+(cx)]);
		}
	}

	memcpy(data, red_chan, w*h);
	memcpy(data+(w*h), green_chan, w*h);
	memcpy(data+((w*h)*2), blue_chan, w*h);
	free(red_chan);
	free(green_chan);
	free(blue_chan);

	result[0] = 'P';
	result[1] = 'T';
	result[2] = 'i';
	result[3] = 1;
	result[4] = w;
	result[5] = w>>8;
	result[6] = h;
	result[7] = h>>8;

	int i = (w*h)*3-8;
	if (BZ2_bzBuffToBuffCompress((char *)(result+8), (unsigned *)&i, (char *)data, datalen, 9, 0, 0) != BZ_OK)
	{
		free(data);
		free(result);
		return NULL;
	}

	*result_size = i+8;
	free(data);
	return result;
}

pixel * ptif_unpack(void *datain, int size, int *w, int *h)
{
	if (size<16)
	{
		printf("Image empty\n");
		return NULL;
	}
	unsigned char *data = (unsigned char*)datain;
	if (!(data[0]=='P' && data[1]=='T' && data[2]=='i'))
	{
		printf("Image header invalid\n");
		return NULL;
	}

	int width = data[4]|(data[5]<<8);
	int height = data[6]|(data[7]<<8);
	int i = (width*height)*3;
	unsigned char *undata = (unsigned char*)calloc(1, (width*height)*3);
	unsigned char *red_chan = (unsigned char*)calloc(1, width*height);
	unsigned char *green_chan = (unsigned char*)calloc(1, width*height);
	unsigned char *blue_chan = (unsigned char*)calloc(1, width*height);
	pixel *result = (pixel*)calloc(width*height, PIXELSIZE);

	int resCode = BZ2_bzBuffToBuffDecompress((char *)undata, (unsigned *)&i, (char *)(data+8), size-8, 0, 0);
	if (resCode)
	{
		printf("Decompression failure, %d\n", resCode);
		free(red_chan);
		free(green_chan);
		free(blue_chan);
		free(undata);
		free(result);
		return NULL;
	}
	if (i != (width*height)*3)
	{
		printf("Result buffer size mismatch, %d != %d\n", i, (width*height)*3);
		free(red_chan);
		free(green_chan);
		free(blue_chan);
		free(undata);
		free(result);
		return NULL;
	}
	memcpy(red_chan, undata, width*height);
	memcpy(green_chan, undata+(width*height), width*height);
	memcpy(blue_chan, undata+((width*height)*2), width*height);

	for (int cx = 0; cx < width; cx++)
	{
		for (int cy = 0; cy < height; cy++)
		{
			result[width*(cy)+(cx)] = PIXRGB(red_chan[width*(cy)+(cx)], green_chan[width*(cy)+(cx)], blue_chan[width*(cy)+(cx)]);
		}
	}

	*w = width;
	*h = height;
	free(red_chan);
	free(green_chan);
	free(blue_chan);
	free(undata);
	return result;
}

pixel * resample_img_nn(pixel * src, int sw, int sh, int rw, int rh)
{
	pixel *q = (pixel*)malloc(rw*rh*PIXELSIZE);
	for (int y = 0; y < rh; y++)
		for (int x = 0; x < rw; x++)
			q[rw*y+x] = src[sw*(y*sh/rh)+(x*sw/rw)];
	return q;
}

//TODO: Actual resampling, this is just cheap nearest pixel crap
pixel * resample_img(pixel *src, int sw, int sh, int rw, int rh)
{
	if (rw == sw && rh == sh)
	{
		//Don't resample
		pixel *q = (pixel*)malloc(rw*rh*PIXELSIZE);
		memcpy(q, src, rw*rh*PIXELSIZE);
		return q;
	}
	else if (rw > sw && rh > sh)
	{
		int fxceil, fyceil;
		float fx, fy, fxc, fyc;
		double intp;
		pixel tr, tl, br, bl;
		pixel *q = (pixel*)malloc(rw*rh*PIXELSIZE);
		//Bilinear interpolation for upscaling
		for (int y = 0; y < rh; y++)
			for (int x = 0; x < rw; x++)
			{
				fx = ((float)x)*((float)sw)/((float)rw);
				fy = ((float)y)*((float)sh)/((float)rh);
				fxc = (float)modf(fx, &intp);
				fyc = (float)modf(fy, &intp);
				fxceil = (int)ceil(fx);
				fyceil = (int)ceil(fy);
				if (fxceil>=sw) fxceil = sw-1;
				if (fyceil>=sh) fyceil = sh-1;
				tr = src[sw*(int)floor(fy)+fxceil];
				tl = src[sw*(int)floor(fy)+(int)floor(fx)];
				br = src[sw*fyceil+fxceil];
				bl = src[sw*fyceil+(int)floor(fx)];
				q[rw*y+x] = PIXRGB(
					(int)(((((float)PIXR(tl))*(1.0f-fxc))+(((float)PIXR(tr))*(fxc)))*(1.0f-fyc) + ((((float)PIXR(bl))*(1.0f-fxc))+(((float)PIXR(br))*(fxc)))*(fyc)),
					(int)(((((float)PIXG(tl))*(1.0f-fxc))+(((float)PIXG(tr))*(fxc)))*(1.0f-fyc) + ((((float)PIXG(bl))*(1.0f-fxc))+(((float)PIXG(br))*(fxc)))*(fyc)),
					(int)(((((float)PIXB(tl))*(1.0f-fxc))+(((float)PIXB(tr))*(fxc)))*(1.0f-fyc) + ((((float)PIXB(bl))*(1.0f-fxc))+(((float)PIXB(br))*(fxc)))*(fyc))
					);
			}
		return q;
	}
	else
	{
		//Stairstepping
		int fxceil, fyceil;
		float fx, fy, fyc, fxc;
		double intp;
		pixel tr, tl, br, bl;
		int rrw = rw, rrh = rh;
		pixel *q;
		pixel * oq = (pixel*)malloc(sw*sh*PIXELSIZE);
		memcpy(oq, src, sw*sh*PIXELSIZE);
		rw = sw;
		rh = sh;
		while (rrw != rw && rrh != rh)
		{
			rw = (int)(rw*0.7);
			rh = (int)(rh*0.7);
			if (rw <= rrw || rh <= rrh)
			{
				rw = rrw;
				rh = rrh;
			}
			q = (pixel*)malloc(rw*rh*PIXELSIZE);
			//Bilinear interpolation for upscaling
			for (int y = 0; y < rh; y++)
				for (int x = 0; x < rw; x++)
				{
					fx = ((float)x)*((float)sw)/((float)rw);
					fy = ((float)y)*((float)sh)/((float)rh);
					fxc = (float)modf(fx, &intp);
					fyc = (float)modf(fy, &intp);
					fxceil = (int)ceil(fx);
					fyceil = (int)ceil(fy);
					if (fxceil>=sw) fxceil = sw-1;
					if (fyceil>=sh) fyceil = sh-1;
					tr = oq[sw*(int)floor(fy)+fxceil];
					tl = oq[sw*(int)floor(fy)+(int)floor(fx)];
					br = oq[sw*fyceil+fxceil];
					bl = oq[sw*fyceil+(int)floor(fx)];
					q[rw*y+x] = PIXRGB(
						(int)(((((float)PIXR(tl))*(1.0f-fxc))+(((float)PIXR(tr))*(fxc)))*(1.0f-fyc) + ((((float)PIXR(bl))*(1.0f-fxc))+(((float)PIXR(br))*(fxc)))*(fyc)),
						(int)(((((float)PIXG(tl))*(1.0f-fxc))+(((float)PIXG(tr))*(fxc)))*(1.0f-fyc) + ((((float)PIXG(bl))*(1.0f-fxc))+(((float)PIXG(br))*(fxc)))*(fyc)),
						(int)(((((float)PIXB(tl))*(1.0f-fxc))+(((float)PIXB(tr))*(fxc)))*(1.0f-fyc) + ((((float)PIXB(bl))*(1.0f-fxc))+(((float)PIXB(br))*(fxc)))*(fyc))
						);
				}
			free(oq);
			oq = q;
			sw = rw;
			sh = rh;
		}
		return q;
	}
}

pixel * rescale_img(pixel *src, int sw, int sh, int *qw, int *qh, int f)
{
	int w = (sw+f-1)/f;
	int h = (sh+f-1)/f;
	pixel *q = (pixel*)malloc(w*h*PIXELSIZE);
	for (int y = 0; y < h; y++)
		for (int x = 0; x < w; x++)
		{
			int r = 0, g = 0, b = 0, c = 0;
			for (int j = 0; j < f; j++)
				for (int i = 0; i < f; i++)
					if (x*f+i<sw && y*f+j<sh)
					{
						pixel p = src[(y*f+j)*sw + (x*f+i)];
						if (p)
						{
							r += PIXR(p);
							g += PIXG(p);
							b += PIXB(p);
							c ++;
						}
					}
			if (c > 1)
			{
				r = (r+c/2)/c;
				g = (g+c/2)/c;
				b = (b+c/2)/c;
			}
			q[y*w+x] = PIXRGB(r, g, b);
		}
	*qw = w;
	*qh = h;
	return q;
}
