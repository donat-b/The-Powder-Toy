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
#include "defines.h"
//#include "Simulation.h" //TODO: make Simulation an arg later


enum { ELEMENT_TOOL, WALL_TOOL, TOOL_TOOL, DECO_TOOL, GOL_TOOL, INVALID_TOOL };

struct Point;
class Brush;
class Tool
{
	std::string identifier;
protected:
	int type;
	int ID;
public:
	Tool(int toolID, std::string toolIdentifier);
	Tool(int toolType, int toolID, std::string toolIdentifier);
	virtual ~Tool() {}

	int GetType() { return type; }
	int GetID() { return ID; }
	std::string GetIdentifier() { return identifier; }

	virtual int DrawPoint(Brush* brush, Point position);
	virtual void DrawLine(Brush* brush, Point startPos, Point endPos, bool held);
	virtual void DrawRect(Brush* brush, Point startPos, Point endPos);
	virtual int FloodFill(Brush* brush, Point position);
	virtual void Click(Point position);
	virtual Tool* Sample(Point position);
};

class ElementTool : public Tool
{
public:
	ElementTool(int elementID);
	int GetID();
};

class PlopTool : public ElementTool
{
public:
	PlopTool(int elementID);

	virtual int DrawPoint(Brush* brush, Point position);
	virtual void DrawLine(Brush* brush, Point startPos, Point endPos, bool held);
	virtual void DrawRect(Brush* brush, Point startPos, Point endPos);
	virtual int FloodFill(Brush* brush, Point position);
	virtual void Click(Point position);
};

class GolTool : public Tool
{
public:
	GolTool(int golID);
	int GetID();

	virtual int DrawPoint(Brush* brush, Point position);
	virtual void DrawLine(Brush* brush, Point startPos, Point endPos, bool held);
	virtual void DrawRect(Brush* brush, Point startPos, Point endPos);
	virtual int FloodFill(Brush* brush, Point position);
};

class WallTool : public Tool
{
public:
	WallTool(int wallID);
	int GetID() { if (type == WALL_TOOL) return ID; else return -1; }

	virtual int DrawPoint(Brush* brush, Point position);
	virtual void DrawLine(Brush* brush, Point startPos, Point endPos, bool held);
	virtual void DrawRect(Brush* brush, Point startPos, Point endPos);
	virtual int FloodFill(Brush* brush, Point position);
};

class StreamlineTool : public WallTool
{
public:
	StreamlineTool();

	virtual int DrawPoint(Brush* brush, Point position);
	virtual void DrawLine(Brush* brush, Point startPos, Point endPos, bool held);
	virtual int FloodFill(Brush* brush, Point position);
};

class ToolTool : public Tool
{
public:
	ToolTool(int toolID);
	int GetID() { if (type == TOOL_TOOL) return ID; else return -1; }

	virtual int DrawPoint(Brush* brush, Point position);
	virtual void DrawLine(Brush* brush, Point startPos, Point endPos, bool held);
	virtual void DrawRect(Brush* brush, Point startPos, Point endPos);
	virtual int FloodFill(Brush* brush, Point position);
	virtual void Click(Point position);
};

class PropTool : public ToolTool
{
public:
	PropTool();
	~PropTool() {}

	virtual int DrawPoint(Brush* brush, Point position);
	virtual void DrawLine(Brush* brush, Point startPos, Point endPos, bool held);
	virtual void DrawRect(Brush* brush, Point startPos, Point endPos);
	virtual int FloodFill(Brush* brush, Point position);

	PropertyType propType;
	PropertyValue propValue;
	size_t propOffset;
};

class DecoTool : public Tool
{
public:
	DecoTool(int decoID);
	int GetID() { if (type == DECO_TOOL) return ID; else return -1; }

	virtual int DrawPoint(Brush* brush, Point position);
	virtual void DrawLine(Brush* brush, Point startPos, Point endPos, bool held);
	virtual void DrawRect(Brush* brush, Point startPos, Point endPos);
	virtual int FloodFill(Brush* brush, Point position);
	virtual Tool* Sample(Point position);
};

#endif
