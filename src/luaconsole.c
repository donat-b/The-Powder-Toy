/**
 * Powder Toy - Lua console
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

#include <defines.h>

#ifdef LUACONSOLE
#include <powder.h>
#include "gravity.h"
#include <console.h>
#include <luaconsole.h>
#include <save.h>
#include <math.h>

lua_State *l;
int step_functions[6] = {0, 0, 0, 0, 0, 0};
int keypress_function_count = 0;
int *keypress_functions = NULL;
int mouseclick_function_count = 0;
int *mouseclick_functions = NULL;
int tptProperties; //Table for some TPT properties
int tptPropertiesVersion;
int tptElements; //Table for TPT element names
int tptParts, tptPartsMeta, tptElementTransitions, tptPartsCData, tptPartMeta, tptPart, cIndex;
void luacon_open(){
	int i = 0, j;
	char tmpname[12];
	int currentElementMeta, currentElement;
	const static struct luaL_reg tptluaapi [] = {
		{"test", &luatpt_test},
		{"drawtext", &luatpt_drawtext},
		{"create", &luatpt_create},
		{"set_pause", &luatpt_setpause},
		{"toggle_pause", &luatpt_togglepause},
		{"set_console", &luatpt_setconsole},
		{"log", &luatpt_log},
		{"set_pressure", &luatpt_set_pressure},
		{"set_aheat", &luatpt_set_aheat},
		{"set_velocity", &luatpt_set_velocity},
		{"set_gravity", &luatpt_set_gravity},
		{"reset_gravity_field", &luatpt_reset_gravity_field},
		{"reset_velocity", &luatpt_reset_velocity},
		{"reset_spark", &luatpt_reset_spark},
		{"set_property", &luatpt_set_property},
		{"get_property", &luatpt_get_property},
		{"set_wallmap",&luatpt_createwall},
		{"get_wallmap",&luatpt_getwall},
		{"set_elecmap",&luatpt_set_elecmap},
		{"get_elecmap",&luatpt_get_elecmap},
		{"drawpixel", &luatpt_drawpixel},
		{"drawrect", &luatpt_drawrect},
		{"fillrect", &luatpt_fillrect},
		{"drawcircle", &luatpt_drawcircle},
		{"fillcircle", &luatpt_fillcircle},
		{"drawline", &luatpt_drawline},
		{"textwidth", &luatpt_textwidth},
		{"get_name", &luatpt_get_name},
		{"set_shortcuts", &luatpt_set_shortcuts},
		{"delete", &luatpt_delete},
		{"register_step", &luatpt_register_step},
		{"unregister_step", &luatpt_unregister_step},
		{"register_mouseclick", &luatpt_register_mouseclick},
		{"unregister_mouseclick", &luatpt_unregister_mouseclick},
		{"register_keypress", &luatpt_register_keypress},
		{"unregister_keypress", &luatpt_unregister_keypress},
		{"register_mouseevent", &luatpt_register_mouseclick},
		{"unregister_mouseevent", &luatpt_unregister_mouseclick},
		{"register_keyevent", &luatpt_register_keypress},
		{"unregister_keyevent", &luatpt_unregister_keypress},
		{"input", &luatpt_input},
		{"message_box", &luatpt_message_box},
		{"get_numOfParts", &luatpt_get_numOfParts},
		{"start_getPartIndex", &luatpt_start_getPartIndex},
		{"next_getPartIndex", &luatpt_next_getPartIndex},
		{"getPartIndex", &luatpt_getPartIndex},
		{"hud", &luatpt_hud},
		{"newtonian_gravity", &luatpt_gravity},
		{"ambient_heat", &luatpt_airheat},
		{"active_menu", &luatpt_active_menu},
		{"decorations_enable", &luatpt_decorations_enable},
		{"display_mode", &luatpt_cmode_set},
		{"throw_error", &luatpt_error},
		{"heat", &luatpt_heat},
		{"setfire", &luatpt_setfire},
		{"setdebug", &luatpt_setdebug},
		{"setfpscap",&luatpt_setfpscap},
		{"getscript",&luatpt_getscript},
		{"setwindowsize",&luatpt_setwindowsize},
		{"watertest",&luatpt_togglewater},
		{"screenshot",&luatpt_screenshot},
		{"element",&luatpt_getelement},
		{"element_func",&luatpt_element_func},
		{"graphics_func",&luatpt_graphics_func},
		{"sound",&luatpt_sound},
		{"load",&luatpt_load},
		{"bubble",&luatpt_bubble},
		{"reset_pressure",&luatpt_reset_pressure},
		{"reset_temp",&luatpt_reset_temp},
		{"get_pressure",&luatpt_get_pressure},
		{"get_aheat",&luatpt_get_aheat},
		{"get_velocity",&luatpt_get_velocity},
		{"get_gravity",&luatpt_get_gravity},
		{"maxframes",&luatpt_maxframes},
		{"get_wall",&luatpt_getwall},
		{"create_wall",&luatpt_createwall},
		{"clear_sim",&luatpt_clear_sim},
		{"reset_elements",&luatpt_reset_elements},
		{"indestructible",&luatpt_indestructible},
		{"moving_solid",&luatpt_moving_solid},
		{"create_parts",&luatpt_create_parts},
		{"create_line",&luatpt_create_line},
		{"floodfill",&luatpt_floodfill},
		{"save_stamp",&luatpt_save_stamp},
		{"load_stamp",&luatpt_load_stamp},
		{NULL,NULL}
	};

	l = lua_open();
	luaL_openlibs(l);
	luaL_register(l, "tpt", tptluaapi);
	
	tptProperties = lua_gettop(l);
	
	lua_pushinteger(l, 0);
	lua_setfield(l, tptProperties, "mousex");
	lua_pushinteger(l, 0);
	lua_setfield(l, tptProperties, "mousey");
	lua_pushinteger(l, 0);
	lua_setfield(l, tptProperties, "selectedl");
	lua_pushinteger(l, 0);
	lua_setfield(l, tptProperties, "selectedr");
	
	lua_newtable(l);
	tptPropertiesVersion = lua_gettop(l);
	lua_pushinteger(l, SAVE_VERSION);
	lua_setfield(l, tptPropertiesVersion, "major");
	lua_pushinteger(l, MINOR_VERSION);
	lua_setfield(l, tptPropertiesVersion, "minor");
	lua_pushinteger(l, BUILD_NUM);
	lua_setfield(l, tptPropertiesVersion, "build");
	lua_pushinteger(l, MOD_VERSION);
	lua_setfield(l, tptPropertiesVersion, "jacob1s_mod");
	lua_setfield(l, tptProperties, "version");
	
#ifdef FFI
	//LuaJIT's ffi gives us direct access to parts data, no need for nested metatables. HOWEVER, this is in no way safe, it's entirely possible for someone to try to read parts[-10]
	lua_pushlightuserdata(l, parts);
	lua_setfield(l, tptProperties, "partsdata");
	
	luaL_dostring (l, "ffi = require(\"ffi\")\n\
ffi.cdef[[\n\
typedef struct { int type; int life, ctype; float x, y, vx, vy; float temp; float pavg[2]; int flags; int tmp; int tmp2; unsigned int dcolour; } particle;\n\
]]\n\
tpt.parts = ffi.cast(\"particle *\", tpt.partsdata)\n\
ffi = nil\n\
tpt.partsdata = nil");
	//Since ffi is REALLY REALLY dangrous, we'll remove it from the environment completely (TODO)
	//lua_pushstring(l, "parts");
	//tptPartsCData = lua_gettable(l, tptProperties);
#else
	lua_newtable(l);
	tptParts = lua_gettop(l);
	lua_newtable(l);
	tptPartsMeta = lua_gettop(l);
	lua_pushcfunction(l, luacon_partswrite);
	lua_setfield(l, tptPartsMeta, "__newindex");
	lua_pushcfunction(l, luacon_partsread);
	lua_setfield(l, tptPartsMeta, "__index");
	lua_setmetatable(l, tptParts);
	lua_setfield(l, tptProperties, "parts");
	
	lua_newtable(l);
	tptPart = lua_gettop(l);
	lua_newtable(l);
	tptPartMeta = lua_gettop(l);
	lua_pushcfunction(l, luacon_partwrite);
	lua_setfield(l, tptPartMeta, "__newindex");
	lua_pushcfunction(l, luacon_partread);
	lua_setfield(l, tptPartMeta, "__index");
	lua_setmetatable(l, tptPart);
	
	tptPart = luaL_ref(l, LUA_REGISTRYINDEX);
#endif
	
	lua_newtable(l);
	tptElements = lua_gettop(l);
	for(i = 1; i < PT_NUM; i++)
	{
		if (i == PT_EXPL)
			continue;
		for(j = 0; j < strlen(ptypes[i].name); j++)
			tmpname[j] = tolower(ptypes[i].name[j]);
		tmpname[strlen(ptypes[i].name)] = 0;
		
		lua_newtable(l);
		currentElement = lua_gettop(l);
		lua_pushinteger(l, i);
		lua_setfield(l, currentElement, "id");
		
		lua_newtable(l);
		currentElementMeta = lua_gettop(l);
		lua_pushcfunction(l, luacon_elementwrite);
		lua_setfield(l, currentElementMeta, "__newindex");
		lua_pushcfunction(l, luacon_elementread);
		lua_setfield(l, currentElementMeta, "__index");
		lua_setmetatable(l, currentElement);
		
		lua_setfield(l, tptElements, tmpname);
	}
	lua_setfield(l, tptProperties, "el");
	
	lua_newtable(l);
	tptElementTransitions = lua_gettop(l);
	for(i = 1; i < PT_NUM; i++)
	{
		if (i == PT_EXPL)
			continue;
		for(j = 0; j < strlen(ptypes[i].name); j++)
			tmpname[j] = tolower(ptypes[i].name[j]);
		tmpname[strlen(ptypes[i].name)] = 0;
		
		lua_newtable(l);
		currentElement = lua_gettop(l);		
		lua_newtable(l);
		currentElementMeta = lua_gettop(l);
		lua_pushinteger(l, i);
		lua_setfield(l, currentElement, "value");
		lua_pushcfunction(l, luacon_transitionwrite);
		lua_setfield(l, currentElementMeta, "__newindex");
		lua_pushcfunction(l, luacon_transitionread);
		lua_setfield(l, currentElementMeta, "__index");
		lua_setmetatable(l, currentElement);
		
		lua_setfield(l, tptElementTransitions, tmpname);
	}
	lua_setfield(l, tptProperties, "eltransition");
	
	lua_el_func = (int*)calloc(PT_NUM, sizeof(int));
	lua_el_mode = (int*)calloc(PT_NUM, sizeof(int));
	lua_gr_func = (int*)calloc(PT_NUM, sizeof(int));
	for(i = 0; i < PT_NUM; i++)
	{
		lua_el_mode[i] = 0;
		lua_gr_func[i] = 0;
	}
	lua_sethook(l, &lua_hook, LUA_MASKCOUNT, 4000000);
}
#ifndef FFI
int luacon_partread(lua_State* l){
	int format, offset, tempinteger;
	float tempfloat;
	int i;
	char * key = mystrdup((char*)luaL_optstring(l, 2, ""));
	offset = luacon_particle_getproperty(key, &format);

	i = cIndex;
	
	if(i < 0 || i >= NPART || offset==-1)
	{
		if(i < 0 || i >= NPART) {
			free(key);
			return luaL_error(l, "Out of range");
		} else if(strcmp(key, "id")==0) {
			free(key);
			lua_pushnumber(l, i);
			return 1;
		} else {
			free(key);
			return luaL_error(l, "Invalid property");
		}
	}
	free(key);
	switch(format)
	{
	case 0:
		tempinteger = *((int*)(((char*)&parts[i])+offset));
		lua_pushnumber(l, tempinteger);
		break;
	case 1:
		tempfloat = *((float*)(((char*)&parts[i])+offset));
		lua_pushnumber(l, tempfloat);
		break;
	}
	return 1;
}
int luacon_partwrite(lua_State* l){
	int format, offset;
	int i;
	char * key = mystrdup((char*)luaL_optstring(l, 2, ""));
	offset = luacon_particle_getproperty(key, &format);
	
	i = cIndex;
	
	if(i < 0 || i >= NPART || offset==-1)
	{
		if(i < 0 || i >= NPART) {
			free(key);
			return luaL_error(l, "array index out of bounds");
		} else {
			free(key);
			return luaL_error(l, "Invalid property");
		}
	}
	free(key);
	switch(format)
	{
	case 0:
		*((int*)(((char*)&parts[i])+offset)) = luaL_optinteger(l, 3, 0);
		break;
	case 1:
		*((float*)(((char*)&parts[i])+offset)) = luaL_optnumber(l, 3, 0);
		break;
	}
	return 1;
}
int luacon_partsread(lua_State* l){
	int format, offset;
	char * tempstring;
	int tempinteger;
	float tempfloat;
	int i, currentPart, currentPartMeta;
	
	i = luaL_optinteger(l, 2, 0);
	
	if(i<0 || i>=NPART)
	{
		return luaL_error(l, "array index out of bounds");
	}
	
	lua_rawgeti(l, LUA_REGISTRYINDEX, tptPart);
	cIndex = i;
	return 1;
}
int luacon_partswrite(lua_State* l){
	return luaL_error(l, "table readonly");
}
#endif
int luacon_particle_getproperty(char * key, int * format)
{
	int offset;
	if (strcmp(key, "type")==0){
		offset = offsetof(particle, type);
		*format = 0;
	} else if (strcmp(key, "life")==0){
		offset = offsetof(particle, life);
		*format = 0;
	} else if (strcmp(key, "ctype")==0){
		offset = offsetof(particle, ctype);
		*format = 0;
	} else if (strcmp(key, "temp")==0){
		offset = offsetof(particle, temp);
		*format = 1;
	} else if (strcmp(key, "tmp")==0){
		offset = offsetof(particle, tmp);
		*format = 0;
	} else if (strcmp(key, "tmp2")==0){
		offset = offsetof(particle, tmp2);
		*format = 0;
	} else if (strcmp(key, "vy")==0){
		offset = offsetof(particle, vy);
		*format = 1;
	} else if (strcmp(key, "vx")==0){
		offset = offsetof(particle, vx);
		*format = 1;
	} else if (strcmp(key, "x")==0){
		offset = offsetof(particle, x);
		*format = 1;
	} else if (strcmp(key, "y")==0){
		offset = offsetof(particle, y);
		*format = 1;
	} else if (strcmp(key, "dcolour")==0){
		offset = offsetof(particle, dcolour);
		*format = 0;
	} else if (strcmp(key, "dcolor")==0){
		offset = offsetof(particle, dcolour);
		*format = 0;
	} else {
		offset = -1;
	}
	return offset;
}
int luacon_transition_getproperty(char * key, int * format)
{
	int offset;
	if (strcmp(key, "presHighValue")==0){
		offset = offsetof(part_transition, phv);
		*format = 1;
	} else if (strcmp(key, "presHighType")==0){
		offset = offsetof(part_transition, pht);
		*format = 0;
	} else if (strcmp(key, "presLowValue")==0){
		offset = offsetof(part_transition, plv);
		*format = 1;
	} else if (strcmp(key, "presLowType")==0){
		offset = offsetof(part_transition, plt);
		*format = 0;
	} else if (strcmp(key, "tempHighValue")==0){
		offset = offsetof(part_transition, thv);
		*format = 1;
	} else if (strcmp(key, "tempHighType")==0){
		offset = offsetof(part_transition, tht);
		*format = 0;
	} else if (strcmp(key, "tempLowValue")==0){
		offset = offsetof(part_transition, tlv);
		*format = 1;
	} else if (strcmp(key, "tempLowType")==0){
		offset = offsetof(part_transition, tlt);
		*format = 0;
	} else {
		offset = -1;
	}
	return offset;
}
int luacon_transitionread(lua_State* l){
	int format, offset;
	int tempinteger;
	float tempfloat;
	int i;
	char * key = mystrdup((char*)luaL_optstring(l, 2, ""));
	offset = luacon_transition_getproperty(key, &format);
	free(key);
	
	//Get Raw Index value for element
	lua_pushstring(l, "value");
	lua_rawget(l, 1);
	
	i = lua_tointeger(l, lua_gettop(l));
	
	lua_pop(l, 1);
	
	if(i < 0 || i >= PT_NUM || offset==-1)
	{
		return luaL_error(l, "Invalid property");
	}
	switch(format)
	{
	case 0:
		tempinteger = *((int*)(((char*)&ptransitions[i])+offset));
		lua_pushnumber(l, tempinteger);
		break;
	case 1:
		tempfloat = *((float*)(((char*)&ptransitions[i])+offset));
		lua_pushnumber(l, tempfloat);
		break;
	}
	return 1;
}
int luacon_transitionwrite(lua_State* l){
	int format, offset;
	int tempinteger;
	float tempfloat;
	int i;
	char * key = mystrdup((char*)luaL_optstring(l, 2, ""));
	offset = luacon_transition_getproperty(key, &format);
	free(key);
	
	//Get Raw Index value for element
	lua_pushstring(l, "value");
	lua_rawget(l, 1);
	
	i = lua_tointeger (l, lua_gettop(l));
	
	lua_pop(l, 1);
	
	if(i < 0 || i >= PT_NUM || offset==-1)
	{
		return luaL_error(l, "Invalid property");
	}
	switch(format)
	{
	case 0:
		*((int*)(((char*)&ptransitions[i])+offset)) = luaL_optinteger(l, 3, 0);
		break;
	case 1:
		*((float*)(((char*)&ptransitions[i])+offset)) = luaL_optnumber(l, 3, 0);
		break;
	}
	return 0;
}
int luacon_element_getproperty(char * key, int * format, unsigned int * modified_stuff)
{
	int offset;
	if (strcmp(key, "name")==0){
		offset = offsetof(part_type, name);
		*format = 2;
	}
	else if (strcmp(key, "color")==0){
		offset = offsetof(part_type, pcolors);
		*format = 0;
		if (modified_stuff)
			*modified_stuff |= LUACON_EL_MODIFIED_GRAPHICS;
	}
	else if (strcmp(key, "colour")==0){
		offset = offsetof(part_type, pcolors);
		*format = 0;
		if (modified_stuff)
			*modified_stuff |= LUACON_EL_MODIFIED_GRAPHICS;
	}
	else if (strcmp(key, "advection")==0){
		offset = offsetof(part_type, advection);
		*format = 1;
	}
	else if (strcmp(key, "airdrag")==0){
		offset = offsetof(part_type, airdrag);
		*format = 1;
	}
	else if (strcmp(key, "airloss")==0){
		offset = offsetof(part_type, airloss);
		*format = 1;
	}
	else if (strcmp(key, "loss")==0){
		offset = offsetof(part_type, loss);
		*format = 1;
	}
	else if (strcmp(key, "collision")==0){
		offset = offsetof(part_type, collision);
		*format = 1;
	}
	else if (strcmp(key, "gravity")==0){
		offset = offsetof(part_type, gravity);
		*format = 1;
	}
	else if (strcmp(key, "diffusion")==0){
		offset = offsetof(part_type, diffusion);
		*format = 1;
	}
	else if (strcmp(key, "hotair")==0){
		offset = offsetof(part_type, hotair);
		*format = 1;
	}
	else if (strcmp(key, "falldown")==0){
		offset = offsetof(part_type, falldown);
		*format = 0;
	}
	else if (strcmp(key, "flammable")==0){
		offset = offsetof(part_type, flammable);
		*format = 0;
	}
	else if (strcmp(key, "explosive")==0){
		offset = offsetof(part_type, explosive);
		*format = 0;
	}
	else if (strcmp(key, "meltable")==0){
		offset = offsetof(part_type, meltable);
		*format = 0;
	}
	else if (strcmp(key, "hardness")==0){
		offset = offsetof(part_type, hardness);
		*format = 0;
	}
	else if (strcmp(key, "menu")==0){
		offset = offsetof(part_type, menu);
		*format = 0;
		if (modified_stuff)
			*modified_stuff |= LUACON_EL_MODIFIED_MENUS;
	}
	else if (strcmp(key, "enabled")==0){
		offset = offsetof(part_type, enabled);
		*format = 0;
	}
	else if (strcmp(key, "weight")==0){
		offset = offsetof(part_type, weight);
		*format = 0;
		if (modified_stuff)
			*modified_stuff |= LUACON_EL_MODIFIED_CANMOVE;
	}
	else if (strcmp(key, "menusection")==0){
		offset = offsetof(part_type, menusection);
		*format = 0;
		if (modified_stuff)
			*modified_stuff |= LUACON_EL_MODIFIED_MENUS;
	}
	else if (strcmp(key, "heat")==0){
		offset = offsetof(part_type, heat);
		*format = 1;
	}
	else if (strcmp(key, "hconduct")==0){
		offset = offsetof(part_type, hconduct);
		*format = 3;
	}
	else if (strcmp(key, "state")==0){
		offset = offsetof(part_type, state);
		*format = 3;
	}
	else if (strcmp(key, "properties")==0){
		offset = offsetof(part_type, properties);
		*format = 0;
		if (modified_stuff)
			*modified_stuff |= LUACON_EL_MODIFIED_GRAPHICS | LUACON_EL_MODIFIED_CANMOVE;
	}
	else if (strcmp(key, "description")==0){
		offset = offsetof(part_type, descs);
		*format = 2;
	}
	else {
		return -1;
	}
	return offset;
}
int luacon_elementread(lua_State* l){
	int format, offset;
	char * tempstring;
	int tempinteger;
	float tempfloat;
	int i;
	char * key = mystrdup((char*)luaL_optstring(l, 2, ""));
	offset = luacon_element_getproperty(key, &format, NULL);
	free(key);
	
	//Get Raw Index value for element
	lua_pushstring(l, "id");
	lua_rawget(l, 1);
	
	i = lua_tointeger (l, lua_gettop(l));
	
	lua_pop(l, 1);
	
	if(i < 0 || i >= PT_NUM || offset==-1)
	{
		return luaL_error(l, "Invalid property");
	}
	switch(format)
	{
	case 0:
		tempinteger = *((int*)(((char*)&ptypes[i])+offset));
		lua_pushnumber(l, tempinteger);
		break;
	case 1:
		tempfloat = *((float*)(((char*)&ptypes[i])+offset));
		lua_pushnumber(l, tempfloat);
		break;
	case 2:
		tempstring = *((char**)(((char*)&ptypes[i])+offset));
		lua_pushstring(l, tempstring);
		break;
	case 3:
		tempinteger = *((unsigned char*)(((char*)&ptypes[i])+offset));
		lua_pushnumber(l, tempinteger);
		break;
	}
	return 1;
}
int luacon_elementwrite(lua_State* l){
	int format, offset;
	char * tempstring;
	int tempinteger;
	float tempfloat;
	int i;
	unsigned int modified_stuff = 0;
	char * key = mystrdup((char*)luaL_optstring(l, 2, ""));
	offset = luacon_element_getproperty(key, &format, &modified_stuff);
	
	//Get Raw Index value for element
	lua_pushstring(l, "id");
	lua_rawget(l, 1);
	
	i = lua_tointeger (l, lua_gettop(l));
	
	lua_pop(l, 1);
	
	if(i < 0 || i >= PT_NUM || offset==-1)
	{
		free(key);
		return luaL_error(l, "Invalid property");
	}
	switch(format)
	{
	case 0:
		*((int*)(((char*)&ptypes[i])+offset)) = luaL_optinteger(l, 3, 0);
		break;
	case 1:
		*((float*)(((char*)&ptypes[i])+offset)) = luaL_optnumber(l, 3, 0);
		break;
	case 2:
		tempstring = mystrdup((char*)luaL_optstring(l, 3, ""));
		if(strcmp(key, "name")==0)
		{
			int j = 0;
			//Convert to upper case
			for(j = 0; j < strlen(tempstring); j++)
				tempstring[j] = toupper(tempstring[j]);
			if(strlen(tempstring)>4)
			{
				free(tempstring);
				free(key);
				return luaL_error(l, "Name too long");
			}
			if(console_parse_type(tempstring, NULL, NULL))
			{
				free(tempstring);
				free(key);
				return luaL_error(l, "Name in use");
			}
		}
		*((char**)(((char*)&ptypes[i])+offset)) = tempstring;
		//Need some way of cleaning up previous values
		break;
	case 3:
		*((unsigned char*)(((char*)&ptypes[i])+offset)) = luaL_optinteger(l, 3, 0);
		break;
	}
	if (modified_stuff)
	{
		if (modified_stuff & LUACON_EL_MODIFIED_MENUS)
			menu_count();
		if (modified_stuff & LUACON_EL_MODIFIED_CANMOVE)
			init_can_move();
		if (modified_stuff & LUACON_EL_MODIFIED_GRAPHICS)
			memset(graphicscache, 0, sizeof(gcache_item)*PT_NUM);
	}
	free(key);
	return 0;
}
int luacon_keyevent(int key, int modifier, int event){
	int i = 0, kpcontinue = 1;
	char tempkey[] = {key, 0};
	if(keypress_function_count){
		for(i = 0; i < keypress_function_count && kpcontinue; i++){
			lua_rawgeti(l, LUA_REGISTRYINDEX, keypress_functions[i]);
			lua_pushstring(l, tempkey);
			lua_pushinteger(l, key);
			lua_pushinteger(l, modifier);
			lua_pushinteger(l, event);
			lua_pcall(l, 4, 1, 0);
			if(lua_isboolean(l, -1)){
				kpcontinue = lua_toboolean(l, -1);
			}
			lua_pop(l, 1);
		}
	}
	return kpcontinue;
}
int luacon_mouseevent(int mx, int my, int mb, int event, int mouse_wheel){
	int i = 0, mpcontinue = 1;
	if(mouseclick_function_count){
		for(i = 0; i < mouseclick_function_count && mpcontinue; i++){
			lua_rawgeti(l, LUA_REGISTRYINDEX, mouseclick_functions[i]);
			lua_pushinteger(l, mx);
			lua_pushinteger(l, my);
			lua_pushinteger(l, mb);
			lua_pushinteger(l, event);
			lua_pushinteger(l, mouse_wheel);
			lua_pcall(l, 5, 1, 0);
			if(lua_isboolean(l, -1)){
				mpcontinue = lua_toboolean(l, -1);
			}
			lua_pop(l, 1);
		}
	}
	return mpcontinue;
}
int luacon_step(int mx, int my, int selectl, int selectr, int bsx, int bsy){
	int tempret = 0, tempb, i, callret;
	lua_pushinteger(l, bsy);
	lua_pushinteger(l, bsx);
	lua_pushinteger(l, SLALT);
	lua_pushinteger(l, selectr);
	lua_pushinteger(l, selectl);
	lua_pushinteger(l, my);
	lua_pushinteger(l, mx);
	lua_setfield(l, tptProperties, "mousex");
	lua_setfield(l, tptProperties, "mousey");
	lua_setfield(l, tptProperties, "selectedl");
	lua_setfield(l, tptProperties, "selectedr");
	lua_setfield(l, tptProperties, "selecteda");
	lua_setfield(l, tptProperties, "brushx");
	lua_setfield(l, tptProperties, "brushy");
	for(i = 0; i<6; i++){
		if(step_functions[i]){
			loop_time = SDL_GetTicks();
			lua_rawgeti(l, LUA_REGISTRYINDEX, step_functions[i]);
			callret = lua_pcall(l, 0, 0, 0);
			if (callret)
			{
				// failed, TODO: better error reporting
				printf("%s\n",luacon_geterror());
				if (!strcmp(luacon_geterror(),"Error: Infinite loop"))
				{
					lua_pushcfunction(l,&luatpt_unregister_step);
					lua_rawgeti(l, LUA_REGISTRYINDEX, step_functions[i]);
					lua_pcall(l, 1, 0, 0);
				}
			}
		}
	}
	return 0;
}
int luacon_eval(char *command){
	loop_time = SDL_GetTicks();
	return luaL_dostring (l, command);
}
void lua_hook(lua_State *L, lua_Debug *ar)
{
	if(ar->event == LUA_HOOKCOUNT && SDL_GetTicks()-loop_time > 3000)
	{
		if (confirm_ui(vid_buf,"Infinite Loop","The Lua code might have an infinite loop. Press OK to stop it","OK"))
			luaL_error(l,"Error: Infinite loop");
		loop_time = SDL_GetTicks();
	}
}
int luacon_part_update(int t, int i, int x, int y, int surround_space, int nt)
{
	int retval = 0;
	if(lua_el_func[t]){
		lua_rawgeti(l, LUA_REGISTRYINDEX, lua_el_func[t]);
		lua_pushinteger(l, i);
		lua_pushinteger(l, x);
		lua_pushinteger(l, y);
		lua_pushinteger(l, surround_space);
		lua_pushinteger(l, nt);
		lua_pcall(l, 5, 1, 0);
		if(lua_isboolean(l, -1)){
			retval = lua_toboolean(l, -1);
		}
		lua_pop(l, 1);
	}
	return retval;
}
int luacon_graphics_update(int t, int i, int *pixel_mode, int *cola, int *colr, int *colg, int *colb, int *firea, int *firer, int *fireg, int *fireb)
{
	int cache = 0;
	lua_rawgeti(l, LUA_REGISTRYINDEX, lua_gr_func[t]);
	lua_pushinteger(l, i);
	lua_pushinteger(l, *colr);
	lua_pushinteger(l, *colg);
	lua_pushinteger(l, *colb);
	lua_pcall(l, 4, 10, 0);

	cache = luaL_optint(l, 2, 0);
	*pixel_mode = luaL_optint(l, 3, *pixel_mode);
	*cola = luaL_optint(l, 4, *cola);
	*colr = luaL_optint(l, 5, *colr);
	*colg = luaL_optint(l, 6, *colg);
	*colb = luaL_optint(l, 7, *colb);
	*firea = luaL_optint(l, 8, *firea);
	*firer = luaL_optint(l, 9, *firer);
	*fireg = luaL_optint(l, 10, *fireg);
	*fireb = luaL_optint(l, 11, *fireb);
	lua_pop(l, 10);
	return cache;
}
char *luacon_geterror(){
	char *error = (char*)lua_tostring(l, -1);
	if(error==NULL || !error[0]){
		error = "failed to execute";
	}
	return error;
}
void luacon_close(){
	lua_close(l);
}
int process_command_lua(pixel *vid_buf, char *console, char *console_error)
{
	int commandret;
	char * tmp_error;
	char console2[15];
	char console3[15];
	char console4[15];
	char console5[15];
	//sprintf(console_error, "%s", console);
	if (console && strcmp(console, "")!=0 && strncmp(console, " ", 1)!=0)
	{
		sscanf(console,"%14s %14s %14s %14s", console2, console3, console4, console5);
		if (strcmp(console2, "quit")==0)
		{
			return -1;
		}
		else if(strncmp(console, "!", 1)==0)
		{
			return process_command_old(vid_buf, console+1, console_error);
		}
		else
		{
			commandret = luacon_eval(console);
			if (commandret){
				tmp_error = luacon_geterror();
				strncpy(console_error, tmp_error, 254);
				printf("%s\n", tmp_error);
			}
		}
	}
	return 1;
}
//Being TPT interface methods:
int luatpt_test(lua_State* l)
{
    int testint = 0;
	testint = luaL_optint(l, 1, 0);
	printf("Test successful, got %d\n", testint);
	return 0;
}
int luatpt_getelement(lua_State *l)
{
	int t;
	char * name = (char*)luaL_optstring(l, 1, "dust");
	if (!console_parse_type(name, &t, NULL))
		return luaL_error(l,"Unrecognised element '%s'", name);
	lua_pushinteger(l, t);
	return 1;
}
int luatpt_element_func(lua_State *l)
{
	if(lua_isfunction(l, 1))
	{
		int element = luaL_optint(l, 2, 0);
		int replace = luaL_optint(l, 3, 0);
		int function;
		lua_pushvalue(l, 1);
		function = luaL_ref(l, LUA_REGISTRYINDEX);
		if(element > 0 && element < PT_NUM)
		{
			lua_el_func[element] = function;
			if(replace)
				lua_el_mode[element] = 2;
			else
				lua_el_mode[element] = 1;
			return 0;
		}
		else
		{
			return luaL_error(l, "Invalid element");
		}
	}
	return 0;
}
int luatpt_graphics_func(lua_State *l)
{
	if(lua_isfunction(l, 1))
	{
		int element = luaL_optint(l, 2, 0);
		int function;
		lua_pushvalue(l, 1);
		function = luaL_ref(l, LUA_REGISTRYINDEX);
		if(element > 0 && element < PT_NUM)
		{
			lua_gr_func[element] = function;
			graphicscache[element].isready = 0;
			return 0;
		}
		else
		{
			return luaL_error(l, "Invalid element");
		}
	}
	else
		return luaL_error(l, "Not a function");
	return 0;
}
int luatpt_error(lua_State* l)
{
	char *error = "";
	error = mystrdup((char*)luaL_optstring(l, 1, "Error text"));
	if(vid_buf!=NULL){
		error_ui(vid_buf, 0, error);
		free(error);
		return 0;
	}
	free(error);
	return luaL_error(l, "Screen buffer does not exist");
}
int luatpt_drawtext(lua_State* l)
{
    char *string;
	int textx, texty, textred, textgreen, textblue, textalpha;
	textx = luaL_optint(l, 1, 0);
	texty = luaL_optint(l, 2, 0);
	string = (char*)luaL_optstring(l, 3, "");
	textred = luaL_optint(l, 4, 255);
	textgreen = luaL_optint(l, 5, 255);
	textblue = luaL_optint(l, 6, 255);
	textalpha = luaL_optint(l, 7, 255);
	if (textx<0 || texty<0 || textx>=XRES+BARSIZE || texty>=YRES+MENUSIZE)
		return luaL_error(l, "Screen coordinates out of range (%d,%d)", textx, texty);
	if (textred<0) textred = 0;
	if (textred>255) textred = 255;
	if (textgreen<0) textgreen = 0;
	if (textgreen>255) textgreen = 255;
	if (textblue<0) textblue = 0;
	if (textblue>255) textblue = 255;
	if (textalpha<0) textalpha = 0;
	if (textalpha>255) textalpha = 255;
	if(vid_buf!=NULL){
		drawtext(vid_buf, textx, texty, string, textred, textgreen, textblue, textalpha);
		return 0;
	}
	return luaL_error(l, "Screen buffer does not exist");
}

int luatpt_create(lua_State* l)
{
	int x, y, retid, t = -1;
	char * name;
	x = abs(luaL_optint(l, 1, 0));
	y = abs(luaL_optint(l, 2, 0));
	if(x < XRES && y < YRES){
		if(lua_isnumber(l, 3)){
			t = luaL_optint(l, 3, 0);
			if (t<0 || t >= PT_NUM || !ptypes[t].enabled)
				return luaL_error(l, "Unrecognised element number '%d'", t);
		} else {
			name = (char*)luaL_optstring(l, 3, "dust");
			if (!console_parse_type(name, &t, NULL))
				return luaL_error(l,"Unrecognised element '%s'", name);
		}
		retid = create_part(-1, x, y, t);
		// failing to create a particle often happens (e.g. if space is already occupied) and isn't usually important, so don't raise an error
		lua_pushinteger(l, retid);
		return 1;
	}
	return luaL_error(l, "Coordinates out of range (%d,%d)", x, y);
}

int luatpt_setpause(lua_State* l)
{
	int pausestate;
	pausestate = luaL_optint(l, 1, -1);
	if (pausestate == -1)
	{
		lua_pushnumber(l, sys_pause);
		return 1;
	}
	sys_pause = (pausestate==0?0:1);
	return 0;
}

int luatpt_togglepause(lua_State* l)
{
	sys_pause=!sys_pause;
	lua_pushnumber(l, sys_pause);
	return 1;
}

int luatpt_togglewater(lua_State* l)
{
	water_equal_test=!water_equal_test;
	lua_pushnumber(l, water_equal_test);
	return 1;
}

int luatpt_setconsole(lua_State* l)
{
	int consolestate;
	consolestate = luaL_optint(l, 1, -1);
	if (consolestate == -1)
	{
		lua_pushnumber(l, console_mode);
		return 1;
	}
	console_mode = (consolestate==0?0:1);
	return 0;
}

int luatpt_log(lua_State* l)
{
	char *buffer;
	buffer = (char*)luaL_optstring(l, 1, "");
	strncpy(console_error, buffer, 254);
	return 0;
}

void set_map(int x, int y, int width, int height, float value, int map)
{
	int nx, ny;
	if(x > (XRES/CELL)-1)
		x = (XRES/CELL)-1;
	if(y > (YRES/CELL)-1)
		y = (YRES/CELL)-1;
	if(x+width > (XRES/CELL)-1)
		width = (XRES/CELL)-x;
	if(y+height > (YRES/CELL)-1)
		height = (YRES/CELL)-y;
	for (nx = x; nx<x+width; nx++)
		for (ny = y; ny<y+height; ny++)
		{
			if (map == 1)
				pv[ny][nx] = value;
			else if (map == 2)
				hv[ny][nx] = value;
			else if (map == 3)
				vx[ny][nx] = value;
			else if (map == 4)
				vy[ny][nx] = value;
			else
				gravmap[ny*(XRES/CELL)+nx] = value; //TODO: setting gravity setting doesn't work anymore?

		}
}

int luatpt_set_pressure(lua_State* l)
{
	int x, y, width, height;
	float value;
	x = abs(luaL_optint(l, 1, 0));
	y = abs(luaL_optint(l, 2, 0));
	width = abs(luaL_optint(l, 3, XRES/CELL));
	height = abs(luaL_optint(l, 4, YRES/CELL));
	value = (float)luaL_optnumber(l, 5, 0);
	if(value > 256.0f)
		value = 256.0f;
	else if(value < -256.0f)
		value = -256.0f;

	set_map(x, y, width, height, value, 1);
	return 0;
}

int luatpt_set_aheat(lua_State* l)
{
	int x, y, width, height;
	float value;
	x = abs(luaL_optint(l, 1, 0));
	y = abs(luaL_optint(l, 2, 0));
	width = abs(luaL_optint(l, 3, XRES/CELL));
	height = abs(luaL_optint(l, 4, YRES/CELL));
	value = (float)luaL_optnumber(l, 5, 0);
	if(value > MAX_TEMP)
		value = MAX_TEMP;
	else if(value < MIN_TEMP)
		value = MIN_TEMP;

	set_map(x, y, width, height, value, 2);
	return 0;
}

int luatpt_set_velocity(lua_State* l)
{
	int x, y, width, height, dir;
	float value;
	char *direction = (char*)luaL_optstring(l, 1, "");
	x = abs(luaL_optint(l, 2, 0));
	y = abs(luaL_optint(l, 3, 0));
	width = abs(luaL_optint(l, 4, XRES/CELL));
	height = abs(luaL_optint(l, 5, YRES/CELL));
	value = (float)luaL_optnumber(l, 6, 0);
	if(value > 256.0f)
		value = 256.0f;
	else if(value < -256.0f)
		value = -256.0f;

	if (strcmp(direction,"x")==0)
		set_map(x, y, width, height, value, 3);
	else if (strcmp(direction,"y")==0)
		set_map(x, y, width, height, value, 4);
	else
		return luaL_error(l, "Invalid direction: %s", direction);
	return 0;
}

int luatpt_set_gravity(lua_State* l)
{
	int x, y, width, height;
	float value;
	x = abs(luaL_optint(l, 1, 0));
	y = abs(luaL_optint(l, 2, 0));
	width = abs(luaL_optint(l, 3, XRES/CELL));
	height = abs(luaL_optint(l, 4, YRES/CELL));
	value = (float)luaL_optnumber(l, 5, 0);

	set_map(x, y, width, height, value, 5);
	return 0;
}

int luatpt_reset_gravity_field(lua_State* l)
{
	int nx, ny;
	int x1, y1, width, height;
	x1 = abs(luaL_optint(l, 1, 0));
	y1 = abs(luaL_optint(l, 2, 0));
	width = abs(luaL_optint(l, 3, XRES/CELL));
	height = abs(luaL_optint(l, 4, YRES/CELL));
	if(x1 > (XRES/CELL)-1)
		x1 = (XRES/CELL)-1;
	if(y1 > (YRES/CELL)-1)
		y1 = (YRES/CELL)-1;
	if(x1+width > (XRES/CELL)-1)
		width = (XRES/CELL)-x1;
	if(y1+height > (YRES/CELL)-1)
		height = (YRES/CELL)-y1;
	for (nx = x1; nx<x1+width; nx++)
		for (ny = y1; ny<y1+height; ny++)
		{
			gravx[ny*(XRES/CELL)+nx] = 0;
			gravy[ny*(XRES/CELL)+nx] = 0;
			gravp[ny*(XRES/CELL)+nx] = 0;
		}
	return 0;
}

int luatpt_reset_velocity(lua_State* l)
{
	int nx, ny;
	int x1, y1, width, height;
	x1 = abs(luaL_optint(l, 1, 0));
	y1 = abs(luaL_optint(l, 2, 0));
	width = abs(luaL_optint(l, 3, XRES/CELL));
	height = abs(luaL_optint(l, 4, YRES/CELL));
	if(x1 > (XRES/CELL)-1)
		x1 = (XRES/CELL)-1;
	if(y1 > (YRES/CELL)-1)
		y1 = (YRES/CELL)-1;
	if(x1+width > (XRES/CELL)-1)
		width = (XRES/CELL)-x1;
	if(y1+height > (YRES/CELL)-1)
		height = (YRES/CELL)-y1;
	for (nx = x1; nx<x1+width; nx++)
		for (ny = y1; ny<y1+height; ny++)
		{
			vx[ny][nx] = 0;
			vy[ny][nx] = 0;
		}
	return 0;
}

int luatpt_reset_spark(lua_State* l)
{
	int i;
	for (i=0; i<NPART; i++)
	{
		if (parts[i].type==PT_SPRK)
		{
			if (parts[i].ctype >= 0 && parts[i].ctype < PT_NUM)
			{
				parts[i].type = parts[i].ctype;
				parts[i].life = 0;
			}
			else
				kill_part(i);
		}
	}
	return 0;
}

int luatpt_set_property(lua_State* l)
{
	char *prop, *name;
	int r, i, x, y, w, h, t, format, nx, ny, partsel = 0, acount;
	float f;
	size_t offset;
	acount = lua_gettop(l);
	prop = (char*)luaL_optstring(l, 1, "");
	if(lua_isnumber(l, 3))
		i = abs(luaL_optint(l, 3, -1));
	else
		i = -1;
	if(lua_isnumber(l, 4))
		y = abs(luaL_optint(l, 4, -1));
	else
		y = -1;
	if(lua_isnumber(l, 5))
		w = abs(luaL_optint(l, 5, -1));
	else
		w = -1;
	if(lua_isnumber(l, 6))
		h = abs(luaL_optint(l, 6, -1));
	else
		h = -1;
	//TODO: Use particle_getproperty
	if (strcmp(prop,"type")==0){
		offset = offsetof(particle, type);
		format = 3;
	} else if (strcmp(prop,"life")==0){
		offset = offsetof(particle, life);
		format = 1;
	} else if (strcmp(prop,"ctype")==0){
		offset = offsetof(particle, ctype);
		format = 4;
	} else if (strcmp(prop,"temp")==0){
		offset = offsetof(particle, temp);
		format = 2;
	} else if (strcmp(prop,"tmp")==0){
		offset = offsetof(particle, tmp);
		format = 1;
	} else if (strcmp(prop,"tmp2")==0){
		offset = offsetof(particle, tmp2);
		format = 1;
	} else if (strcmp(prop,"vy")==0){
		offset = offsetof(particle, vy);
		format = 2;
	} else if (strcmp(prop,"vx")==0){
		offset = offsetof(particle, vx);
		format = 2;
	} else if (strcmp(prop,"x")==0){
		offset = offsetof(particle, x);
		format = 2;
	} else if (strcmp(prop,"y")==0){
		offset = offsetof(particle, y);
		format = 2;
	} else if (strcmp(prop,"dcolour")==0){
		offset = offsetof(particle, dcolour);
		format = 1;
	} else {
		return luaL_error(l, "Invalid property '%s'", prop);
	}
	if(acount>2){
		if(!lua_isnumber(l, acount) && lua_isstring(l, acount)){
			name = (char*)luaL_optstring(l, acount, "none");
			if (!console_parse_type(name, &partsel, NULL))
				return luaL_error(l, "Unrecognised element '%s'", name);
		}
	}
	if(lua_isnumber(l, 2)){
		if(format==2){
			f = luaL_optnumber(l, 2, 0);
		} else {
			t = luaL_optint(l, 2, 0);
		}
		if (format == 3 && (t<0 || t>=PT_NUM || !ptypes[t].enabled))
			return luaL_error(l, "Unrecognised element number '%d'", t);
	} else {
		name = (char*)luaL_optstring(l, 2, "dust");
		if (!console_parse_type(name, &t, NULL))
			return luaL_error(l, "Unrecognised element '%s'", name);
	}
	if(i == -1 || (w != -1 && h != -1)){
		// Got a region
		if(i == -1){
			i = 0;
			y = 0;
			w = XRES;
			h = YRES;
		}
		if (i>=XRES || y>=YRES)
			return luaL_error(l, "Coordinates out of range (%d,%d)", i, y);
		x = i;
		if(x+w > XRES)
			w = XRES-x;
		if(y+h > YRES)
			h = YRES-y;
		for (nx = x; nx<x+w; nx++)
			for (ny = y; ny<y+h; ny++){
				r = pmap[ny][nx];
				if (!r || (partsel && partsel != parts[r>>8].type))
				{
					r = photons[ny][nx];
					if (!r || (partsel && partsel != parts[r>>8].type))
						continue;
				}
				i = r>>8;
				if(format==2){
					*((float*)(((char*)&parts[i])+offset)) = f;
				} else {
					*((int*)(((char*)&parts[i])+offset)) = t;
				}
			}
	} else {
		// Got coords or particle index
		if(i != -1 && y != -1){
			if (i>=XRES || y>=YRES)
				return luaL_error(l, "Coordinates out of range (%d,%d)", i, y);
			r = pmap[y][i];
			if (!r || (partsel && partsel != parts[r>>8].type))
				r = photons[y][i];
			if (!r || (partsel && partsel != parts[r>>8].type))
				return 0;
			i = r>>8;
		}
		if (i < 0 || i >= NPART)
			return luaL_error(l, "Invalid particle ID '%d'", i);
		if (!parts[i].type)
			return 0;
		if (partsel && partsel != parts[i].type)
			return 0;
		if(format==2){
			*((float*)(((char*)&parts[i])+offset)) = f;
		} else {
			*((int*)(((char*)&parts[i])+offset)) = t;
		}
	}
	return 0;
}

int luatpt_get_property(lua_State* l)
{
	int i, r, y;
	char *prop;
	prop = (char*)luaL_optstring(l, 1, "");
	i = luaL_optint(l, 2, 0);
	y = luaL_optint(l, 3, -1);
	if(y!=-1 && y < YRES && y >= 0 && i < XRES && i >= 0){
		r = pmap[y][i];
		if (!r)
			r = photons[y][i];
		if (!r)
		{
			if (strcmp(prop,"type")==0){
				lua_pushinteger(l, 0);
				return 1;
			}
			return luaL_error(l, "Particle does not exist");
		}
		i = r>>8;
	}
	else if (y!=-1)
		return luaL_error(l, "Coordinates out of range (%d,%d)", i, y);
	if (i < 0 || i >= NPART)
		return luaL_error(l, "Invalid particle ID '%d'", i);
	if (parts[i].type)
	{
		//TODO: Use particle_getproperty
		if (strcmp(prop,"type")==0){
			lua_pushinteger(l, parts[i].type);
			return 1;
		}
		if (strcmp(prop,"life")==0){
			lua_pushinteger(l, parts[i].life);
			return 1;
		}
		if (strcmp(prop,"ctype")==0){
			lua_pushinteger(l, parts[i].ctype);
			return 1;
		}
		if (strcmp(prop,"temp")==0){
			lua_pushnumber(l, parts[i].temp);
			return 1;
		}
		if (strcmp(prop,"tmp")==0){
			lua_pushinteger(l, parts[i].tmp);
			return 1;
		}
		if (strcmp(prop,"tmp2")==0){
			lua_pushinteger(l, parts[i].tmp2);
			return 1;
		}
		if (strcmp(prop,"vy")==0){
			lua_pushnumber(l, (double)parts[i].vy);
			return 1;
		}
		if (strcmp(prop,"vx")==0){
			lua_pushnumber(l, (double)parts[i].vx);
			return 1;
		}
		if (strcmp(prop,"x")==0){
			lua_pushnumber(l, parts[i].x);
			return 1;
		}
		if (strcmp(prop,"y")==0){
			lua_pushnumber(l, parts[i].y);
			return 1;
		}
		if (strcmp(prop,"dcolour")==0){
			lua_pushinteger(l, parts[i].dcolour);
			return 1;
		}
		if (strcmp(prop,"id")==0){
			lua_pushnumber(l, i);
			return 1;
		}
	}
	else if (strcmp(prop,"type")==0){
		lua_pushinteger(l, 0);
		return 1;
	}
	return luaL_error(l, "Particle does not exist");
}

int luatpt_drawpixel(lua_State* l)
{
	int x, y, r, g, b, a;
	x = luaL_optint(l, 1, 0);
	y = luaL_optint(l, 2, 0);
	r = luaL_optint(l, 3, 255);
	g = luaL_optint(l, 4, 255);
	b = luaL_optint(l, 5, 255);
	a = luaL_optint(l, 6, 255);

	if (x<0 || y<0 || x>=XRES+BARSIZE || y>=YRES+MENUSIZE)
		return luaL_error(l, "Screen coordinates out of range (%d,%d)", x, y);
	if (r<0) r = 0;
	if (r>255) r = 255;
	if (g<0) g = 0;
	if (g>255) g = 255;
	if (b<0) b = 0;
	if (b>255) b = 255;
	if (a<0) a = 0;
	if (a>255) a = 255;
	if (vid_buf!=NULL)
	{
		drawpixel(vid_buf, x, y, r, g, b, a);
		return 0;
	}
	return luaL_error(l, "Screen buffer does not exist");
}

int luatpt_drawrect(lua_State* l)
{
	int x, y, w, h, r, g, b, a;
	x = luaL_optint(l, 1, 0);
	y = luaL_optint(l, 2, 0);
	w = luaL_optint(l, 3, 10);
	h = luaL_optint(l, 4, 10);
	r = luaL_optint(l, 5, 255);
	g = luaL_optint(l, 6, 255);
	b = luaL_optint(l, 7, 255);
	a = luaL_optint(l, 8, 255);

	if (x<0 || y<0 || x>=XRES+BARSIZE || y>=YRES+MENUSIZE)
		return luaL_error(l, "Screen coordinates out of range (%d,%d)", x, y);
	if(x+w > XRES+BARSIZE)
		w = XRES+BARSIZE-x;
	if(y+h > YRES+MENUSIZE)
		h = YRES+MENUSIZE-y;
	if (r<0) r = 0;
	if (r>255) r = 255;
	if (g<0) g = 0;
	if (g>255) g = 255;
	if (b<0) b = 0;
	if (b>255) b = 255;
	if (a<0) a = 0;
	if (a>255) a = 255;
	if (vid_buf!=NULL)
	{
		drawrect(vid_buf, x, y, w, h, r, g, b, a);
		return 0;
	}
	return luaL_error(l, "Screen buffer does not exist");
}

int luatpt_fillrect(lua_State* l)
{
	int x,y,w,h,r,g,b,a;
	x = luaL_optint(l, 1, 0);
	y = luaL_optint(l, 2, 0);
	w = luaL_optint(l, 3, 10);
	h = luaL_optint(l, 4, 10);
	r = luaL_optint(l, 5, 255);
	g = luaL_optint(l, 6, 255);
	b = luaL_optint(l, 7, 255);
	a = luaL_optint(l, 8, 255);

	if (x<0 || y<0 || x>=XRES+BARSIZE || y>=YRES+MENUSIZE)
		return luaL_error(l, "Screen coordinates out of range (%d,%d)", x, y);
	if(x+w > XRES+BARSIZE)
		w = XRES+BARSIZE-x;
	if(y+h > YRES+MENUSIZE)
		h = YRES+MENUSIZE-y;
	if (r<0) r = 0;
	if (r>255) r = 255;
	if (g<0) g = 0;
	if (g>255) g = 255;
	if (b<0) b = 0;
	if (b>255) b = 255;
	if (a<0) a = 0;
	if (a>255) a = 255;
	if (vid_buf!=NULL)
	{
		fillrect(vid_buf, x, y, w, h, r, g, b, a);
		return 0;
	}
	return luaL_error(l, "Screen buffer does not exist");
}

int luatpt_drawcircle(lua_State* l)
{
	int x, y, rx, ry, r, g, b, a;
	x = luaL_optint(l, 1, 0);
	y = luaL_optint(l, 2, 0);
	rx = luaL_optint(l, 3, 10);
	ry = luaL_optint(l, 4, 10);
	r = luaL_optint(l, 5, 255);
	g = luaL_optint(l, 6, 255);
	b = luaL_optint(l, 7, 255);
	a = luaL_optint(l, 8, 255);

	if (x<0 || y<0 || x>=XRES+BARSIZE || y>=YRES+MENUSIZE)
		return luaL_error(l, "Screen coordinates out of range (%d,%d)", x, y);
	if (r<0) r = 0;
	if (r>255) r = 255;
	if (g<0) g = 0;
	if (g>255) g = 255;
	if (b<0) b = 0;
	if (b>255) b = 255;
	if (a<0) a = 0;
	if (a>255) a = 255;
	if (vid_buf!=NULL)
	{
		drawcircle(vid_buf, x, y, rx, ry, r, g, b, a);
		return 0;
	}
	return luaL_error(l, "Screen buffer does not exist");
}

int luatpt_fillcircle(lua_State* l)
{
	int x,y,rx,ry,r,g,b,a;
	x = luaL_optint(l, 1, 0);
	y = luaL_optint(l, 2, 0);
	rx = luaL_optint(l, 3, 10);
	ry = luaL_optint(l, 4, 10);
	r = luaL_optint(l, 5, 255);
	g = luaL_optint(l, 6, 255);
	b = luaL_optint(l, 7, 255);
	a = luaL_optint(l, 8, 255);

	if (x<0 || y<0 || x>=XRES+BARSIZE || y>=YRES+MENUSIZE)
		return luaL_error(l, "Screen coordinates out of range (%d,%d)", x, y);
	if (r<0) r = 0;
	if (r>255) r = 255;
	if (g<0) g = 0;
	if (g>255) g = 255;
	if (b<0) b = 0;
	if (b>255) b = 255;
	if (a<0) a = 0;
	if (a>255) a = 255;
	if (vid_buf!=NULL)
	{
		fillcircle(vid_buf, x, y, rx, ry, r, g, b, a);
		return 0;
	}
	return luaL_error(l, "Screen buffer does not exist");
}

int luatpt_drawline(lua_State* l)
{
	int x1,y1,x2,y2,r,g,b,a;
	x1 = luaL_optint(l, 1, 0);
	y1 = luaL_optint(l, 2, 0);
	x2 = luaL_optint(l, 3, 10);
	y2 = luaL_optint(l, 4, 10);
	r = luaL_optint(l, 5, 255);
	g = luaL_optint(l, 6, 255);
	b = luaL_optint(l, 7, 255);
	a = luaL_optint(l, 8, 255);

	//Don't need to check coordinates, as they are checked in blendpixel
	if (r<0) r = 0;
	if (r>255) r = 255;
	if (g<0) g = 0;
	if (g>255) g = 255;
	if (b<0) b = 0;
	if (b>255) b = 255;
	if (a<0) a = 0;
	if (a>255) a = 255;
	if (vid_buf!=NULL)
	{
		blend_line(vid_buf, x1, y1, x2, y2, r, g, b, a);
		return 0;
	}
	return luaL_error(l, "Screen buffer does not exist");
}

int luatpt_textwidth(lua_State* l)
{
	char * string;
	int strwidth = 0;
	string = (char*)luaL_optstring(l, 1, "");
	strwidth = textwidth(string);
	lua_pushinteger(l, strwidth);
	return 1;
}

int luatpt_get_name(lua_State* l)
{
	if (svf_login){
		lua_pushstring(l, svf_user);
		return 1;
	}
	lua_pushstring(l, "");
	return 1;
}

int luatpt_set_shortcuts(lua_State* l)
{
	int state;
	state = luaL_optint(l, 1, -1);
	if (state == -1)
	{
		lua_pushnumber(l, sys_shortcuts);
		return 1;
	}
	sys_shortcuts = (state==0?0:1);
	return 0;
}

int luatpt_delete(lua_State* l)
{
	int arg1, arg2;
	arg1 = abs(luaL_optint(l, 1, 0));
	arg2 = luaL_optint(l, 2, -1);
	if(arg2 == -1 && arg1 < NPART){
		kill_part(arg1);
		return 0;
	}
	arg2 = abs(arg2);
	if(arg2 < YRES && arg1 < XRES){
		delete_part(arg1, arg2, 0);
		return 0;
	}
	return luaL_error(l,"Invalid coordinates or particle ID");
}

int luatpt_register_step(lua_State* l)
{
	int ref, i, ifree = -1;
	if(lua_isfunction(l, 1)){
		for(i = 0; i<6; i++){
			if(!step_functions[i]){
				if (ifree<0) ifree = i;
			} else {
				lua_rawgeti(l, LUA_REGISTRYINDEX, step_functions[i]);
				if(lua_equal(l, 1, lua_gettop(l))){
					lua_pop(l, 1);
					return luaL_error(l, "Function already registered");
				}
				lua_pop(l, 1);
			}
		}
		if (ifree>=0)
		{
			ref = luaL_ref(l, LUA_REGISTRYINDEX);
			step_functions[ifree] = ref;
			return 0;
		}
		else return luaL_error(l, "Step function limit reached");
	}
	return 0;
}
int luatpt_unregister_step(lua_State* l)
{
	int i;
	if(lua_isfunction(l, 1)){
		for(i = 0; i<6; i++){
			if (step_functions[i]){
				lua_rawgeti(l, LUA_REGISTRYINDEX, step_functions[i]);
				if(lua_equal(l, 1, lua_gettop(l))){
					lua_pop(l, 1);
					luaL_unref(l, LUA_REGISTRYINDEX, step_functions[i]);
					step_functions[i] = 0;
				}
				else lua_pop(l, 1);
			}
		}
	}
	return 0;
}
int luatpt_register_keypress(lua_State* l)
{
	int *newfunctions, i;
	if(lua_isfunction(l, 1)){
		for(i = 0; i<keypress_function_count; i++){
			lua_rawgeti(l, LUA_REGISTRYINDEX, keypress_functions[i]);
			if(lua_equal(l, 1, lua_gettop(l))){
				lua_pop(l, 1);
				return luaL_error(l, "Function already registered");
			}
			lua_pop(l, 1);
		}
		newfunctions = calloc(keypress_function_count+1, sizeof(int));
		if(keypress_functions){
			memcpy(newfunctions, keypress_functions, keypress_function_count*sizeof(int));
			free(keypress_functions);
		}
		newfunctions[keypress_function_count] = luaL_ref(l, LUA_REGISTRYINDEX);
		keypress_function_count++;
		keypress_functions = newfunctions;
	}
	return 0;
}
int luatpt_unregister_keypress(lua_State* l)
{
	int *newfunctions, i, functionindex = -1;
	if(lua_isfunction(l, 1)){
		for(i = 0; i<keypress_function_count; i++){
			lua_rawgeti(l, LUA_REGISTRYINDEX, keypress_functions[i]);
			if(lua_equal(l, 1, lua_gettop(l))){
				functionindex = i;
			}
			lua_pop(l, 1);
		}
	}
	if(functionindex != -1){
		luaL_unref(l, LUA_REGISTRYINDEX, keypress_functions[functionindex]);
		if(functionindex != keypress_function_count-1){
			memmove(keypress_functions+functionindex+1, keypress_functions+functionindex+1, (keypress_function_count-functionindex-1)*sizeof(int));
		}
		if(keypress_function_count-1 > 0){
			newfunctions = calloc(keypress_function_count-1, sizeof(int));
			memcpy(newfunctions, keypress_functions, (keypress_function_count-1)*sizeof(int));
			free(keypress_functions);
			keypress_functions = newfunctions;
		} else {
			free(keypress_functions);
			keypress_functions = NULL;
		}
		keypress_function_count--;
	} else {
		return luaL_error(l, "Function not registered");
	}
	return 0;
}
int luatpt_register_mouseclick(lua_State* l)
{
	int *newfunctions, i;
	if(lua_isfunction(l, 1)){
		for(i = 0; i<mouseclick_function_count; i++){
			lua_rawgeti(l, LUA_REGISTRYINDEX, mouseclick_functions[i]);
			if(lua_equal(l, 1, lua_gettop(l))){
				lua_pop(l, 1);
				return luaL_error(l, "Function already registered");
			}
			lua_pop(l, 1);
		}
		newfunctions = calloc(mouseclick_function_count+1, sizeof(int));
		if(mouseclick_functions){
			memcpy(newfunctions, mouseclick_functions, mouseclick_function_count*sizeof(int));
			free(mouseclick_functions);
		}
		newfunctions[mouseclick_function_count] = luaL_ref(l, LUA_REGISTRYINDEX);
		mouseclick_function_count++;
		mouseclick_functions = newfunctions;
	}
	return 0;
}
int luatpt_unregister_mouseclick(lua_State* l)
{
	int *newfunctions, i, functionindex = -1;
	if(lua_isfunction(l, 1)){
		for(i = 0; i<mouseclick_function_count; i++){
			lua_rawgeti(l, LUA_REGISTRYINDEX, mouseclick_functions[i]);
			if(lua_equal(l, 1, lua_gettop(l))){
				functionindex = i;
			}
			lua_pop(l, 1);
		}
	}
	if(functionindex != -1){
		luaL_unref(l, LUA_REGISTRYINDEX, mouseclick_functions[functionindex]);
		if(functionindex != mouseclick_function_count-1){
			memmove(mouseclick_functions+functionindex+1, mouseclick_functions+functionindex+1, (mouseclick_function_count-functionindex-1)*sizeof(int));
		}
		if(mouseclick_function_count-1 > 0){
			newfunctions = calloc(mouseclick_function_count-1, sizeof(int));
			memcpy(newfunctions, mouseclick_functions, (mouseclick_function_count-1)*sizeof(int));
			free(mouseclick_functions);
			mouseclick_functions = newfunctions;
		} else {
			free(mouseclick_functions);
			mouseclick_functions = NULL;
		}
		mouseclick_function_count--;
	} else {
		return luaL_error(l, "Function not registered");
	}
	return 0;
}
int luatpt_input(lua_State* l)
{
	char *prompt, *title, *result, *shadow, *text;
	title = mystrdup((char*)luaL_optstring(l, 1, "Title"));
	prompt = mystrdup((char*)luaL_optstring(l, 2, "Enter some text:"));
	text = mystrdup((char*)luaL_optstring(l, 3, ""));
	shadow = mystrdup((char*)luaL_optstring(l, 4, ""));

	if (vid_buf!=NULL)
	{
		result = input_ui(vid_buf, title, prompt, text, shadow);
		lua_pushstring(l, result);
		free(result);
		free(title);
		free(prompt);
		free(text);
		free(shadow);
		return 1;
	}
	free(title);
	free(prompt);
	free(text);
	free(shadow);
	return luaL_error(l, "Screen buffer does not exist");
}
int luatpt_message_box(lua_State* l)
{
	char *title, *text;
	title = mystrdup((char*)luaL_optstring(l, 1, "Title"));
	text = mystrdup((char*)luaL_optstring(l, 2, "Message"));
	if (vid_buf!=NULL)
	{
		info_ui(vid_buf, title, text);
		free(title);
		free(text);
		return 0;
	}
	free(title);
	free(text);
	return luaL_error(l, "Screen buffer does not exist");;
}
int luatpt_get_numOfParts(lua_State* l)
{
    lua_pushinteger(l, NUM_PARTS);
    return 1;
}
int luatpt_start_getPartIndex(lua_State* l)
{
    getPartIndex_curIdx = -1;
    return 1;
}
int luatpt_next_getPartIndex(lua_State* l)
{
    while(1)
    {
        getPartIndex_curIdx++;
        if(getPartIndex_curIdx >= NPART)
        {
            getPartIndex_curIdx = 0;
            lua_pushboolean(l, 0);
            return 1;
        }
        if(parts[getPartIndex_curIdx].type)
            break;

    }

    lua_pushboolean(l, 1);
    return 1;
}
int luatpt_getPartIndex(lua_State* l)
{
    if(getPartIndex_curIdx < 0)
    {
        lua_pushinteger(l, 0);
        return 1;
    }
    lua_pushinteger(l, getPartIndex_curIdx);
    return 1;
}
int luatpt_hud(lua_State* l)
{
    int hudstate;
    hudstate = luaL_optint(l, 1, -1);
	if (hudstate == -1)
	{
		lua_pushnumber(l, hud_enable);
		return 1;
	}
    hud_enable = (hudstate==0?0:1);
    return 0;
}
int luatpt_gravity(lua_State* l)
{
    int gravstate;
    gravstate = luaL_optint(l, 1, -1);
	if (gravstate == -1)
	{
		lua_pushnumber(l, ngrav_enable);
		return 1;
	}
    if(gravstate)
        start_grav_async();
    else
        stop_grav_async();
    ngrav_enable = (gravstate==0?0:1);
    return 0;
}
int luatpt_airheat(lua_State* l)
{
    int aheatstate;
    aheatstate = luaL_optint(l, 1, -1);
	if (aheatstate == -1)
	{
		lua_pushnumber(l, aheat_enable);
		return 1;
	}
    aheat_enable = (aheatstate==0?0:1);
    return 0;
}
int luatpt_active_menu(lua_State* l)
{
    int menuid;
    menuid = luaL_optint(l, 1, -1);
	if (menuid == -1)
	{
		lua_pushnumber(l, active_menu);
		return 1;
	}
    if (menuid < SC_TOTAL && menuid >= 0)
        active_menu = menuid;
    else
        return luaL_error(l, "Invalid menu");
    return 0;
}
int luatpt_decorations_enable(lua_State* l)
{
    int decostate;
    decostate = luaL_optint(l, 1, -1);
	if (decostate == -1)
	{
		lua_pushnumber(l, decorations_enable);
		return 1;
	}
    decorations_enable = (decostate==0?0:1);
    return 0;
}

int luatpt_heat(lua_State* l)
{
	int heatstate;
	heatstate = luaL_optint(l, 1, -1);
	if (heatstate == -1)
	{
		lua_pushnumber(l, !legacy_enable);
		return 1;
	}
	legacy_enable = (heatstate==1?0:1);
	return 0;
}
int luatpt_cmode_set(lua_State* l)
{
	int cmode = luaL_optint(l, 1, CM_FIRE);
	/*if (cmode == -1) //TODO: Functions to set render, display, & color modes
	{
		lua_pushnumber(l, ngrav_enable);
		return 1;
	}*/
	set_cmode(cmode);
	return 0;
}
int luatpt_setfire(lua_State* l)
{
	int firesize = luaL_optint(l, 2, 4);
	float fireintensity = (float)luaL_optnumber(l, 1, 1.0f);
	prepare_alpha(firesize, fireintensity);
	return 0;
}
int luatpt_setdebug(lua_State* l)
{
	int debug = luaL_optint(l, 1, -1);
	if (debug == -1)
	{
		lua_pushnumber(l, debug);
		return 1;
	}
	debug_flags = debug;
	return 0;
}
int luatpt_setfpscap(lua_State* l)
{
	int fpscap = luaL_optint(l, 1, -1);
	if (fpscap == -1)
	{
		lua_pushnumber(l, limitFPS);
		return 1;
	}
	if (fpscap < 2)
		return luaL_error(l, "fps cap too small");
	limitFPS = fpscap;
	return 0;
}
int luatpt_getscript(lua_State* l)
{
	char *fileid = NULL, *filedata = NULL, *fileuri = NULL, *fileauthor = NULL, *filename = NULL, *lastError = NULL, *luacommand = NULL;
	int len, ret,run_script;
	FILE * outputfile;

	fileauthor = mystrdup((char*)luaL_optstring(l, 1, ""));
	fileid = mystrdup((char*)luaL_optstring(l, 2, ""));
	run_script = luaL_optint(l, 3, 0);
	if(!fileauthor || !fileid || strlen(fileauthor)<1 || strlen(fileid)<1)
		goto fin;
	if(!confirm_ui(vid_buf, "Do you want to install script?", fileid, "Install"))
		goto fin;

	fileuri = malloc(strlen(SCRIPTSERVER)+strlen(fileauthor)+strlen(fileid)+44);
	sprintf(fileuri, "http://" SCRIPTSERVER "/GetScript.api?Author=%s&Filename=%s", fileauthor, fileid);
	
	filedata = (char*)http_auth_get(fileuri, svf_user_id, NULL, svf_session_id, &ret, &len);
	
	if(len <= 0 || !filedata)
	{
		lastError = "Server did not return data.";
		goto fin;
	}
	if(ret != 200)
	{
		lastError = (char*)http_ret_text(ret);
		goto fin;
	}
	
	filename = malloc(strlen(fileauthor)+strlen(fileid)+strlen(PATH_SEP)+strlen(LOCAL_LUA_DIR)+6);
	sprintf(filename, LOCAL_LUA_DIR PATH_SEP "%s_%s.lua", fileauthor, fileid);
	
#ifdef WIN32
	_mkdir(LOCAL_LUA_DIR);
#else
	mkdir(LOCAL_LUA_DIR, 0755);
#endif
	
	outputfile = fopen(filename, "r");
	if(outputfile)
	{
		fclose(outputfile);
		outputfile = NULL;
		if(confirm_ui(vid_buf, "File already exists, overwrite?", filename, "Overwrite"))
		{
			outputfile = fopen(filename, "w");
		}
		else
		{
			goto fin;
		}
	}
	else
	{
		outputfile = fopen(filename, "w");
	}
	
	if(!outputfile)
	{
		lastError = "Unable to write to file";
		goto fin;
	}
	
	
	fputs(filedata, outputfile);
	fclose(outputfile);
	outputfile = NULL;
	if(run_script)
	{
    luacommand = malloc(strlen(filename)+20);
    sprintf(luacommand,"dofile(\"%s\")",filename);
    luacon_eval(luacommand);
    }
    
fin:
	if(fileid) free(fileid);
	if(filedata) free(filedata);
	if(fileuri) free(fileuri);
	if(fileauthor) free(fileauthor);
	if(filename) free(filename);
	if(luacommand) free(luacommand);
	luacommand = NULL;
		
	if(lastError) return luaL_error(l, lastError);
	return 0;
}

