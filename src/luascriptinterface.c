#include <defines.h>
#include <luascriptinterface.h>
#include <powder.h>
#include <gravity.h>
#include <powdergraphics.h>
#include <dirent.h>
#ifdef WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif
#include <sys/stat.h>

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

/*

ELEMENTS API

*/

char * getIdentifier(int i)
{
	char identifier[24];
	if (i == PT_EQUALVEL)
		sprintf(identifier,"DEFAULT_PT_116");//This list is much larger than I expected ...
	else if (i == 146)
		sprintf(identifier,"DEFAULT_PT_146");
	else if (i == PT_BANG)
		sprintf(identifier,"DEFAULT_PT_BANG");
	else if (i == PT_BIZRG)
		sprintf(identifier,"DEFAULT_PT_BIZRG");
	else if (i == PT_BIZRS)
		sprintf(identifier,"DEFAULT_PT_BIZRS");
	else if (i == PT_BHOL)
		sprintf(identifier,"DEFAULT_PT_BHOL");
	else if (i == PT_WHOL)
		sprintf(identifier,"DEFAULT_PT_WHOL");
	else if (i == PT_NBHL)
		sprintf(identifier,"DEFAULT_PT_NBHL");
	else if (i == PT_NWHL)
		sprintf(identifier,"DEFAULT_PT_NWHL");
	else if (i == PT_BREL)
		sprintf(identifier,"DEFAULT_PT_BREC");
	else if (i == PT_CBNW)
		sprintf(identifier,"DEFAULT_PT_CBNW");
	else if (i == PT_H2)
		sprintf(identifier,"DEFAULT_PT_H2");
	else if (i == PT_HFLM)
		sprintf(identifier,"DEFAULT_PT_HFLM");
	else if (i == PT_ICEI)
		sprintf(identifier,"DEFAULT_PT_ICEI");
	else if (i == PT_INVIS)
		sprintf(identifier,"DEFAULT_PT_INVIS");
	else if (i == PT_LNTG)
		sprintf(identifier,"DEFAULT_PT_LNTG");
	else if (i == PT_LO2)
		sprintf(identifier,"DEFAULT_PT_LO2");
	else if (i == PT_NONE)
		sprintf(identifier,"DEFAULT_PT_NONE");
	else if (i == PT_O2)
		sprintf(identifier,"DEFAULT_PT_O2");
	else if (i == PT_PLEX)
		sprintf(identifier,"DEFAULT_PT_PLEX");
	else if (i == PT_SHLD1)
		sprintf(identifier,"DEFAULT_PT_SHLD1");
	else if (i == PT_SHLD2)
		sprintf(identifier,"DEFAULT_PT_SHLD2");
	else if (i == PT_SHLD3)
		sprintf(identifier,"DEFAULT_PT_SHLD3");
	else if (i == PT_SHLD4)
		sprintf(identifier,"DEFAULT_PT_SHLD4");
	else if (i == PT_SHLD4)
		sprintf(identifier,"DEFAULT_PT_SHLD1");
	else if (i == PT_SPAWN)
		sprintf(identifier,"DEFAULT_PT_SPAWN");
	else if (i == PT_SPAWN2)
		sprintf(identifier,"DEFAULT_PT_SPAWN2");
	else if (i == PT_STKM2)
		sprintf(identifier,"DEFAULT_PT_STKM2");
	else if (i == PT_SHLD4)
		sprintf(identifier,"DEFAULT_PT_SHLD1");
	else if (i == PT_SHLD4)
		sprintf(identifier,"DEFAULT_PT_SHLD1");
	else
		sprintf(identifier,"DEFAULT_PT_%s",ptypes[i].name);
	return strdup(identifier);
}

void initIdentifiers()
{
	int i;
	for (i = 0; i < PT_NUM; i++)
		pidentifiers[i] = getIdentifier(i);
}

