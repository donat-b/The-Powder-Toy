/**
 * Powder Toy - Tool (header)
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
#ifndef TOOL_H
#define TOOL_H

#include <iostream>
//#include "Simulation.h" //TODO: make Simulation an arg later
#include "common/Point.h"

enum { ELEMENT_TOOL, WALL_TOOL, TOOL_TOOL, DECO_TOOL, GOL_TOOL, INVALID_TOOL };

class Brush;
class Tool
{
	int type;
	int ID;
	std::string identifier;
	float strength;
public:
	Tool(int toolType, int toolID, std::string toolIdentifier);

	void SetStrength(float newStrength) { strength = newStrength; }

	int GetType() { return type; }
	int GetID() { return ID; }
	std::string GetIdentifier() { return identifier; }
	int GetElementID() { if (type == ELEMENT_TOOL) return ID; else return -1; }
	int GetWallID() { if (type == WALL_TOOL) return ID; else return -1; }
	int GetToolID() { if (type == TOOL_TOOL) return ID; else return -1; }

	int DrawPoint(Brush* brush, Point position);
	int DrawLine(Brush* brush, Point startPos, Point endPos, bool held);
	void DrawRect(Brush* brush, Point startPos, Point endPos);
	int FloodFill(Point position);
	Tool* Sample(Point position);
};

#endif
