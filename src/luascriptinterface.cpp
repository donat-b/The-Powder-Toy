#include <dirent.h>
#include <string>
#ifdef WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif
#include <sys/stat.h>

#include "defines.h"
#include "luascriptinterface.h"
#include "interface.h"
#include "powder.h"
#include "gravity.h"
#include "powdergraphics.h"

#include "game/Menus.h"
#include "simulation/Simulation.h"

/*

SIMULATION API

*/

void initSimulationAPI(lua_State * l)
{
	//Methods
	struct luaL_reg simulationAPIMethods [] = {
		{"partNeighbours", simulation_partNeighbours},
		{"partChangeType", simulation_partChangeType},
		{"partCreate", simulation_partCreate},
		{"partID", simulation_partID},
		{"partProperty", simulation_partProperty},
		{"partPosition", simulation_partPosition},
		{"partKill", simulation_partKill},
		{"pressure", simulation_pressure},
		{"ambientHeat", simulation_ambientHeat},
		{"velocityX", simulation_velocityX},
		{"velocityY", simulation_velocityY},
		{"gravMap", simulation_gravMap},
		{NULL, NULL}
	};
	luaL_register(l, "simulation", simulationAPIMethods);

	//Sim shortcut
	lua_getglobal(l, "simulation");
	lua_setglobal(l, "sim");

	//Static values
	SETCONST(l, XRES);
	SETCONST(l, YRES);
	SETCONST(l, PT_NUM);
	lua_pushinteger(l, 0); lua_setfield(l, -2, "NUM_PARTS");
	SETCONST(l, R_TEMP);
	SETCONST(l, MAX_TEMP);
	SETCONST(l, MIN_TEMP);

}

int simulation_partNeighbours(lua_State * l)
{
	int ids = 0;
	if(lua_gettop(l) == 4)
	{
		int x = lua_tointeger(l, 1), y = lua_tointeger(l, 2), r = lua_tointeger(l, 3), t = lua_tointeger(l, 4), rx, ry, n;
		for (rx = -r; rx <= r; rx++)
			for (ry = -r; ry <= r; ry++)
				if (x+rx >= 0 && y+ry >= 0 && x+rx < XRES && y+ry < YRES && (rx || ry))
				{
					n = pmap[y+ry][x+rx];
					if(n && (n&0xFF) == t)
					{
						ids++;
						lua_pushinteger(l, n>>8);
					}
				}

	}
	else
	{
		int x = lua_tointeger(l, 1), y = lua_tointeger(l, 2), r = lua_tointeger(l, 3), rx, ry, n;
		for (rx = -r; rx <= r; rx++)
			for (ry = -r; ry <= r; ry++)
				if (x+rx >= 0 && y+ry >= 0 && x+rx < XRES && y+ry < YRES && (rx || ry))
				{
					n = pmap[y+ry][x+rx];
					if(n)
					{
						ids++;
						lua_pushinteger(l, n>>8);
					}
				}
	}
	return ids;
}

int simulation_partChangeType(lua_State * l)
{
	int partIndex = lua_tointeger(l, 1), x, y;
	if(partIndex < 0 || partIndex >= NPART || !parts[partIndex].type)
		return 0;
	part_change_type(partIndex, (int)(parts[partIndex].x+0.5f), (int)(parts[partIndex].y+0.5f), lua_tointeger(l, 2));
	return 0;
}

int simulation_partCreate(lua_State * l)
{
	int newID = lua_tointeger(l, 1);
	if(newID >= NPART || newID < -3)
	{
		lua_pushinteger(l, -1);
		return 1;
	}
	lua_pushinteger(l, create_part(newID, lua_tointeger(l, 2), lua_tointeger(l, 3), lua_tointeger(l, 4)));
	return 1;
}

int simulation_partID(lua_State * l)
{
	int x = lua_tointeger(l, 1);
	int y = lua_tointeger(l, 2);
	int amalgam; // "an alloy of mercury with another metal that is solid or liquid at room temperature" What?

	if(x < 0 || x >= XRES || y < 0 || y >= YRES)
	{
		lua_pushnil(l);
		return 1;
	}

	amalgam = pmap[y][x];
	if(!amalgam)
		amalgam = photons[y][x];
	lua_pushinteger(l, amalgam >> 8);
	return 1;
}

int simulation_partPosition(lua_State * l)
{
	int particleID = lua_tointeger(l, 1);
	int argCount = lua_gettop(l);
	if(particleID < 0 || particleID >= NPART || !parts[particleID].type)
	{
		if(argCount == 1)
		{
			lua_pushnil(l);
			lua_pushnil(l);
			return 2;
		} else {
			return 0;
		}
	}
	
	if(argCount == 3)
	{
		parts[particleID].x = lua_tonumber(l, 2);
		parts[particleID].y = lua_tonumber(l, 3);
		return 0;
	}
	else
	{
		lua_pushnumber(l, parts[particleID].x);
		lua_pushnumber(l, parts[particleID].y);
		return 2;
	}
}

int simulation_partProperty(lua_State * l)
{
	return luaL_error(l, "Not implemented, bug jacob1 to put this in");
}

int simulation_partKill(lua_State * l)
{
	if(lua_gettop(l)==2)
		delete_part(lua_tointeger(l, 1), lua_tointeger(l, 2), 0);
	else
		kill_part(lua_tointeger(l, 1));
	return 0;
}

int simulation_pressure(lua_State* l)
{
	int argCount = lua_gettop(l), x, y, width = 1, height = 1;
	float value;
	luaL_checktype(l, 1, LUA_TNUMBER);
	luaL_checktype(l, 2, LUA_TNUMBER);
	x = lua_tointeger(l, 1);
	y = lua_tointeger(l, 2);
	if (x*CELL<0 || y*CELL<0 || x*CELL>=XRES || y*CELL>=YRES)
		return luaL_error(l, "coordinates out of range (%d,%d)", x, y);

	if (argCount == 2)
	{
		lua_pushnumber(l, pv[y][x]);
		return 1;
	}
	luaL_checktype(l, 3, LUA_TNUMBER);
	if (argCount == 3)
		value = (float)lua_tonumber(l, 3);
	else
	{
		luaL_checktype(l, 4, LUA_TNUMBER);
		luaL_checktype(l, 5, LUA_TNUMBER);
		width = lua_tointeger(l, 3);
		height = lua_tointeger(l, 4);
		value = (float)lua_tonumber(l, 5);
	}
	if(value > 256.0f)
		value = 256.0f;
	else if(value < -256.0f)
		value = -256.0f;

	set_map(x, y, width, height, value, 1);
	return 0;
}