void initElementsAPI(lua_State * l)
{
	int elementsAPI, i;
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

	elementsAPI = lua_gettop(l);

	//Static values
	//Element types/properties/states
	lua_pushinteger(l, TYPE_PART); 			lua_setfield(l, elementsAPI, "TYPE_PART");
	lua_pushinteger(l, TYPE_LIQUID); 		lua_setfield(l, elementsAPI, "TYPE_LIQUID");
	lua_pushinteger(l, TYPE_SOLID); 		lua_setfield(l, elementsAPI, "TYPE_SOLID");
	lua_pushinteger(l, TYPE_GAS); 			lua_setfield(l, elementsAPI, "TYPE_GAS");
	lua_pushinteger(l, TYPE_ENERGY); 		lua_setfield(l, elementsAPI, "TYPE_ENERGY");
	lua_pushinteger(l, PROP_CONDUCTS); 		lua_setfield(l, elementsAPI, "PROP_CONDUCTS");
	lua_pushinteger(l, PROP_BLACK); 		lua_setfield(l, elementsAPI, "PROP_BLACK");
	lua_pushinteger(l, PROP_NEUTPENETRATE); lua_setfield(l, elementsAPI, "PROP_NEUTPENETRATE");
	lua_pushinteger(l, PROP_NEUTABSORB); 	lua_setfield(l, elementsAPI, "PROP_NEUTABSORB");
	lua_pushinteger(l, PROP_NEUTPASS); 		lua_setfield(l, elementsAPI, "PROP_NEUTPASS");
	lua_pushinteger(l, PROP_DEADLY); 		lua_setfield(l, elementsAPI, "PROP_DEADLY");
	lua_pushinteger(l, PROP_HOT_GLOW); 		lua_setfield(l, elementsAPI, "PROP_HOT_GLOW");
	lua_pushinteger(l, PROP_LIFE); 			lua_setfield(l, elementsAPI, "PROP_LIFE");
	lua_pushinteger(l, PROP_RADIOACTIVE); 	lua_setfield(l, elementsAPI, "PROP_RADIOACTIVE");
	lua_pushinteger(l, PROP_LIFE_DEC); 		lua_setfield(l, elementsAPI, "PROP_LIFE_DEC");
	lua_pushinteger(l, PROP_LIFE_KILL); 	lua_setfield(l, elementsAPI, "PROP_LIFE_KILL");
	lua_pushinteger(l, PROP_LIFE_KILL_DEC); lua_setfield(l, elementsAPI, "PROP_LIFE_KILL_DEC");
	lua_pushinteger(l, PROP_INDESTRUCTIBLE); lua_setfield(l, elementsAPI, "PROP_INDESTRUCTIBLE");
	lua_pushinteger(l, PROP_CLONE);			lua_setfield(l, elementsAPI, "PROP_CLONE");
	lua_pushinteger(l, PROP_BREAKABLECLONE); lua_setfield(l, elementsAPI, "PROP_BREAKABLECLONE");
	lua_pushinteger(l, PROP_POWERED);		lua_setfield(l, elementsAPI, "PROP_POWERED");
	lua_pushinteger(l, PROP_SPARKSETTLE); 	lua_setfield(l, elementsAPI, "PROP_SPARKSETTLE");
	lua_pushinteger(l, PROP_NOAMBHEAT); 	lua_setfield(l, elementsAPI, "PROP_NOAMBHEAT");
	lua_pushinteger(l, PROP_MOVS); 			lua_setfield(l, elementsAPI, "PROP_MOVS");
	lua_pushinteger(l, FLAG_STAGNANT); 		lua_setfield(l, elementsAPI, "FLAG_STAGNANT");
	lua_pushinteger(l, FLAG_SKIPMOVE); 		lua_setfield(l, elementsAPI, "FLAG_SKIPMOVE");
	lua_pushinteger(l, 0); 					lua_setfield(l, elementsAPI, "FLAG_MOVABLE");
	lua_pushinteger(l, FLAG_EXPLODE); 		lua_setfield(l, elementsAPI, "FLAG_EXPLODE");
	lua_pushinteger(l, FLAG_INSTACTV); 		lua_setfield(l, elementsAPI, "FLAG_INSTACTV");
	lua_pushinteger(l, FLAG_WATEREQUAL); 	lua_setfield(l, elementsAPI, "FLAG_WATEREQUAL");
	lua_pushinteger(l, ST_NONE); 			lua_setfield(l, elementsAPI, "ST_NONE");
	lua_pushinteger(l, ST_SOLID); 			lua_setfield(l, elementsAPI, "ST_SOLID");
	lua_pushinteger(l, ST_LIQUID); 			lua_setfield(l, elementsAPI, "ST_LIQUID");
	lua_pushinteger(l, ST_GAS); 			lua_setfield(l, elementsAPI, "ST_GAS");

	lua_pushinteger(l, SC_WALL);		lua_setfield(l, elementsAPI, "SC_WALL");
	lua_pushinteger(l, SC_ELEC);		lua_setfield(l, elementsAPI, "SC_ELEC");
	lua_pushinteger(l, SC_POWERED);		lua_setfield(l, elementsAPI, "SC_POWERED");
	lua_pushinteger(l, SC_FORCE);		lua_setfield(l, elementsAPI, "SC_FORCE");
	lua_pushinteger(l, SC_EXPLOSIVE);	lua_setfield(l, elementsAPI, "SC_EXPLOSIVE");
	lua_pushinteger(l, SC_GAS);			lua_setfield(l, elementsAPI, "SC_GAS");
	lua_pushinteger(l, SC_LIQUID);		lua_setfield(l, elementsAPI, "SC_LIQUID");
	lua_pushinteger(l, SC_POWDERS);		lua_setfield(l, elementsAPI, "SC_POWDERS");
	lua_pushinteger(l, SC_SOLIDS);		lua_setfield(l, elementsAPI, "SC_SOLIDS");
	lua_pushinteger(l, SC_NUCLEAR);		lua_setfield(l, elementsAPI, "SC_NUCLEAR");
	lua_pushinteger(l, SC_SPECIAL);		lua_setfield(l, elementsAPI, "SC_SPECIAL");
	lua_pushinteger(l, SC_LIFE);		lua_setfield(l, elementsAPI, "SC_LIFE");
	lua_pushinteger(l, SC_TOOL);		lua_setfield(l, elementsAPI, "SC_TOOL");
	lua_pushinteger(l, SC_DECO);		lua_setfield(l, elementsAPI, "SC_DECO");
	lua_pushinteger(l, SC_FAV);			lua_setfield(l, elementsAPI, "SC_FAV");
	lua_pushinteger(l, SC_FAV2);		lua_setfield(l, elementsAPI, "SC_FAV2");
	lua_pushinteger(l, SC_HUD);			lua_setfield(l, elementsAPI, "SC_HUD");
	lua_pushinteger(l, SC_CRACKER);		lua_setfield(l, elementsAPI, "SC_CRACKER");

	//Element identifiers
	for(i = 0; i < PT_NUM; i++)
	{
		if(ptypes[i].enabled)
		{
			char realidentifier[24];
			lua_pushinteger(l, i);
			lua_setfield(l, elementsAPI, pidentifiers[i]);
			sprintf(realidentifier,"DEFAULT_PT_%s",ptypes[i].name);
			if (i != 0 && i != PT_NBHL && i != PT_NWHL && strcmp(pidentifiers[i], realidentifier))
			{
				lua_pushinteger(l, i);
				lua_setfield(l, elementsAPI, realidentifier);
			}
		}
	}
}

