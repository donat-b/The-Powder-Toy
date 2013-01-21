#include <defines.h>
#include <luascriptinterface.h>
#include <powder.h>
#include <gravity.h>
#include <powdergraphics.h>

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
	lua_pushinteger(l, EFFECT_LINES); lua_setfield(l, rendererAPI, "EFFECT_DBGLINES");

	//Display/Render/Colour modes
	lua_pushinteger(l, RENDER_EFFE); lua_setfield(l, rendererAPI, "RENDER_EFFE");
	lua_pushinteger(l, RENDER_FIRE); lua_setfield(l, rendererAPI, "RENDER_FIRE");
	lua_pushinteger(l, RENDER_GLOW); lua_setfield(l, rendererAPI, "RENDER_GLOW");
	lua_pushinteger(l, RENDER_BLUR); lua_setfield(l, rendererAPI, "RENDER_BLUR");
	lua_pushinteger(l, DISPLAY_BLOB); lua_setfield(l, rendererAPI, "RENDER_BLOB");
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