int simulation_ambientHeat(lua_State* l)
{
	int argCount = lua_gettop(l), x, y, width = 1, height = 1;
	float value;
	luaL_checktype(l, 1, LUA_TNUMBER);
	luaL_checktype(l, 2, LUA_TNUMBER);
	x = lua_tointeger(l, 1);
	y = lua_tointeger(l, 2);
	if (x*CELL<0 || y*CELL<0 || x*CELL>=XRES || y*CELL>=YRES)
		return luaL_error(l, "coordinates out of range (%d,%d)", x, y);

	if (argCount == 2)
	{
		lua_pushnumber(l, hv[y][x]);
		return 1;
	}
	luaL_checktype(l, 3, LUA_TNUMBER);
	if (argCount == 3)
		value = (float)lua_tonumber(l, 3);
	else
	{
		luaL_checktype(l, 4, LUA_TNUMBER);
		luaL_checktype(l, 5, LUA_TNUMBER);
		width = lua_tointeger(l, 3);
		height = lua_tointeger(l, 4);
		value = (float)lua_tonumber(l, 5);
	}
	if(value > MAX_TEMP)
		value = MAX_TEMP;
	else if(value < MIN_TEMP)
		value = MIN_TEMP;

	set_map(x, y, width, height, value, 2);
	return 0;
}

int simulation_velocityX(lua_State* l)
{
	int argCount = lua_gettop(l), x, y, width = 1, height = 1;
	float value;
	luaL_checktype(l, 1, LUA_TNUMBER);
	luaL_checktype(l, 2, LUA_TNUMBER);
	x = lua_tointeger(l, 1);
	y = lua_tointeger(l, 2);
	if (x*CELL<0 || y*CELL<0 || x*CELL>=XRES || y*CELL>=YRES)
		return luaL_error(l, "coordinates out of range (%d,%d)", x, y);

	if (argCount == 2)
	{
		lua_pushnumber(l, vx[y][x]);
		return 1;
	}
	luaL_checktype(l, 3, LUA_TNUMBER);
	if (argCount == 3)
		value = (float)lua_tonumber(l, 3);
	else
	{
		luaL_checktype(l, 4, LUA_TNUMBER);
		luaL_checktype(l, 5, LUA_TNUMBER);
		width = lua_tointeger(l, 3);
		height = lua_tointeger(l, 4);
		value = (float)lua_tonumber(l, 5);
	}
	if(value > 256.0f)
		value = 256.0f;
	else if(value < -256.0f)
		value = -256.0f;

	set_map(x, y, width, height, value, 3);
	return 0;
}

int simulation_velocityY(lua_State* l)
{
	int argCount = lua_gettop(l), x, y, width = 1, height = 1;
	float value;
	luaL_checktype(l, 1, LUA_TNUMBER);
	luaL_checktype(l, 2, LUA_TNUMBER);
	x = lua_tointeger(l, 1);
	y = lua_tointeger(l, 2);
	if (x*CELL<0 || y*CELL<0 || x*CELL>=XRES || y*CELL>=YRES)
		return luaL_error(l, "coordinates out of range (%d,%d)", x, y);

	if (argCount == 2)
	{
		lua_pushnumber(l, vy[y][x]);
		return 1;
	}
	luaL_checktype(l, 3, LUA_TNUMBER);
	if (argCount == 3)
		value = (float)lua_tonumber(l, 3);
	else
	{
		luaL_checktype(l, 4, LUA_TNUMBER);
		luaL_checktype(l, 5, LUA_TNUMBER);
		width = lua_tointeger(l, 3);
		height = lua_tointeger(l, 4);
		value = (float)lua_tonumber(l, 5);
	}
	if(value > 256.0f)
		value = 256.0f;
	else if(value < -256.0f)
		value = -256.0f;

	set_map(x, y, width, height, value, 4);
	return 0;
}

int simulation_gravMap(lua_State* l)
{
	int argCount = lua_gettop(l), x, y, width = 1, height = 1;
	float value;
	luaL_checktype(l, 1, LUA_TNUMBER);
	luaL_checktype(l, 2, LUA_TNUMBER);
	x = lua_tointeger(l, 1);
	y = lua_tointeger(l, 2);
	if (x*CELL<0 || y*CELL<0 || x*CELL>=XRES || y*CELL>=YRES)
		return luaL_error(l, "coordinates out of range (%d,%d)", x, y);

	/*if (argCount == 2)
	{
		lua_pushnumber(l, gravmap[y*XRES/CELL+x]);
		return 1;
	}*/
	luaL_checktype(l, 3, LUA_TNUMBER);
	if (argCount == 3)
		value = (float)lua_tonumber(l, 3);
	else
	{
		luaL_checktype(l, 4, LUA_TNUMBER);
		luaL_checktype(l, 5, LUA_TNUMBER);
		width = lua_tointeger(l, 3);
		height = lua_tointeger(l, 4);
		value = (float)lua_tonumber(l, 5);
	}

	set_map(x, y, width, height, value, 5);
	return 0;
}

/*

RENDERER API

*/