int elements_getProperty(char * key, int * format)
{
	int offset;
	if (strcmp(key, "Name")==0){
		offset = offsetof(part_type, name);
		*format = 2;
	}
	else if (strcmp(key, "Color")==0){
		offset = offsetof(part_type, pcolors);
		*format = 4;
	}
	else if (strcmp(key, "Colour")==0){
		offset = offsetof(part_type, pcolors);
		*format = 4;
	}
	else if (strcmp(key, "Advection")==0){
		offset = offsetof(part_type, advection);
		*format = 1;
	}
	else if (strcmp(key, "AirDrag")==0){
		offset = offsetof(part_type, airdrag);
		*format = 1;
	}
	else if (strcmp(key, "AirLoss")==0){
		offset = offsetof(part_type, airloss);
		*format = 1;
	}
	else if (strcmp(key, "Loss")==0){
		offset = offsetof(part_type, loss);
		*format = 1;
	}
	else if (strcmp(key, "Collision")==0){
		offset = offsetof(part_type, collision);
		*format = 1;
	}
	else if (strcmp(key, "Gravity")==0){
		offset = offsetof(part_type, gravity);
		*format = 1;
	}
	else if (strcmp(key, "Diffusion")==0){
		offset = offsetof(part_type, diffusion);
		*format = 1;
	}
	else if (strcmp(key, "HotAir")==0){
		offset = offsetof(part_type, hotair);
		*format = 1;
	}
	else if (strcmp(key, "Falldown")==0){
		offset = offsetof(part_type, falldown);
		*format = 0;
	}
	else if (strcmp(key, "Flammable")==0){
		offset = offsetof(part_type, flammable);
		*format = 0;
	}
	else if (strcmp(key, "Explosive")==0){
		offset = offsetof(part_type, explosive);
		*format = 0;
	}
	else if (strcmp(key, "Meltable")==0){
		offset = offsetof(part_type, meltable);
		*format = 0;
	}
	else if (strcmp(key, "Hardness")==0){
		offset = offsetof(part_type, hardness);
		*format = 0;
	}
	else if (strcmp(key, "MenuVisible")==0){
		offset = offsetof(part_type, menu);
		*format = 0;
	}
	else if (strcmp(key, "Enabled")==0){
		offset = offsetof(part_type, enabled);
		*format = 0;
	}
	else if (strcmp(key, "Weight")==0){
		offset = offsetof(part_type, weight);
		*format = 0;
	}
	else if (strcmp(key, "MenuSection")==0){
		offset = offsetof(part_type, menusection);
		*format = 0;
	}
	else if (strcmp(key, "Temperature")==0){
		offset = offsetof(part_type, heat);
		*format = 1;
	}
	else if (strcmp(key, "HeatConduct")==0){
		offset = offsetof(part_type, hconduct);
		*format = 3;
	}
	else if (strcmp(key, "State")==0){
		offset = offsetof(part_type, state);
		*format = 6;
	}
	else if (strcmp(key, "Properties")==0){
		offset = offsetof(part_type, properties);
		*format = 5;
	}
	else if (strcmp(key, "Description")==0){
		offset = offsetof(part_type, descs);
		*format = 2;
	}
	else if (strcmp(key, "LowPressure")==0){
		offset = offsetof(part_transition, plv);
		*format = 8;
	}
	else if (strcmp(key, "LowPressureTransition")==0){
		offset = offsetof(part_transition, plt);
		*format = 7;
	}
	else if (strcmp(key, "HighPressure")==0){
		offset = offsetof(part_transition, phv);
		*format = 8;
	}
	else if (strcmp(key, "HighPressureTransition")==0){
		offset = offsetof(part_transition, pht);
		*format = 7;
	}
	else if (strcmp(key, "LowTemperature")==0){
		offset = offsetof(part_transition, tlv);
		*format = 8;
	}
	else if (strcmp(key, "LowTemperatureTransition")==0){
		offset = offsetof(part_transition, tlt);
		*format = 7;
	}
	else if (strcmp(key, "HighTemperature")==0){
		offset = offsetof(part_transition, thv);
		*format = 8;
	}
	else if (strcmp(key, "HighTemperatureTransition")==0){
		offset = offsetof(part_transition, tht);
		*format = 7;
	}
	else {
		return -1;
	}
	return offset;
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
		lua_setfield(l, -2, pidentifiers[id]);

		if(id < PT_NUM)
		{
			ptypes[id] = ptypes2[id];
			ptransitions[id] = ptransitions2[id];
			pidentifiers[id] = getIdentifier(id);
		}
		//else
		//	luacon_sim->elements[id] = Element();

		lua_pushinteger(l, id);
		lua_setfield(l, -2, pidentifiers[id]);
		lua_pop(l, 1);
	}
	else
	{
		memcpy(ptypes,ptypes2,sizeof(ptypes2));
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
		if(ptypes[i].enabled && !strcmp(pidentifiers[i],identifier))
			return luaL_error(l, "Element identifier already in use");
	}

	for(i = PT_NUM-1; i >= 0; i--)
	{
		if(!ptypes[i].enabled)
		{
			newID = i;
			ptypes[i].pcolors = 0xFF00FF;
			ptypes[i].airloss = 1.0f;
			ptypes[i].loss = 1.0f;
			ptypes[i].hardness = 30;
			ptypes[i].weight = 50;
			ptypes[i].heat = 273.15f;
			ptypes[i].hconduct = 128;
			ptypes[i].descs = "No description";
			ptypes[i].state = ST_SOLID;
			ptypes[i].properties = TYPE_SOLID;
			ptypes[i].enabled = 1;
			ptransitions[i].pht = ptransitions[i].plt = ptransitions[i].tht = ptransitions[i].tlt = -1;
			ptransitions[i].phv = 257.0f;
			ptransitions[i].plv = -257.0f;
			ptransitions[i].thv = MAX_TEMP+1;
			ptransitions[i].tlv = MIN_TEMP-1;
			pidentifiers[i]  = strdup(identifier);
			break;
		}
	}

	if(newID != -1)
	{	
		lua_getglobal(l, "elements");
		lua_pushinteger(l, newID);
		lua_setfield(l, -2, pidentifiers[i]);
		lua_pop(l, 1);
	}

	lua_pushinteger(l, newID);
	return 1;
}