int luatpt_setwindowsize(lua_State* l)
{
	int result, scale = luaL_optint(l,1,1), kiosk = luaL_optint(l,2,0);
	if (scale!=2) scale = 1;
	if (kiosk!=1) kiosk = 0;
	result = set_scale(scale, kiosk);
	lua_pushnumber(l, result);
	return 1;
}

int luatpt_screenshot(lua_State* l)
{
	int captureUI = luaL_optint(l, 1, 0);
	if(vid_buf)
	{
		if(captureUI)
		{
			dump_frame(vid_buf, XRES+BARSIZE, YRES+MENUSIZE, XRES+BARSIZE);
		}
		else
		{
			dump_frame(vid_buf, XRES, YRES, XRES+BARSIZE);
		}
		return 0;
	}
	return luaL_error(l, "Screen buffer does not exist");
}

int luatpt_sound(lua_State* l)
{
	char *filename;
	filename = mystrdup((char*)luaL_optstring(l, 1, ""));
	if (sound_enable) play_sound(filename);
	else return luaL_error(l, "Audio device not available - cannot play sounds");
	return 0;
}

int luatpt_load(lua_State* l)
{
	char *savenum;
	int instant;
	savenum = mystrdup((char*)luaL_optstring(l, 1, ""));
	instant = luaL_optint(l, 2, 0);
	open_ui(vid_buf, savenum, NULL, instant);
	console_mode = 0;
	return 0;
}