void initRendererAPI(lua_State * l)
{
	//Methods
	struct luaL_reg rendererAPIMethods [] = {
		{"renderModes", renderer_renderModes},
		{"displayModes", renderer_displayModes},
		{"colourMode", renderer_colourMode},
		{"colorMode", renderer_colourMode}, //Duplicate of above to make Americans happy
		{"decorations", renderer_decorations},
		{NULL, NULL}
	};
	luaL_register(l, "renderer", rendererAPIMethods);

	//Ren shortcut
	lua_getglobal(l, "renderer");
	lua_setglobal(l, "ren");

	//Static values
	//Particle pixel modes/fire mode/effects
	SETCONST(l, PMODE);
	SETCONST(l, PMODE_NONE);
	SETCONST(l, PMODE_FLAT);
	SETCONST(l, PMODE_BLOB);
	SETCONST(l, PMODE_BLUR);
	SETCONST(l, PMODE_GLOW);
	SETCONST(l, PMODE_SPARK);
	SETCONST(l, PMODE_FLARE);
	SETCONST(l, PMODE_LFLARE);
	SETCONST(l, PMODE_ADD);
	SETCONST(l, PMODE_BLEND);
	SETCONST(l, PSPEC_STICKMAN);
	SETCONST(l, OPTIONS);
	SETCONST(l, NO_DECO);
	SETCONST(l, DECO_FIRE);
	SETCONST(l, FIREMODE);
	SETCONST(l, FIRE_ADD);
	SETCONST(l, FIRE_BLEND);
	SETCONST(l, EFFECT);
	SETCONST(l, EFFECT_GRAVIN);
	SETCONST(l, EFFECT_GRAVOUT);
	SETCONST(l, EFFECT_LINES);
	SETCONST(l, EFFECT_DBGLINES);

	//Display/Render/Colour modes
	SETCONST(l, RENDER_EFFE);
	SETCONST(l, RENDER_FIRE);
	SETCONST(l, RENDER_GLOW);
	SETCONST(l, RENDER_BLUR);
	SETCONST(l, RENDER_BLOB);
	SETCONST(l, RENDER_BASC);
	SETCONST(l, RENDER_NONE);
	SETCONST(l, COLOUR_HEAT);
	SETCONST(l, COLOUR_LIFE);
	SETCONST(l, COLOUR_GRAD);
	SETCONST(l, COLOUR_BASC);
	SETCONST(l, COLOUR_DEFAULT);
	SETCONST(l, DISPLAY_AIRC);
	SETCONST(l, DISPLAY_AIRP);
	SETCONST(l, DISPLAY_AIRV);
	SETCONST(l, DISPLAY_AIRH);
	SETCONST(l, DISPLAY_AIR);
	SETCONST(l, DISPLAY_WARP);
	SETCONST(l, DISPLAY_PERS);
	SETCONST(l, DISPLAY_EFFE);
}

//get/set render modes list
int renderer_renderModes(lua_State * l)
{
	int args = lua_gettop(l);
	if(args)
	{
		int size = 0, i;
		luaL_checktype(l, 1, LUA_TTABLE);
		size = luaL_getn(l, 1);
		
		free(render_modes);
		render_modes = (unsigned int*)calloc(size + 1, sizeof(unsigned int));
		for(i = 1; i <= size; i++)
		{
			lua_rawgeti(l, 1, i);
			luaL_checktype(l, -1, LUA_TNUMBER);
			error_ui(vid_buf, lua_tointeger(l, -1), "asdf");
			render_modes[i-1] = lua_tointeger(l, -1);
			lua_pop(l, 1);
		}
		render_modes[size] = 0;
		update_display_modes();
		return 0;
	}
	else
	{
		int i = 1;
		lua_newtable(l);
		while(render_modes[i-1])
		{
			lua_pushinteger(l, render_modes[i-1]);
			lua_rawseti(l, -2, i++);
		}
		return 1;
	}
}

int renderer_displayModes(lua_State * l)
{
	int args = lua_gettop(l);
	if(args)
	{
		int size = 0, i;
		luaL_checktype(l, 1, LUA_TTABLE);
		size = luaL_getn(l, 1);
		
		free(display_modes);
		display_modes = (unsigned int*)calloc(size + 1, sizeof(unsigned int));
		for(i = 1; i <= size; i++)
		{
			lua_rawgeti(l, 1, i);
			display_modes[i-1] = lua_tointeger(l, -1);
			lua_pop(l, 1);
		}
		display_modes[size] = 0;
		update_display_modes();
		return 0;
	}
	else
	{
		int i = 1;
		lua_newtable(l);
		while(display_modes[i-1])
		{
			lua_pushinteger(l, display_modes[i-1]);
			lua_rawseti(l, -2, i++);
		}
		return 1;
	}
}

int renderer_colourMode(lua_State * l)
{
	int args = lua_gettop(l);
	if(args)
	{
		luaL_checktype(l, 1, LUA_TNUMBER);
		colour_mode = lua_tointeger(l, 1);
		return 0;
	}
	else
	{
		lua_pushinteger(l, colour_mode);
		return 1;
	}
}

int renderer_decorations(lua_State * l)
{
	int args = lua_gettop(l);
	if(args)
	{
		decorations_enable = lua_toboolean(l, 1);
		return 0;
	}
	else
	{
		lua_pushboolean(l, decorations_enable);
		return 1;
	}
}

/*

FILESYSTEM API

*/

void initFileSystemAPI(lua_State * l)
{
	//Methods
	struct luaL_reg fileSystemAPIMethods [] = {
		{"list", fileSystem_list},
		{"exists", fileSystem_exists},
		{"isFile", fileSystem_isFile},
		{"isDirectory", fileSystem_isDirectory},
		{"makeDirectory", fileSystem_makeDirectory},
		{"removeDirectory", fileSystem_removeDirectory},
		{"removeFile", fileSystem_removeFile},
		{"move", fileSystem_move},
		{"copy", fileSystem_copy},
		{NULL, NULL}
	};
	luaL_register(l, "fileSystem", fileSystemAPIMethods);

	//elem shortcut
	lua_getglobal(l, "fileSystem");
	lua_setglobal(l, "fs");
}

int fileSystem_list(lua_State * l)
{
	const char * directoryName = lua_tostring(l, 1);
	DIR * directory;
	struct dirent * entry;

	int index = 1;
	lua_newtable(l);

	directory = opendir(directoryName);
	if (directory != NULL)
	{
		while (entry = readdir(directory))
		{
			if(strncmp(entry->d_name, "..", 3) && strncmp(entry->d_name, ".", 2))
			{
				lua_pushstring(l, entry->d_name);
				lua_rawseti(l, -2, index++);
			}
		}
		closedir(directory);
	}
	else
	{
		lua_pushnil(l);
	}

	return 1;
}