int elements_element(lua_State * l)
{
	int args = lua_gettop(l);
	int id, i = 0;
	char *propertyList[] = { "Name", "Colour", "Color", "MenuVisible", "MenuSection", "Advection", "AirDrag", "AirLoss", "Loss", "Collision", "Gravity", "Diffusion", "HotAir", "Falldown", "Flammable", "Explosive", "Meltable", "Hardness", "Weight", "Temperature", "HeatConduct", "Description", "State", "Properties", "LowPressure", "LowPressureTransition", "HighPressure", "HighPressureTransition", "LowTemperature", "LowTemperatureTransition", "HighTemperature", "HighTemperatureTransition", NULL};
	luaL_checktype(l, 1, LUA_TNUMBER);
	id = lua_tointeger(l, 1);

	if(id < 0 || id >= PT_NUM || !ptypes[id].enabled)
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
				int offset = elements_getProperty(propertyList[i], &format);
				switch(format)
				{
					case 0: //Int
						*((int*)(((unsigned char*)&ptypes[id])+offset)) = lua_tointeger(l, -1);
						break;
					case 1: //Float
						*((float*)(((unsigned char*)&ptypes[id])+offset)) = lua_tonumber(l, -1);
						break;
					case 2: //String
						*((char**)(((unsigned char*)&ptypes[id])+offset)) = strdup(lua_tostring(l, -1));
						break;
					//Extra things just used for one type ...
					case 3: //Unsigned char (hconduct)
						*((unsigned char*)(((unsigned char*)&ptypes[id])+offset)) = lua_tointeger(l, -1);
						break;
					case 4: //Color (color)
#if PIXELSIZE == 4
						*((unsigned int*)(((unsigned char*)&ptypes[id])+offset)) = lua_tointeger(l, -1);
#else
						*((unsigned short*)(((unsigned char*)&ptypes[id])+offset)) = lua_tointeger(l, -1);
#endif
						break;
					case 5: //Unsigned int (properties)
						*((unsigned int*)(((unsigned char*)&ptypes[id])+offset)) = lua_tointeger(l, -1);
						break;
					case 6: //Char (state)
						*((char*)(((unsigned char*)&ptypes[id])+offset)) = lua_tointeger(l, -1);
						break;
					//Transitions, in a separate array so done separately
					case 7: //Int
						*((int*)(((unsigned char*)&ptransitions[id])+offset)) = lua_tointeger(l, -1);
						break;
					case 8: //Float
						*((float*)(((unsigned char*)&ptransitions[id])+offset)) = lua_tonumber(l, -1);
						break;
				}
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
			int offset = elements_getProperty(propertyList[i], &format);
			switch(format)
			{
				case 0: //Int
					lua_pushinteger(l, *((int*)(((unsigned char*)&ptypes[id])+offset)));
					break;
				case 1: //Float
					lua_pushnumber(l, *((float*)(((unsigned char*)&ptypes[id])+offset)));
					break;
				case 2: //String
					lua_pushstring(l, *((char**)(((unsigned char*)&ptypes[id])+offset)));
					break;
				//Extra things just used for one type ...
				case 3: //Unsigned char (hconduct)
					lua_pushinteger(l, *((unsigned char*)(((unsigned char*)&ptypes[id])+offset)));
					break;
				case 4: //Color (color)
#if PIXELSIZE == 4
					lua_pushinteger(l, *((unsigned int*)(((unsigned char*)&ptypes[id])+offset)));
#else
					lua_pushinteger(l, *((unsigned short*)(((unsigned char*)&ptypes[id])+offset)));
#endif
					break;
				case 5: //Unsigned int (properties)
					lua_pushinteger(l, *((unsigned int*)(((unsigned char*)&ptypes[id])+offset)));
					break;
				case 6: //Char (state)
					lua_pushinteger(l, *((char*)(((unsigned char*)&ptypes[id])+offset)));
					break;
				//Transitions, in a separate array so done separately
				case 7: //Int
					lua_pushinteger(l, *((int*)(((unsigned char*)&ptransitions[id])+offset)));
					break;
				case 8: //Float
					lua_pushnumber(l, *((float*)(((unsigned char*)&ptransitions[id])+offset)));
					break;
				default:
					lua_pushnil(l);
			}
			lua_setfield(l, -2, propertyList[i]);
			i++;
		}
		return 1;
	}
	return 0;
}