int luatpt_bubble(lua_State* l)
{
	int x = luaL_optint(l, 1, 0);
	int y = luaL_optint(l, 1, 0);
	int first, rem1, rem2, i;

	first = create_part(-1, x+18, y, PT_SOAP);
	rem1 = first;

	for (i = 1; i<=30; i++)
	{
		rem2 = create_part(-1, (int)(x+18*cosf(i/5.0)), (int)(y+18*sinf(i/5.0)), PT_SOAP);

		parts[rem1].ctype = 7;
		parts[rem1].tmp = rem2;
		parts[rem2].tmp2 = rem1;

		rem1 = rem2;
	}

	parts[rem1].ctype = 7;
	parts[rem1].tmp = first;
	parts[first].tmp2 = rem1;
	parts[first].ctype = 7;
	return 0;
}

int luatpt_reset_pressure(lua_State* l)
{
	int nx, ny;
	int x1, y1, width, height;
	x1 = abs(luaL_optint(l, 1, 0));
	y1 = abs(luaL_optint(l, 2, 0));
	width = abs(luaL_optint(l, 3, XRES/CELL));
	height = abs(luaL_optint(l, 4, YRES/CELL));
	if(x1 > (XRES/CELL)-1)
		x1 = (XRES/CELL)-1;
	if(y1 > (YRES/CELL)-1)
		y1 = (YRES/CELL)-1;
	if(x1+width > (XRES/CELL)-1)
		width = (XRES/CELL)-x1;
	if(y1+height > (YRES/CELL)-1)
		height = (YRES/CELL)-y1;
	for (nx = x1; nx<x1+width; nx++)
		for (ny = y1; ny<y1+height; ny++)
		{
			pv[ny][nx] = 0;
		}
	return 0;
}