int fileSystem_exists(lua_State * l)
{
	char * filename = (char*)lua_tostring(l, 1);

	lua_pushboolean(l, file_exists(filename));
	return 1;
}

int fileSystem_isFile(lua_State * l)
{
	const char * filename = lua_tostring(l, 1);

	int isFile = 0;
#ifdef WIN
	struct _stat s;
	if(_stat(filename, &s) == 0)
#else
	struct stat s;
	if(stat(filename, &s) == 0)
#endif
	{
		if(s.st_mode & S_IFREG)
		{
			isFile = 1; //Is file
		}
		else
		{
			isFile = 0; //Is directory or something else
		}
	}
	else
	{
		isFile = 0; //Doesn't exist
	}

	lua_pushboolean(l, isFile);
	return 1;
}

int fileSystem_isDirectory(lua_State * l)
{
	const char * filename = lua_tostring(l, 1);

	int isDir = 0;
#ifdef WIN
	struct _stat s;
	if(_stat(filename, &s) == 0)
#else
	struct stat s;
	if(stat(filename, &s) == 0)
#endif
	{
		if(s.st_mode & S_IFDIR)
		{
			isDir = 1; //Is directory
		}
		else
		{
			isDir = 0; //Is file or something else
		}
	}
	else
	{
		isDir = 0; //Doesn't exist
	}

	lua_pushboolean(l, isDir);
	return 1;
}

int fileSystem_makeDirectory(lua_State * l)
{
	const char * dirname = lua_tostring(l, 1);

	int ret = 0;
#ifdef WIN32
	ret = _mkdir(dirname);
#else
	ret = mkdir(dirname, 0755);
#endif
	lua_pushboolean(l, ret == 0);
	return 1;
}

int fileSystem_removeDirectory(lua_State * l)
{
	const char * filename = lua_tostring(l, 1);

	int ret = 0;
#ifdef WIN32
	ret = _rmdir(filename);
#else
	ret = rmdir(filename);
#endif
	lua_pushboolean(l, ret == 0);
	return 1;
}

int fileSystem_removeFile(lua_State * l)
{
	const char * filename = lua_tostring(l, 1);

	int ret = 0;
#ifdef WIN32
	ret = _unlink(filename);
#else
	ret = unlink(filename);
#endif
	lua_pushboolean(l, ret == 0);
	return 1;
}

int fileSystem_move(lua_State * l)
{
	const char * filename = lua_tostring(l, 1);
	const char * newFilename = lua_tostring(l, 2);
	int ret = 0;

	ret = rename(filename, newFilename);

	lua_pushboolean(l, ret == 0);
	return 1;
}

int fileSystem_copy(lua_State * l)
{
	const char * filename = lua_tostring(l, 1);
	const char * newFilename = lua_tostring(l, 2);
	int ret = 1;

	char buf[BUFSIZ];
	size_t size;

	FILE* source = fopen(filename, "rb");
	if(source)
	{
		FILE* dest = fopen(newFilename, "wb");
		if (dest)
		{
			while (size = fread(buf, 1, BUFSIZ, source)) {
				fwrite(buf, 1, size, dest);
			}

			fclose(dest);
			ret = 0;
		}
		fclose(source);
	}

	lua_pushboolean(l, ret == 0);
	return 1;
}

/*

GRAPHICS API

*/

void initGraphicsAPI(lua_State * l)
{
	//Methods
	struct luaL_reg graphicsAPIMethods [] = {
		{"textSize", graphics_textSize},
		{"drawText", graphics_drawText},
		{"drawLine", graphics_drawLine},
		{"drawRect", graphics_drawRect},
		{"fillRect", graphics_fillRect},
		{NULL, NULL}
	};
	luaL_register(l, "graphics", graphicsAPIMethods);

	//elem shortcut
	lua_getglobal(l, "graphics");
	lua_setglobal(l, "gfx");

	lua_pushinteger(l, XRES+BARSIZE);	lua_setfield(l, -2, "WIDTH");
	lua_pushinteger(l, YRES+MENUSIZE);	lua_setfield(l, -2, "HEIGHT");
}

int graphics_textSize(lua_State * l)
{
    char * text;
    int width, height;
	text = (char*)lua_tostring(l, 1);
	textsize(text, &width, &height);

	lua_pushinteger(l, width);
	lua_pushinteger(l, height);
	return 2;
}

int graphics_drawText(lua_State * l)
{
	char * text;
	int x, y, r, g, b, a;
	x = lua_tointeger(l, 1);
	y = lua_tointeger(l, 2);
	text = (char*)lua_tostring(l, 3);
	r = luaL_optint(l, 4, 255);
	g = luaL_optint(l, 5, 255);
	b = luaL_optint(l, 6, 255);
	a = luaL_optint(l, 7, 255);
	
	if (r<0) r = 0;
	if (r>255) r = 255;
	if (g<0) g = 0;
	if (g>255) g = 255;
	if (b<0) b = 0;
	if (b>255) b = 255;
	if (a<0) a = 0;
	if (a>255) a = 255;

	drawtext(vid_buf, x, y, text, r, g, b, a);
	return 0;
}

int graphics_drawLine(lua_State * l)
{
	int x1, y1, x2, y2, r, g, b, a;
	x1 = lua_tointeger(l, 1);
	y1 = lua_tointeger(l, 2);
	x2 = lua_tointeger(l, 3);
	y2 = lua_tointeger(l, 4);
	r = luaL_optint(l, 5, 255);
	g = luaL_optint(l, 6, 255);
	b = luaL_optint(l, 7, 255);
	a = luaL_optint(l, 8, 255);

	if (r<0) r = 0;
	if (r>255) r = 255;
	if (g<0) g = 0;
	if (g>255) g = 255;
	if (b<0) b = 0;
	if (b>255) b = 255;
	if (a<0) a = 0;
	if (a>255) a = 255;
	blend_line(vid_buf, x1, y1, x2, y2, r, g, b, a);
	return 0;
}

