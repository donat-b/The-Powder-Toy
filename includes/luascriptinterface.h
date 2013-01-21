#ifndef LUASCRIPT_H
#define LUASCRIPT_H

#include <luaconsole.h>

void initSimulationAPI(lua_State * l);
int simulation_partNeighbours(lua_State * l);
int simulation_partChangeType(lua_State * l);
int simulation_partCreate(lua_State * l);
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

#endif