int luatpt_reset_temp(lua_State* l)
{
	int i;
	for (i=0; i<NPART; i++)
	{
		if (parts[i].type)
		{
			parts[i].temp = ptypes[parts[i].type].heat;
		}
	}
	return 0;
}

int luatpt_get_pressure(lua_State* l)
{
	int x, y;
	x = luaL_optint(l, 1, 0);
	y = luaL_optint(l, 2, 0);
	if (x*CELL<0 || y*CELL<0 || x*CELL>=XRES || y*CELL>=YRES)
		return luaL_error(l, "coordinates out of range (%d,%d)", x, y);
	lua_pushnumber(l, pv[y][x]);
	return 1;
}

int luatpt_get_aheat(lua_State* l)
{
	int x, y;
	x = luaL_optint(l, 1, 0);
	y = luaL_optint(l, 2, 0);
	if (x*CELL<0 || y*CELL<0 || x*CELL>=XRES || y*CELL>=YRES)
		return luaL_error(l, "coordinates out of range (%d,%d)", x, y);
	lua_pushnumber(l, hv[y][x]);
	return 1;
}

int luatpt_get_velocity(lua_State* l)
{
	int x, y;
	char *direction = (char*)luaL_optstring(l, 1, "x");
	x = luaL_optint(l, 2, 0);
	y = luaL_optint(l, 3, 0);
	if (x*CELL<0 || y*CELL<0 || x*CELL>=XRES || y*CELL>=YRES)
		return luaL_error(l, "coordinates out of range (%d,%d)", x, y);
	if (strcmp(direction,"x")==0)
		lua_pushnumber(l, vx[y][x]);
	else if (strcmp(direction,"y")==0)
		lua_pushnumber(l, vy[y][x]);
	else
		return luaL_error(l, "Invalid direction: %s", direction);
	return 1;
}

