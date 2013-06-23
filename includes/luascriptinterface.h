#ifndef LUASCRIPT_H
#define LUASCRIPT_H

#include <luaconsole.h>

// idea from mniip, makes things much simpler 
#define SETCONST(L, NAME)\
	lua_pushinteger(L, NAME);\
	lua_setfield(L, -2, #NAME);

void initSimulationAPI(lua_State * l);
int simulation_partNeighbours(lua_State * l);
int simulation_partChangeType(lua_State * l);
int simulation_partCreate(lua_State * l);
int simulation_partID(lua_State * l);
int simulation_partProperty(lua_State * l);
int simulation_partPosition(lua_State * l);
int simulation_partKill(lua_State * l);
int simulation_pressure(lua_State* l);
int simulation_ambientHeat(lua_State* l);
int simulation_velocityX(lua_State* l);
int simulation_velocityY(lua_State* l);
int simulation_gravMap(lua_State* l);

void initRendererAPI(lua_State * l);
int renderer_renderModes(lua_State * l);
int renderer_displayModes(lua_State * l);
int renderer_colourMode(lua_State * l);
int renderer_decorations(lua_State * l);

void initFileSystemAPI(lua_State * l);
int fileSystem_list(lua_State * l);
int fileSystem_exists(lua_State * l);
int fileSystem_isFile(lua_State * l);
int fileSystem_isDirectory(lua_State * l);
int fileSystem_makeDirectory(lua_State * l);
int fileSystem_removeDirectory(lua_State * l);
int fileSystem_removeFile(lua_State * l);
int fileSystem_move(lua_State * l);
int fileSystem_copy(lua_State * l);

void initGraphicsAPI(lua_State * l);
int graphics_textSize(lua_State * l);
int graphics_drawText(lua_State * l);
int graphics_drawLine(lua_State * l);
int graphics_drawRect(lua_State * l);
int graphics_fillRect(lua_State * l);

void initIdentifiers();
void initElementsAPI(lua_State * l);
int elements_allocate(lua_State * l);
int elements_element(lua_State * l);
int elements_property(lua_State * l);
int elements_loadDefault(lua_State * l);
int elements_free(lua_State * l);

#endif