int graphics_drawRect(lua_State * l)
{
	int x, y, w, h, r, g, b, a;
	x = lua_tointeger(l, 1);
	y = lua_tointeger(l, 2);
	w = lua_tointeger(l, 3)-1;
	h = lua_tointeger(l, 4)-1;
	r = luaL_optint(l, 5, 255);
	g = luaL_optint(l, 6, 255);
	b = luaL_optint(l, 7, 255);
	a = luaL_optint(l, 8, 255);

	if (r<0) r = 0;
	if (r>255) r = 255;
	if (g<0) g = 0;
	if (g>255) g = 255;
	if (b<0) b = 0;
	if (b>255) b = 255;
	if (a<0) a = 0;
	if (a>255) a = 255;
	drawrect(vid_buf, x, y, w, h, r, g, b, a);
	return 0;
}

int graphics_fillRect(lua_State * l)
{
	int x, y, w, h, r, g, b, a;
	x = lua_tointeger(l, 1)-1;
	y = lua_tointeger(l, 2)-1;
	w = lua_tointeger(l, 3)+1;
	h = lua_tointeger(l, 4)+1;
	r = luaL_optint(l, 5, 255);
	g = luaL_optint(l, 6, 255);
	b = luaL_optint(l, 7, 255);
	a = luaL_optint(l, 8, 255);

	if (r<0) r = 0;
	if (r>255) r = 255;
	if (g<0) g = 0;
	if (g>255) g = 255;
	if (b<0) b = 0;
	if (b>255) b = 255;
	if (a<0) a = 0;
	if (a>255) a = 255;
	fillrect(vid_buf, x, y, w, h, r, g, b, a);
	return 0;
}

/*

ELEMENTS API

*/

void initElementsAPI(lua_State * l)
{
	int i;
	//Methods
	struct luaL_reg elementsAPIMethods [] = {
		{"allocate", elements_allocate},
		{"element", elements_element},
		{"property", elements_property},
		{"free", elements_free},
		{"loadDefault", elements_loadDefault},
		{NULL, NULL}
	};
	luaL_register(l, "elements", elementsAPIMethods);

	//elem shortcut
	lua_getglobal(l, "elements");
	lua_setglobal(l, "elem");

	//Static values
	//Element types/properties/states
	SETCONST(l, TYPE_PART);
	SETCONST(l, TYPE_LIQUID);
	SETCONST(l, TYPE_SOLID);
	SETCONST(l, TYPE_GAS);
	SETCONST(l, TYPE_ENERGY);
	SETCONST(l, PROP_CONDUCTS);
	SETCONST(l, PROP_BLACK);
	SETCONST(l, PROP_NEUTPENETRATE);
	SETCONST(l, PROP_NEUTABSORB);
	SETCONST(l, PROP_NEUTPASS);
	SETCONST(l, PROP_DEADLY);
	SETCONST(l, PROP_HOT_GLOW);
	SETCONST(l, PROP_LIFE);
	SETCONST(l, PROP_RADIOACTIVE);
	SETCONST(l, PROP_LIFE_DEC);
	SETCONST(l, PROP_LIFE_KILL);
	SETCONST(l, PROP_INDESTRUCTIBLE);
	SETCONST(l, PROP_CLONE);
	SETCONST(l, PROP_BREAKABLECLONE);
	SETCONST(l, PROP_POWERED);
	SETCONST(l, PROP_SPARKSETTLE);
	SETCONST(l, PROP_NOAMBHEAT);
	SETCONST(l, PROP_MOVS);
	SETCONST(l, PROP_DRAWONCTYPE);
	SETCONST(l, PROP_NOCTYPEDRAW);
	SETCONST(l, FLAG_STAGNANT);
	SETCONST(l, FLAG_SKIPMOVE);
	lua_pushinteger(l, 0); lua_setfield(l, -2, "FLAG_MOVABLE"); //removed this constant, sponge moves again and no reason for other elements to be allowed to
	SETCONST(l, FLAG_EXPLODE);
	SETCONST(l, FLAG_INSTACTV);
	SETCONST(l, FLAG_WATEREQUAL);
	SETCONST(l, ST_NONE);
	SETCONST(l, ST_SOLID);
	SETCONST(l, ST_LIQUID);
	SETCONST(l, ST_GAS);

	SETCONST(l, SC_WALL);
	SETCONST(l, SC_ELEC);
	SETCONST(l, SC_POWERED);
	SETCONST(l, SC_SENSOR);
	SETCONST(l, SC_FORCE);
	SETCONST(l, SC_EXPLOSIVE);
	SETCONST(l, SC_GAS);
	SETCONST(l, SC_LIQUID);
	SETCONST(l, SC_POWDERS);
	SETCONST(l, SC_SOLIDS);
	SETCONST(l, SC_NUCLEAR);
	SETCONST(l, SC_SPECIAL);
	SETCONST(l, SC_LIFE);
	SETCONST(l, SC_TOOL);
	SETCONST(l, SC_DECO);
	SETCONST(l, SC_FAV);
	SETCONST(l, SC_FAV2);
	SETCONST(l, SC_HUD);
	SETCONST(l, SC_CRACKER);

	//Element identifiers
	for(i = 0; i < PT_NUM; i++)
	{
		if(globalSim->elements[i].Enabled)
		{
			char realidentifier[24];
			lua_pushinteger(l, i);
			lua_setfield(l, -2, globalSim->elements[i].Identifier.c_str());
			sprintf(realidentifier,"DEFAULT_PT_%s",ptypes[i].name);
			if (i != 0 && i != PT_NBHL && i != PT_NWHL && strcmp(globalSim->elements[i].Identifier.c_str(), realidentifier))
			{
				lua_pushinteger(l, i);
				lua_setfield(l, -2, realidentifier);
			}
		}
	}
}