int luatpt_get_gravity(lua_State* l)
{
	int x, y;
	char* xy;
	x = luaL_optint(l, 1, 0);
	y = luaL_optint(l, 2, 0);
	xy = (char*)luaL_optstring(l, 3, "");
	if (x*CELL<0 || y*CELL<0 || x*CELL>=XRES || y*CELL>=YRES)
		return luaL_error(l, "coordinates out of range (%d,%d)", x, y);
	if (strcmp(xy,"x")==0)
		lua_pushnumber(l, (double)gravx[y*XRES/4+x]);
	else if (strcmp(xy,"y")==0)
		lua_pushnumber(l, (double)gravy[y*XRES/4+x]);
	else
		lua_pushnumber(l,gravmap[y*(XRES/CELL)+x]); //Getting gravity doesn't work either...
	return 1;
}

int luatpt_maxframes(lua_State* l)
{
	int newmaxframes = luaL_optint(l,1,-1), i;
	if (newmaxframes == -1)
	{
		lua_pushnumber(l, maxframes);
		return 1;
	}
	if (newmaxframes > 0 && newmaxframes <= 256)
		maxframes = newmaxframes;
	else
		return luaL_error(l, "must be between 1 and 256");
	for (i = 0; i < NPART; i++)
		if (parts[i].type == PT_ANIM)
		{
			kill_part(i);
			create_part(-1, (int)parts[i].x, (int)parts[i].y, PT_ANIM);
		}
	return 0;
}

