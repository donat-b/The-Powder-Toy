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

#endif