int elements_getProperty(char * key, int * format, unsigned int * modifiedStuff)
{
	int offset;
	if (!strcmp(key, "Name"))
	{
		offset = offsetof(Element, Name);
		*format = 2;
	}
	else if (!strcmp(key, "Color") || !strcmp(key, "Colour"))
	{
		offset = offsetof(Element, Colour);
		*format = 4;
		*modifiedStuff |= LUACON_EL_MODIFIED_GRAPHICS;
	}
	else if (!strcmp(key, "Advection"))
	{
		offset = offsetof(Element, Advection);
		*format = 1;
	}
	else if (!strcmp(key, "AirDrag"))
	{
		offset = offsetof(Element, AirDrag);
		*format = 1;
	}
	else if (!strcmp(key, "AirLoss"))
	{
		offset = offsetof(Element, AirLoss);
		*format = 1;
	}
	else if (!strcmp(key, "Loss"))
	{
		offset = offsetof(Element, Loss);
		*format = 1;
	}
	else if (!strcmp(key, "Collision"))
	{
		offset = offsetof(Element, Collision);
		*format = 1;
	}
	else if (!strcmp(key, "Gravity"))
	{
		offset = offsetof(Element, Gravity);
		*format = 1;
	}
	else if (!strcmp(key, "Diffusion"))
	{
		offset = offsetof(Element, Diffusion);
		*format = 1;
	}
	else if (!strcmp(key, "HotAir"))
	{
		offset = offsetof(Element, PressureAdd_NoAmbHeat);
		*format = 1;
	}
	else if (!strcmp(key, "Falldown"))
	{
		offset = offsetof(Element, Falldown);
		*format = 0;
	}
	else if (!strcmp(key, "Flammable"))
	{
		offset = offsetof(Element, Flammable);
		*format = 0;
	}
	else if (!strcmp(key, "Explosive"))
	{
		offset = offsetof(Element, Explosive);
		*format = 0;
	}
	else if (!strcmp(key, "Meltable"))
	{
		offset = offsetof(Element, Meltable);
		*format = 0;
	}
	else if (!strcmp(key, "Hardness"))
	{
		offset = offsetof(Element, Hardness);
		*format = 0;
	}
	else if (!strcmp(key, "PhotonReflectWavelengths"))
	{
		offset = offsetof(Element, PhotonReflectWavelengths);
		*format = 5;
	}
	else if (!strcmp(key, "MenuVisible"))
	{
		offset = offsetof(Element, MenuVisible);
		*format = 0;
		*modifiedStuff |= LUACON_EL_MODIFIED_MENUS;
	}
	else if (!strcmp(key, "MenuSection"))
	{
		offset = offsetof(Element, MenuSection);
		*format = 0;
		*modifiedStuff |= LUACON_EL_MODIFIED_MENUS;
	}
	else if (!strcmp(key, "Enabled"))
	{
		offset = offsetof(Element, Enabled);
		*format = 0;
		*modifiedStuff |= LUACON_EL_MODIFIED_MENUS;
	}
	else if (!strcmp(key, "Weight"))
	{
		offset = offsetof(Element, Weight);
		*format = 0;
		*modifiedStuff |= LUACON_EL_MODIFIED_CANMOVE;
	}
	else if (!strcmp(key, "Temperature"))
	{
		offset = offsetof(Element, DefaultProperties.temp);
		*format = 1;
	}
	else if (!strcmp(key, "HeatConduct"))
	{
		offset = offsetof(Element, HeatConduct);
		*format = 3;
	}
	else if (!strcmp(key, "Latent"))
	{
		offset = offsetof(Element, Latent);
		*format = 5;
	}
	else if (!strcmp(key, "State"))
	{
		offset = offsetof(Element, State);
		*format = 6;
	}
	else if (!strcmp(key, "Properties"))
	{
		offset = offsetof(Element, Properties);
		*format = 5;
		*modifiedStuff |= LUACON_EL_MODIFIED_GRAPHICS | LUACON_EL_MODIFIED_CANMOVE;
	}
	else if (!strcmp(key, "Description"))
	{
		offset = offsetof(Element, Description);
		*format = 2;
	}
	else if (!strcmp(key, "LowPressure"))
	{
		offset = offsetof(Element, LowPressureTransitionThreshold);
		*format = 1;
	}
	else if (!strcmp(key, "LowPressureTransition"))
	{
		offset = offsetof(Element, LowPressureTransitionElement);
		*format = 0;
	}
	else if (!strcmp(key, "HighPressure"))
	{
		offset = offsetof(Element, HighPressureTransitionThreshold);
		*format = 1;
	}
	else if (!strcmp(key, "HighPressureTransition"))
	{
		offset = offsetof(Element, HighPressureTransitionElement);
		*format = 0;
	}
	else if (!strcmp(key, "LowTemperature"))
	{
		offset = offsetof(Element, LowTemperatureTransitionThreshold);
		*format = 1;
	}
	else if (!strcmp(key, "LowTemperatureTransition"))
	{
		offset = offsetof(Element, LowTemperatureTransitionElement);
		*format = 0;
	}
	else if (!strcmp(key, "HighTemperature"))
	{
		offset = offsetof(Element, HighTemperatureTransitionThreshold);
		*format = 1;
	}
	else if (!strcmp(key, "HighTemperatureTransition"))
	{
		offset = offsetof(Element, HighTemperatureTransitionElement);
		*format = 0;
	}
	else
	{
		return -1;
	}
	return offset;
}

void elements_setProperty(lua_State * l, int id, int format, int offset)
{
	switch(format)
	{
		case 0: //Int
			*((int*)(((unsigned char*)&globalSim->elements[id])+offset)) = lua_tointeger(l, 3);
			break;
		case 1: //Float
			*((float*)(((unsigned char*)&globalSim->elements[id])+offset)) = lua_tonumber(l, 3);
			break;
		case 2: //String
			*((std::string*)(((unsigned char*)&globalSim->elements[id])+offset)) = std::string(lua_tostring(l, 3));
			break;
		case 3: //Unsigned char (HeatConduct)
			*((unsigned char*)(((unsigned char*)&globalSim->elements[id])+offset)) = lua_tointeger(l, 3);
			break;
		case 4: //Color (Color)
		{
#if PIXELSIZE == 4
			unsigned int col = (unsigned int)lua_tonumber(l, 3);
			*((unsigned int*)(((unsigned char*)&globalSim->elements[id])+offset)) = col;
#else
			*((unsigned short*)(((unsigned char*)&globalSim->elements[id])+offset)) = lua_tointeger(l, 3);
#endif
			break;
		}
		case 5: //Unsigned int (Properties, PhotonReflectWavelength, Latent)
			*((unsigned int*)(((unsigned char*)&globalSim->elements[id])+offset)) = lua_tointeger(l, 3);
			break;
		case 6: //Char (State)
			*((char*)(((unsigned char*)&globalSim->elements[id])+offset)) = lua_tointeger(l, 3);
			break;
	}
}