int luatpt_createwall(lua_State* l)
{
	int acount, wx, wy, wt, width = 0, height = 0, nx, ny;
	acount = lua_gettop(l);
	wx = luaL_optint(l,1,-1);
	wy = luaL_optint(l,2,-1);
	if (acount > 3)
	{
		width = luaL_optint(l,3,1);
		height = luaL_optint(l,4,1);
	}
	if(lua_isnumber(l, acount))
		wt = luaL_optint(l,acount,WL_WALL);
	else
	{
		char* name = (char*)luaL_optstring(l, acount, "WALL");
		if (!console_parse_wall_type(name, &wt))
			return luaL_error(l, "Unrecognised wall '%s'", name);
	}
	if (wx < 0 || wx >= XRES/CELL || wy < 0 || wy >= YRES/CELL)
		return luaL_error(l, "coordinates out of range (%d,%d)", wx, wy);
	if (wx+width > (XRES/CELL))
		width = (XRES/CELL)-wx;
	if (wy+height > (YRES/CELL))
		height = (YRES/CELL)-wy;
	if ((wt < UI_ACTUALSTART && wt != 0) || wt >= UI_ACTUALSTART+UI_WALLCOUNT || (wtypes[wt-UI_ACTUALSTART].drawstyle == -1 && !secret_els))
		return luaL_error(l, "Unrecognised wall number %d", wt);
	for (nx = wx; nx < wx+width; nx++)
		for (ny = wy; ny < wy+height; ny++)
			bmap[ny][nx] = wt;
	return 0;
}

