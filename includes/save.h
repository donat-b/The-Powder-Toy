/**
 * Powder Toy - saving and loading functions header
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef SAVE_H
#define SAVE_H
#include <simulation/ElementNumbers.h>

//transforms a save when you move it around with the arrow keys, or rotate it
void *transform_save(void *odata, int *size, matrix2d transform, vector2d translate);

//builds a thumb or something? idk
void *build_thumb(int *size, int bzip2);

//calls the correct function to prerender either an OPS or PSV save
pixel *prerender_save(void *save, int size, int *width, int *height);

//calls the correct function to parse either an OPS or PSV save
int parse_save(void *save, int size, int replace, int x0, int y0, unsigned char bmap[YRES/CELL][XRES/CELL], float vx[YRES/CELL][XRES/CELL], float vy[YRES/CELL][XRES/CELL], float pv[YRES/CELL][XRES/CELL], float fvx[YRES/CELL][XRES/CELL], float fvy[YRES/CELL][XRES/CELL], sign signs[MAXSIGNS], void* partsptr, unsigned pmap[YRES][XRES]);

//converts mod elements from older saves into the new correct id's, since as new elements are added to tpt the id's go up
int fix_type(int type, int version, int modver, int (elementPalette)[PT_NUM] = NULL);

//functions to check saves for invalid elements that shouldn't be saved
int invalid_element(int save_as, int el);
int check_save(int save_as, int orig_x0, int orig_y0, int orig_w, int orig_h, int give_warning);

//converts old format and new tpt++ format wall id's into the correct id's for this version
int change_wall(int wt);
int change_wallpp(int wt);

//Current save prerenderer, builder, and parser
pixel *prerender_save_OPS(void *save, int size, int *width, int *height);
void *build_save(int *size, int orig_x0, int orig_y0, int orig_w, int orig_h, unsigned char bmap[YRES/CELL][XRES/CELL], float vx[YRES/CELL][XRES/CELL], float vy[YRES/CELL][XRES/CELL], float pv[YRES/CELL][XRES/CELL], float fvx[YRES/CELL][XRES/CELL], float fvy[YRES/CELL][XRES/CELL], sign signs[MAXSIGNS], void* partsptr, bool tab = false);
int parse_save_OPS(void *save, int size, int replace, int x0, int y0, unsigned char bmap[YRES/CELL][XRES/CELL], float vx[YRES/CELL][XRES/CELL], float vy[YRES/CELL][XRES/CELL], float pv[YRES/CELL][XRES/CELL], float fvx[YRES/CELL][XRES/CELL], float fvy[YRES/CELL][XRES/CELL], sign signs[MAXSIGNS], void* partsptr, unsigned pmap[YRES][XRES]);

//Old save prerenderer and parser
pixel *prerender_save_PSv(void *save, int size, int *width, int *height);
int parse_save_PSv(void *save, int size, int replace, int x0, int y0, unsigned char bmap[YRES/CELL][XRES/CELL], float fvx[YRES/CELL][XRES/CELL], float fvy[YRES/CELL][XRES/CELL], sign signs[MAXSIGNS], void* partsptr, unsigned pmap[YRES][XRES]);

#endif