void elements_writeProperty(lua_State *l, int id, int format, int offset)
{
	switch(format)
	{
		case 0: //Int
			lua_pushinteger(l, *((int*)(((unsigned char*)&globalSim->elements[id])+offset)));
			break;
		case 1: //Float
			lua_pushnumber(l, *((float*)(((unsigned char*)&globalSim->elements[id])+offset)));
			break;
		case 2: //String
			lua_pushstring(l, (*((std::string*)(((unsigned char*)&globalSim->elements[id])+offset))).c_str());
			break;
		case 3: //Unsigned char (HeatConduct)
			lua_pushinteger(l, *((unsigned char*)(((unsigned char*)&globalSim->elements[id])+offset)));
			break;
		case 4: //Color (Color)
#if PIXELSIZE == 4
			lua_pushinteger(l, *((unsigned int*)(((unsigned char*)&globalSim->elements[id])+offset)));
#else
			lua_pushinteger(l, *((unsigned short*)(((unsigned char*)&globalSim->elements[id])+offset)));
#endif
			break;
		case 5: //Unsigned int (Properties, PhotonReflectWavelengths, Latent)
			lua_pushinteger(l, *((unsigned int*)(((unsigned char*)&globalSim->elements[id])+offset)));
			break;
		case 6: //Char (State)
			lua_pushinteger(l, *((char*)(((unsigned char*)&globalSim->elements[id])+offset)));
			break;
		default:
			lua_pushnil(l);
	}
}

int elements_loadDefault(lua_State * l)
{
	int args = lua_gettop(l);
	if(args)
	{
		int id;
		luaL_checktype(l, 1, LUA_TNUMBER);
		id = lua_tointeger(l, 1);
		if(id < 0 || id >= PT_NUM)
			return luaL_error(l, "Invalid element");

		lua_getglobal(l, "elements");
		lua_pushnil(l);
		lua_setfield(l, -2, globalSim->elements[id].Identifier.c_str());

		if(id < PT_NUM)
		{
			if (globalSim->elements[id].Init)
				globalSim->elements[id].Init(globalSim, &globalSim->elements[id], id);
			else
				globalSim->elements[id] = Element();
			Simulation_Compat_CopyData(globalSim);
		}

		lua_pushinteger(l, id);
		lua_setfield(l, -2, globalSim->elements[id].Identifier.c_str());
		lua_pop(l, 1);
	}
	else
	{
		for (int i = 0; i < PT_NUM; i++)
			if (globalSim->elements[i].Init)
				globalSim->elements[i].Init(globalSim, &globalSim->elements[i], i);
		Simulation_Compat_CopyData(globalSim);
		lua_pushnil(l);
		lua_setglobal(l, "elements");
		lua_pushnil(l);
		lua_setglobal(l, "elem");

		lua_getglobal(l, "package");
		lua_getfield(l, -1, "loaded");
		lua_pushnil(l);
		lua_setfield(l, -2, "elements");

		initElementsAPI(l);
	}

	menu_count();
	init_can_move();
	memset(graphicscache, 0, sizeof(gcache_item)*PT_NUM);
	return 0;
}

int elements_allocate(lua_State * l)
{
	char *group, *id, *identifier, *tmp;
	int i, newID = -1;
	luaL_checktype(l, 1, LUA_TSTRING);
	luaL_checktype(l, 2, LUA_TSTRING);
	group = (char*)lua_tostring(l, 1);
	tmp = group;
	while (*tmp) { *tmp = toupper(*tmp); tmp++; }
	id = (char*)lua_tostring(l, 2);
	tmp = id;
	while (*tmp) { *tmp = toupper(*tmp); tmp++; }

	if(!strcmp(group,"DEFAULT"))
		return luaL_error(l, "You cannot create elements in the 'default' group.");

	identifier = (char*)calloc(strlen(group)+strlen(id)+5,sizeof(char));
	sprintf(identifier,"%s_PT_%s",group,id);

	for(i = 0; i < PT_NUM; i++)
	{
		if(globalSim->elements[i].Enabled && !strcmp(globalSim->elements[i].Identifier.c_str(), identifier))
			return luaL_error(l, "Element identifier already in use");
	}

	for(i = PT_NUM-1; i >= 0; i--)
	{
		if(!globalSim->elements[i].Enabled)
		{
			newID = i;
			globalSim->elements[i] = Element();
			globalSim->elements[i].Enabled = 1;
			globalSim->elements[i].Identifier = identifier;
			Simulation_Compat_CopyData(globalSim);
			break;
		}
	}

	if(newID != -1)
	{
		lua_getglobal(l, "elements");
		lua_pushinteger(l, newID);
		lua_setfield(l, -2, globalSim->elements[i].Identifier.c_str());
		lua_pop(l, 1);
	}

	lua_pushinteger(l, newID);
	return 1;
}