int luatpt_set_elecmap(lua_State* l)
{
	int acount = lua_gettop(l);
	int x1 = luaL_optint(l,1,-1);
	int y1 = luaL_optint(l,2,-1);
	int width = luaL_optint(l,3,1);
	int height = luaL_optint(l,4,1);
	int value = luaL_optint(l,acount,0);
	int nx, ny;

	if (x1 < 0 || x1 >= XRES/CELL || y1 < 0 || y1 >= YRES/CELL)
		return luaL_error(l, "coordinates out of range (%d,%d)", x1, y1);

	if (x1+width > (XRES/CELL))
		width = (XRES/CELL)-x1;
	if (y1+height > (YRES/CELL))
		height = (YRES/CELL)-y1;

	for (nx = x1; nx < x1+width; nx++)
		for (ny = y1; ny < y1+height; ny++)
			emap[ny][nx] = value;
	return 0;
}

int luatpt_getwall(lua_State* l)
{
	int wx = luaL_optint(l,1,-1);
	int wy = luaL_optint(l,2,-1);
	if (wx < 0 || wx > XRES/CELL || wy < 0 || wy > YRES/CELL)
		return luaL_error(l, "coordinates out of range (%d,%d)", wx, wy);
	lua_pushnumber(l, bmap[wy][wx]);
	return 1;
}

int luatpt_get_elecmap(lua_State* l)
{
	int wx = luaL_optint(l,1,-1);
	int wy = luaL_optint(l,2,-1);
	if (wx < 0 || wx > XRES/CELL || wy < 0 || wy > YRES/CELL)
		return luaL_error(l, "coordinates out of range (%d,%d)", wx, wy);
	lua_pushinteger(l, emap[wy][wx]);
	return 1;
}

int luatpt_clear_sim(lua_State* l)
{
	clear_sim();
	return 0;
}

int luatpt_reset_elements(lua_State* l)
{
	memcpy(ptypes,ptypes2,sizeof(ptypes));
	memcpy(ptransitions,ptransitions2,sizeof(ptransitions));
	menu_count();
	init_can_move();
	memset(graphicscache, 0, sizeof(gcache_item)*PT_NUM);
	return 0;
}