int elements_property(lua_State * l)
{
	int args = lua_gettop(l);
	int id;
	char *propertyName;
	luaL_checktype(l, 1, LUA_TNUMBER);
	id = lua_tointeger(l, 1);
	luaL_checktype(l, 2, LUA_TSTRING);
	propertyName = (char*)lua_tostring(l, 2);

	if(id < 0 || id >= PT_NUM || !ptypes[id].enabled)
		return luaL_error(l, "Invalid element");

	if(args > 2)
	{
		int format;
		int offset = elements_getProperty(propertyName, &format);

		if(offset != -1)
		{
			if(lua_type(l, 3) != LUA_TNIL)
			{
				unsigned int col;
				switch(format)
				{
					case 0: //Int
						*((int*)(((unsigned char*)&ptypes[id])+offset)) = lua_tointeger(l, 3);
						break;
					case 1: //Float
						*((float*)(((unsigned char*)&ptypes[id])+offset)) = lua_tonumber(l, 3);
						break;
					case 2: //String
						*((char**)(((unsigned char*)&ptypes[id])+offset)) = strdup(lua_tostring(l, 3));
						break;
					//Extra things just used for one type ...
					case 3: //Unsigned char (hconduct)
						*((unsigned char*)(((unsigned char*)&ptypes[id])+offset)) = lua_tointeger(l, 3);
						break;
					case 4: //Color (color)
#if PIXELSIZE == 4
						col = (unsigned int)lua_tonumber(l, 3);
						*((unsigned int*)(((unsigned char*)&ptypes[id])+offset)) = col;
#else
						*((unsigned short*)(((unsigned char*)&ptypes[id])+offset)) = lua_tointeger(l, 3);
#endif
						break;
					case 5: //Unsigned int (properties)
						*((unsigned int*)(((unsigned char*)&ptypes[id])+offset)) = lua_tointeger(l, 3);
						break;
					case 6: //Char (state)
						*((char*)(((unsigned char*)&ptypes[id])+offset)) = lua_tointeger(l, 3);
						break;
					//Transitions, in a separate array so done separately
					case 7: //Int
						*((int*)(((unsigned char*)&ptransitions[id])+offset)) = lua_tointeger(l, 3);
						break;
					case 8: //Float
						*((float*)(((unsigned char*)&ptransitions[id])+offset)) = lua_tonumber(l, 3);
						break;
				}
			}

			menu_count();
			init_can_move();
			graphicscache[id].isready = 0;

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
		int offset = elements_getProperty(propertyName, &format);

		if(offset != -1)
		{
			switch(format)
			{
				case 0: //Int
					lua_pushinteger(l, *((int*)(((unsigned char*)&ptypes[id])+offset)));
					break;
				case 1: //Float
					lua_pushnumber(l, *((float*)(((unsigned char*)&ptypes[id])+offset)));
					break;
				case 2: //String
					lua_pushstring(l, *((char**)(((unsigned char*)&ptypes[id])+offset)));
					break;
				//Extra things just used for one type ...
				case 3: //Unsigned char (hconduct)
					lua_pushinteger(l, *((unsigned char*)(((unsigned char*)&ptypes[id])+offset)));
					break;
				case 4: //Color (color)
#if PIXELSIZE == 4
					lua_pushinteger(l, *((unsigned int*)(((unsigned char*)&ptypes[id])+offset)));
#else
					lua_pushinteger(l, *((unsigned short*)(((unsigned char*)&ptypes[id])+offset)));
#endif
					break;
				case 5: //Unsigned int (properties)
					lua_pushinteger(l, *((unsigned int*)(((unsigned char*)&ptypes[id])+offset)));
					break;
				case 6: //Char (state)
					lua_pushinteger(l, *((char*)(((unsigned char*)&ptypes[id])+offset)));
					break;
				//Transitions, in a separate array so done separately
				case 7: //Int
					lua_pushinteger(l, *((int*)(((unsigned char*)&ptransitions[id])+offset)));
					break;
				case 8: //Float
					lua_pushnumber(l, *((float*)(((unsigned char*)&ptransitions[id])+offset)));
					break;
				default:
					lua_pushnil(l);
			}
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
	char *identifier;
	luaL_checktype(l, 1, LUA_TNUMBER);
	id = lua_tointeger(l, 1);
	
	if(id < 0 || id >= PT_NUM || !ptypes[id].enabled)
		return luaL_error(l, "Invalid element");

	identifier = pidentifiers[id];
	if(strstr(identifier,"DEFAULT") == identifier)
		return luaL_error(l, "Cannot free default elements");

	ptypes[id].enabled = 0;

	lua_getglobal(l, "elements");
	lua_pushnil(l);
	lua_setfield(l, -2, identifier);
	lua_pop(l, 1);

	return 0;
}