int elements_element(lua_State * l)
{
	int args = lua_gettop(l), id, i = 0;
	unsigned int modifiedStuff = 0;
	char *propertyList[] = { "Name", "Colour", "Color", "MenuVisible", "MenuSection", "Advection", "AirDrag", "AirLoss", "Loss", "Collision", "Gravity", "Diffusion", "HotAir", "Falldown", "Flammable", "Explosive", "Meltable", "Hardness", "PhotonReflectWavelengths", "Weight", "Temperature", "HeatConduct", "Latent", "Description", "State", "Properties", "LowPressure", "LowPressureTransition", "HighPressure", "HighPressureTransition", "LowTemperature", "LowTemperatureTransition", "HighTemperature", "HighTemperatureTransition", NULL};
	luaL_checktype(l, 1, LUA_TNUMBER);
	id = lua_tointeger(l, 1);

	if(id < 0 || id >= PT_NUM || !globalSim->elements[id].Enabled)
		return luaL_error(l, "Invalid element");

	if(args > 1)
	{
		luaL_checktype(l, 2, LUA_TTABLE);
		//Write values from native data to a table
		while (propertyList[i])
		{
			lua_getfield(l, -1, propertyList[i]);
			if(lua_type(l, -1) != LUA_TNIL)
			{
				int format;
				int offset = elements_getProperty(propertyList[i], &format, &modifiedStuff);
				elements_setProperty(l, id, format, offset);
				lua_pop(l, 1);
			}
			i++;
		}

		lua_getfield(l, -1, "Update");
		if(lua_type(l, -1) == LUA_TFUNCTION)
		{
			lua_el_func[id] = luaL_ref(l, LUA_REGISTRYINDEX);
			lua_el_mode[id] = 1;
		}
		else if(lua_type(l, -1) == LUA_TBOOLEAN && !lua_toboolean(l, -1))
		{
			lua_el_func[id] = 0;
			lua_el_mode[id] = 0;
			ptypes[id].update_func = NULL;
		}
		else
			lua_pop(l, 1);

		lua_getfield(l, -1, "Graphics");
		if(lua_type(l, -1) == LUA_TFUNCTION)
		{
			lua_gr_func[id] = luaL_ref(l, LUA_REGISTRYINDEX);
		}
		else if(lua_type(l, -1) == LUA_TBOOLEAN && !lua_toboolean(l, -1))
		{
			lua_gr_func[id] = 0;
			ptypes[id].graphics_func = NULL;
		}
		else
			lua_pop(l, 1);

		Simulation_Compat_CopyData(globalSim);
		menu_count();
		init_can_move();
		graphicscache[id].isready = 0;

		lua_pop(l, 1);
		return 0;
	}
	else
	{
		//Write values from native data to a table
		lua_newtable(l);
		while (propertyList[i])
		{
			int format;
			int offset = elements_getProperty(propertyList[i], &format, &modifiedStuff);
			elements_writeProperty(l, id, format, offset);
			lua_setfield(l, -2, propertyList[i]);
			i++;
		}
		return 1;
	}
	return 0;
}

int elements_property(lua_State * l)
{
	int args = lua_gettop(l), id;
	unsigned int modifiedStuff = 0;
	char *propertyName;
	luaL_checktype(l, 1, LUA_TNUMBER);
	id = lua_tointeger(l, 1);
	luaL_checktype(l, 2, LUA_TSTRING);
	propertyName = (char*)lua_tostring(l, 2);

	if(id < 0 || id >= PT_NUM || !globalSim->elements[id].Enabled)
		return luaL_error(l, "Invalid element");

	if(args > 2)
	{
		int format;
		int offset = elements_getProperty(propertyName, &format, &modifiedStuff);

		if(offset != -1)
		{
			if(lua_type(l, 3) != LUA_TNIL)
			{
				elements_setProperty(l, id, format, offset);
			}

			if (modifiedStuff)
			{
				if (modifiedStuff & LUACON_EL_MODIFIED_MENUS)
					menu_count();
				if (modifiedStuff & LUACON_EL_MODIFIED_CANMOVE)
					init_can_move();
				if (modifiedStuff & LUACON_EL_MODIFIED_GRAPHICS)
					graphicscache[id].isready = 0;
			}

			Simulation_Compat_CopyData(globalSim);
			return 0;
		}
		else if(!strcmp(propertyName,"Update"))
		{
			if(lua_type(l, 3) == LUA_TFUNCTION)
			{
				lua_pushvalue(l, 3);
				lua_el_func[id] = luaL_ref(l, LUA_REGISTRYINDEX);
				if (args > 3)
				{
					int replace;
					luaL_checktype(l, 4, LUA_TNUMBER);
					replace = lua_tointeger(l, 4);
					if (replace == 1)
						lua_el_mode[id] = 2;
					else
						lua_el_mode[id] = 1;
				}
				else
					lua_el_mode[id] = 1;
			}
			else if(lua_type(l, 3) == LUA_TBOOLEAN && !lua_toboolean(l, 3))
			{
				lua_el_func[id] = 0;
				lua_el_mode[id] = 0;
				ptypes[id].update_func = NULL;
			}
		}
		else if(!strcmp(propertyName,"Graphics"))
		{
			if(lua_type(l, 3) == LUA_TFUNCTION)
			{
				lua_pushvalue(l, 3);
				lua_gr_func[id] = luaL_ref(l, LUA_REGISTRYINDEX);
				graphicscache[id].isready = 0;
			}
			else if(lua_type(l, 3) == LUA_TBOOLEAN && !lua_toboolean(l, -1))
			{
				lua_gr_func[id] = 0;
				ptypes[id].graphics_func = NULL;
			}
			memset(graphicscache, 0, sizeof(gcache_item)*PT_NUM);
		}
		else
			return luaL_error(l, "Invalid element property");
	}
	else
	{
		int format;
		int offset = elements_getProperty(propertyName, &format, &modifiedStuff);

		if(offset != -1)
		{
			elements_writeProperty(l, id, format, offset);
			return 1;
		}
		else
			return luaL_error(l, "Invalid element property");
	}
	return 0;
}

int elements_free(lua_State * l)
{
	int id;
	luaL_checktype(l, 1, LUA_TNUMBER);
	id = lua_tointeger(l, 1);
	
	if(id < 0 || id >= PT_NUM || !globalSim->elements[id].Enabled)
		return luaL_error(l, "Invalid element");

	if (globalSim->elements[id].Identifier.find("DEFAULT") != globalSim->elements[id].Identifier.npos)
		return luaL_error(l, "Cannot free default elements");

	globalSim->elements[id].Enabled = ptypes[id].enabled = 0;

	lua_getglobal(l, "elements");
	lua_pushnil(l);
	lua_setfield(l, -2, globalSim->elements[id].Identifier.c_str());
	lua_pop(l, 1);

	return 0;
}
