#include <defines.h>
#include <luascriptinterface.h>
#include <powder.h>
#include <gravity.h>
#include <powdergraphics.h>
#include <dirent.h>
#ifdef WIN32
#include <direct.h>
#endif

/*

SIMULATION API

*/

void initSimulationAPI(lua_State * l)
{
	int simulationAPI;
	//Methods
	struct luaL_reg simulationAPIMethods [] = {
		{"partNeighbours", simulation_partNeighbours},
		{"partChangeType", simulation_partChangeType},
		{"partCreate", simulation_partCreate},
		{"partKill", simulation_partKill},
		{"pressure", simulation_pressure},
		{"ambientHeat", simulation_ambientHeat},
		{"velocityX", simulation_velocityX},
		{"velocityY", simulation_velocityY},
		{"gravMap", simulation_gravMap},
		{NULL, NULL}
	};
	luaL_register(l, "simulation", simulationAPIMethods);
	simulationAPI = lua_gettop(l);

	//Sim shortcut
	lua_getglobal(l, "simulation");
	lua_setglobal(l, "sim");

	//Static values
	lua_pushinteger(l, XRES); lua_setfield(l, simulationAPI, "XRES");
	lua_pushinteger(l, YRES); lua_setfield(l, simulationAPI, "YRES");
	lua_pushinteger(l, PT_NUM); lua_setfield(l, simulationAPI, "PT_NUM");
	lua_pushinteger(l, NUM_PARTS); lua_setfield(l, simulationAPI, "NUM_PARTS");
	lua_pushinteger(l, R_TEMP); lua_setfield(l, simulationAPI, "R_TEMP");
	lua_pushinteger(l, MAX_TEMP); lua_setfield(l, simulationAPI, "MAX_TEMP");
	lua_pushinteger(l, MIN_TEMP); lua_setfield(l, simulationAPI, "MIN_TEMP");

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
	int rendererAPI;
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

	rendererAPI = lua_gettop(l);

	//Static values
	//Particle pixel modes/fire mode/effects
	lua_pushinteger(l, PMODE); lua_setfield(l, rendererAPI, "PMODE");
	lua_pushinteger(l, PMODE_NONE); lua_setfield(l, rendererAPI, "PMODE_NONE");
	lua_pushinteger(l, PMODE_FLAT); lua_setfield(l, rendererAPI, "PMODE_FLAT");
	lua_pushinteger(l, PMODE_BLOB); lua_setfield(l, rendererAPI, "PMODE_BLOB");
	lua_pushinteger(l, PMODE_BLUR); lua_setfield(l, rendererAPI, "PMODE_BLUR");
	lua_pushinteger(l, PMODE_GLOW); lua_setfield(l, rendererAPI, "PMODE_GLOW");
	lua_pushinteger(l, PMODE_SPARK); lua_setfield(l, rendererAPI, "PMODE_SPARK");
	lua_pushinteger(l, PMODE_FLARE); lua_setfield(l, rendererAPI, "PMODE_FLARE");
	lua_pushinteger(l, PMODE_LFLARE); lua_setfield(l, rendererAPI, "PMODE_LFLARE");
	lua_pushinteger(l, PMODE_ADD); lua_setfield(l, rendererAPI, "PMODE_ADD");
	lua_pushinteger(l, PMODE_BLEND); lua_setfield(l, rendererAPI, "PMODE_BLEND");
	lua_pushinteger(l, PSPEC_STICKMAN); lua_setfield(l, rendererAPI, "PSPEC_STICKMAN");
	lua_pushinteger(l, OPTIONS); lua_setfield(l, rendererAPI, "OPTIONS");
	lua_pushinteger(l, NO_DECO); lua_setfield(l, rendererAPI, "NO_DECO");
	lua_pushinteger(l, DECO_FIRE); lua_setfield(l, rendererAPI, "DECO_FIRE");
	lua_pushinteger(l, FIREMODE); lua_setfield(l, rendererAPI, "FIREMODE");
	lua_pushinteger(l, FIRE_ADD); lua_setfield(l, rendererAPI, "FIRE_ADD");
	lua_pushinteger(l, FIRE_BLEND); lua_setfield(l, rendererAPI, "FIRE_BLEND");
	lua_pushinteger(l, EFFECT); lua_setfield(l, rendererAPI, "EFFECT");
	lua_pushinteger(l, EFFECT_GRAVIN); lua_setfield(l, rendererAPI, "EFFECT_GRAVIN");
	lua_pushinteger(l, EFFECT_GRAVOUT); lua_setfield(l, rendererAPI, "EFFECT_GRAVOUT");
	lua_pushinteger(l, EFFECT_LINES); lua_setfield(l, rendererAPI, "EFFECT_LINES");
	lua_pushinteger(l, EFFECT_DBGLINES); lua_setfield(l, rendererAPI, "EFFECT_DBGLINES");

	//Display/Render/Colour modes
	lua_pushinteger(l, RENDER_EFFE); lua_setfield(l, rendererAPI, "RENDER_EFFE");
	lua_pushinteger(l, RENDER_FIRE); lua_setfield(l, rendererAPI, "RENDER_FIRE");
	lua_pushinteger(l, RENDER_GLOW); lua_setfield(l, rendererAPI, "RENDER_GLOW");
	lua_pushinteger(l, RENDER_BLUR); lua_setfield(l, rendererAPI, "RENDER_BLUR");
	lua_pushinteger(l, RENDER_BLOB); lua_setfield(l, rendererAPI, "RENDER_BLOB");
	lua_pushinteger(l, RENDER_BASC); lua_setfield(l, rendererAPI, "RENDER_BASC");
	lua_pushinteger(l, RENDER_NONE); lua_setfield(l, rendererAPI, "RENDER_NONE");
	lua_pushinteger(l, COLOUR_HEAT); lua_setfield(l, rendererAPI, "COLOUR_HEAT");
	lua_pushinteger(l, COLOUR_LIFE); lua_setfield(l, rendererAPI, "COLOUR_LIFE");
	lua_pushinteger(l, COLOUR_GRAD); lua_setfield(l, rendererAPI, "COLOUR_GRAD");
	lua_pushinteger(l, COLOUR_BASC); lua_setfield(l, rendererAPI, "COLOUR_BASC");
	lua_pushinteger(l, COLOUR_DEFAULT); lua_setfield(l, rendererAPI, "COLOUR_DEFAULT");
	lua_pushinteger(l, DISPLAY_AIRC); lua_setfield(l, rendererAPI, "DISPLAY_AIRC");
	lua_pushinteger(l, DISPLAY_AIRP); lua_setfield(l, rendererAPI, "DISPLAY_AIRP");
	lua_pushinteger(l, DISPLAY_AIRV); lua_setfield(l, rendererAPI, "DISPLAY_AIRV");
	lua_pushinteger(l, DISPLAY_AIRH); lua_setfield(l, rendererAPI, "DISPLAY_AIRH");
	lua_pushinteger(l, DISPLAY_AIR); lua_setfield(l, rendererAPI, "DISPLAY_AIR");
	lua_pushinteger(l, DISPLAY_WARP); lua_setfield(l, rendererAPI, "DISPLAY_WARP");
	lua_pushinteger(l, DISPLAY_PERS); lua_setfield(l, rendererAPI, "DISPLAY_PERS");
	lua_pushinteger(l, DISPLAY_EFFE); lua_setfield(l, rendererAPI, "DISPLAY_EFFE");
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
	int fileSystemAPI;
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

	fileSystemAPI = lua_gettop(l);
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
	const char * filename = lua_tostring(l, 1);

	int exists = 0;
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
			exists = 1;
		}
		else if(s.st_mode & S_IFREG)
		{
			exists = 1;
		}
		else
		{
			exists = 1;
		}
	}
	else
	{
		exists = 0;
	}

	lua_pushboolean(l, exists);
	return 1;
}

int fileSystem_isFile(lua_State * l)
{
	const char * filename = lua_tostring(l, 1);

	int exists = 0;
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
			exists = 1;
		}
		else if(s.st_mode & S_IFREG)
		{
			exists = 0;
		}
		else
		{
			exists = 0;
		}
	}
	else
	{
		exists = 0;
	}

	lua_pushboolean(l, exists);
	return 1;
}

int fileSystem_isDirectory(lua_State * l)
{
	const char * filename = lua_tostring(l, 1);

	int exists = 0;
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
			exists = 0;
		}
		else if(s.st_mode & S_IFREG)
		{
			exists = 1;
		}
		else
		{
			exists = 0;
		}
	}
	else
	{
		exists = 0;
	}

	lua_pushboolean(l, exists);
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
	int graphicsAPI;
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

	graphicsAPI = lua_gettop(l);

	lua_pushinteger(l, XRES+BARSIZE);	lua_setfield(l, graphicsAPI, "WIDTH");
	lua_pushinteger(l, YRES+MENUSIZE);	lua_setfield(l, graphicsAPI, "HEIGHT");
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
	draw_line(vid_buf, x1, y1, x2, y2, r, g, b, a);
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