int luatpt_indestructible(lua_State* l)
{
	int el = 0, ind;
	char* name;
	if(lua_isnumber(l, 1)){
		el = luaL_optint(l, 1, 0);
		if (el<0 || el>=PT_NUM)
			return luaL_error(l, "Unrecognised element number '%d'", el);
	} else {
		name = (char*)luaL_optstring(l, 1, "dust");
		if (!console_parse_type(name, &el, NULL))
			return luaL_error(l, "Unrecognised element '%s'", name);
	}
	ind = luaL_optint(l, 2, 1);
	if (ind)
		ptypes[el].properties |= PROP_INDESTRUCTIBLE;
	else
		ptypes[el].properties &= ~PROP_INDESTRUCTIBLE;
	return 0;
}

int luatpt_moving_solid(lua_State* l)
{
	int el = 0, movs;
	char* name;
	if(lua_isnumber(l, 1)){
		el = luaL_optint(l, 1, 0);
		if (el<0 || el>=PT_NUM)
			return luaL_error(l, "Unrecognised element number '%d'", el);
	} else {
		name = (char*)luaL_optstring(l, 1, "dust");
		if (!console_parse_type(name, &el, NULL))
			return luaL_error(l, "Unrecognised element '%s'", name);
	}
	movs = luaL_optint(l, 2, 1);
	if (movs)
	{
		ptypes[el].properties |= PROP_MOVS;
		ptypes[el].advection = ptypes[PT_MOVS].advection;
		ptypes[el].airdrag = ptypes[PT_MOVS].airdrag;
		ptypes[el].airloss = ptypes[PT_MOVS].airloss;
		ptypes[el].gravity = ptypes[PT_MOVS].gravity;
		ptypes[el].loss = ptypes[PT_MOVS].loss;
		ptypes[el].falldown = ptypes[PT_MOVS].falldown;
	}
	else
	{
		ptypes[el].properties &= ~PROP_MOVS;
		ptypes[el].advection = ptypes2[el].advection;
		ptypes[el].airdrag = ptypes2[el].airdrag;
		ptypes[el].airloss = ptypes2[el].airloss;
		ptypes[el].gravity = ptypes2[el].gravity;
		ptypes[el].loss = ptypes2[el].loss;
		ptypes[el].falldown = ptypes2[el].falldown;
	}
	init_can_move();
	return 0;
}

int luatpt_create_parts(lua_State* l)
{
	int x = luaL_optint(l,1,-1);
	int y = luaL_optint(l,2,-1);
	int rx = luaL_optint(l,3,5);
	int ry = luaL_optint(l,4,5);
	int c = luaL_optint(l,5,sl);
	int fill = luaL_optint(l,6,1);
	int brush = luaL_optint(l,7,CIRCLE_BRUSH);
	int flags = luaL_optint(l,8,get_brush_flags());
	int ret, oldbrush = CURRENT_BRUSH;
	if (x < 0 || x > XRES || y < 0 || y > YRES)
		return luaL_error(l, "coordinates out of range (%d,%d)", x, y);
	if ((c>=0 && c < PT_NUM && !ptypes[c].enabled) || c < 0)
		return luaL_error(l, "Unrecognised element number '%d'", c);
	CURRENT_BRUSH = brush;
	ret = create_parts(x, y, rx, ry, c, flags, fill);
	CURRENT_BRUSH = oldbrush;
	lua_pushinteger(l, ret);
	return 1;
}

int luatpt_create_line(lua_State* l)
{
	int x1 = luaL_optint(l,1,-1);
	int y1 = luaL_optint(l,2,-1);
	int x2 = luaL_optint(l,3,-1);
	int y2 = luaL_optint(l,4,-1);
	int rx = luaL_optint(l,5,5);
	int ry = luaL_optint(l,6,5);
	int c = luaL_optint(l,7,sl);
	int brush = luaL_optint(l,8,CIRCLE_BRUSH);
	int flags = luaL_optint(l,9,get_brush_flags());
	int oldbrush = CURRENT_BRUSH;
	if (x1 < 0 || x1 > XRES || y1 < 0 || y1 > YRES)
		return luaL_error(l, "Starting coordinates out of range (%d,%d)", x1, y1);
	if (x2 < 0 || x2 > XRES || y2 < 0 || y2 > YRES)
		return luaL_error(l, "Ending coordinates out of range (%d,%d)", x2, y2);
	if ((c>=0 && c < PT_NUM && !ptypes[c].enabled) || c < 0)
		return luaL_error(l, "Unrecognised element number '%d'", c);
	CURRENT_BRUSH = brush;
	create_line(x1, y1, x2, y2, rx, ry, c, flags);
	CURRENT_BRUSH = oldbrush;
	return 0;
}

int luatpt_floodfill(lua_State* l)
{
	int x = luaL_optint(l,1,-1);
	int y = luaL_optint(l,2,-1);
	int c = luaL_optint(l,3,sl);
	int cm = luaL_optint(l,4,-1);
	int bm = luaL_optint(l,5,-1);
	int flags = luaL_optint(l,6,get_brush_flags());
	int ret;
	if (x < 0 || x > XRES || y < 0 || y > YRES)
		return luaL_error(l, "coordinates out of range (%d,%d)", x, y);
	if ((c>=0 && c < PT_NUM && !ptypes[c].enabled) || c < 0)
		return luaL_error(l, "Unrecognised element number '%d'", c);
	ret = flood_parts(x, y, c, cm, bm, flags);
	lua_pushinteger(l, ret);
	return 1;
}

int luatpt_save_stamp(lua_State* l)
{
	int x = luaL_optint(l,1,0);
	int y = luaL_optint(l,2,0);
	int w = luaL_optint(l,3,XRES);
	int h = luaL_optint(l,4,YRES);
	char *name = stamp_save(x, y, w, h);
	lua_pushstring(l, name);
	return 1;
}

int luatpt_load_stamp(lua_State* l)
{
	int stamp_size, i, j, x, y, ret;
	void *load_data;
	char *filename;
	i = luaL_optint(l, 1, -1);
	if (i < 0 || i >= STAMP_MAX)
	{
		filename = (char*)luaL_optstring(l, 1, "error?");
		for (j=0; j<STAMP_MAX; j++)
			if (!strcmp(stamps[j].name, filename))
			{
				i = j;
				break;
			}
	}
	x = luaL_optint(l,2,0);
	y = luaL_optint(l,3,0);
	if (i < 0 || i >= STAMP_MAX)
		return luaL_error(l, "Invavlid stamp ID: %d", i);
	load_data = stamp_load(i, &stamp_size, 0);
	ret = parse_save(load_data, stamp_size, 0, x, y, bmap, vx, vy, pv, fvx, fvy, signs, parts, pmap);
	lua_pushinteger(l, ret);
	return 1;
}

void addluastuff()
{
	int i, next, j = 0, x = 5, y = 4, num = 0, total = 1, i2, k;
	FILE *file = fopen ("luacode.txt", "r");
	char* test;
	if (file == NULL)
	{
		error_ui(vid_buf,0,"file luacode.txt does not exist");
		return;
	}
	delete_part(4,4,0);
	i2 = create_part(-1,4,4,PT_INDI);
	parts[i2].animations = (unsigned int*)calloc(257,sizeof(unsigned int));
	memset(parts[i2].animations, 0, sizeof(parts[i].animations));
	parts[i2].animations[1] = 0;
	parts[i2].animations[2] = SAVE_VERSION;
	parts[i2].animations[3] = MINOR_VERSION;
	parts[i2].animations[4] = BUILD_NUM;
	parts[i2].animations[5] = 120;
	parts[i2].animations[6] = 0;
	parts[i2].ctype = 256;
	for (k = 0; k < 64; k++)
		parts[i2].animations[k+150] = svf_user[k];

	delete_part(5,4,0);
	i = create_part(-1,5,4,PT_INDI);
	parts[i].animations = (unsigned int*)calloc(257,sizeof(unsigned int));
	memset(parts[i].animations, 0, sizeof(parts[i].animations));
	parts[i].ctype = 256;
	while((next = fgetc(file)) != EOF)
	{
		parts[i].animations[j] = next;
		j++;
		num++;
		total++;
		if (num == 256)
		{
			j = num = 0;
			x++;
			if (x >= XRES-4)
			{
				x = 4;
				y++;
			}
			delete_part(x,y,0);
			i = create_part(-1,x,y,PT_INDI);
			parts[i].animations = (unsigned int*)calloc(257,sizeof(unsigned int));
			memset(parts[i].animations, 0, sizeof(parts[i].animations));
			parts[i].ctype = 256;
		}
	}
	parts[i2].animations[0] = total;
	fclose(file);
}

void readluastuff()
{
	int i = pmap[4][5], i2 = pmap[4][4], x = 5, y = 4, total = 1;
	if ((i2&0xFF) == PT_INDI && parts[i2>>8].animations && parts[i2>>8].animations[1] == 0)
	{
		int num = parts[i2>>8].animations[0], j = 0;
		FILE *file = fopen ("newluacode.txt", "w+");
		if (file == NULL)
		{
			delete_part(4,4,0);
			return;
		}
		while(total < num)
		{
			fputc(parts[i>>8].animations[j++],file);
			total++;
			if (j >= 256)
			{
				j = 0;
				x++;
				if (x >= XRES-4)
				{
					x = 4;
					y++;
				}
				i = pmap[y][x];
				if ((i&0xFF) != PT_INDI)
					j = num;
			}
		}
		parts[i2>>8].animations[1] = 1;
		fclose(file);
		luaL_dostring(l,"\n\
			local env = {\n\
				ipairs = ipairs,\n\
				next = next,\n\
				pairs = pairs,\n\
				pcall = pcall,\n\
				tonumber = tonumber,\n\
				tostring = tostring,\n\
				type = type,\n\
				unpack = unpack,\n\
				coroutine = { create = coroutine.create, resume = coroutine.resume, \n\
					running = coroutine.running, status = coroutine.status, \n\
					wrap = coroutine.wrap }, \n\
				string = { byte = string.byte, char = string.char, find = string.find, \n\
					format = string.format, gmatch = string.gmatch, gsub = string.gsub, \n\
					len = string.len, lower = string.lower, match = string.match, \n\
					rep = string.rep, reverse = string.reverse, sub = string.sub, \n\
					upper = string.upper },\n\
				table = { insert = table.insert, maxn = table.maxn, remove = table.remove, \n\
					sort = table.sort },\n\
				math = { abs = math.abs, acos = math.acos, asin = math.asin, \n\
					atan = math.atan, atan2 = math.atan2, ceil = math.ceil, cos = math.cos, \n\
					cosh = math.cosh, deg = math.deg, exp = math.exp, floor = math.floor, \n\
					fmod = math.fmod, frexp = math.frexp, huge = math.huge, \n\
					ldexp = math.ldexp, log = math.log, log10 = math.log10, max = math.max, \n\
					min = math.min, modf = math.modf, pi = math.pi, pow = math.pow, \n\
					rad = math.rad, random = math.random, randomseed = math.randomseed, sin = math.sin, sinh = math.sinh, \n\
					sqrt = math.sqrt, tan = math.tan, tanh = math.tanh },\n\
				os = { clock = os.clock, difftime = os.difftime, time = os.time, date = os.date, exit = os.exit},\n\
				tpt = tpt\n\
			}\n\
			function run(untrusted_code)\n\
				--if untrusted_code:byte(1) == 27 then return nil, \"binary bytecode prohibited\" end\n\
				--local untrusted_function, message = loadstring(untrusted_code)\n\
				if not untrusted_code then return nil end\n\
				setfenv(untrusted_code, env)\n\
				untrusted_code()\n\
			end\n\
		");

		loop_time = SDL_GetTicks();
		luaL_dostring(l,"run(loadfile(\"newluacode.txt\"))");
		if (parts[i2>>8].animations[6] == 1)
			remove("newluacode.txt");
	}
}

#endif
