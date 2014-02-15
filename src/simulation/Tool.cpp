#include <sstream>
#include "Tool.h"
#include "interface.h"
#include "powder.h"
#include "Simulation.h"
#include "WallNumbers.h"
#include "ToolNumbers.h"
#include "GolNumbers.h"
#include "game/Brush.h"

Tool::Tool(int toolID, std::string toolIdentifier):
	type(INVALID_TOOL),
	ID(toolID),
	identifier(toolIdentifier)
{

}

Tool::Tool(int toolType, int toolID, std::string toolIdentifier):
	type(toolType),
	ID(toolID),
	identifier(toolIdentifier)
{

}

int Tool::DrawPoint(Brush* brush, Point position)
{
	if (globalSim->elements[ID].Properties&PROP_MOVS)
		create_moving_solid(position.X, position.Y, brush->GetRadius().X, brush->GetRadius().Y, ID);
	else
		return globalSim->CreateParts(position.X, position.Y, brush->GetRadius().X, brush->GetRadius().Y, ID, get_brush_flags(), true);
	return 0;
}

void Tool::DrawLine(Brush* brush, Point startPos, Point endPos, bool held)
{
	if (held && (globalSim->elements[ID].Properties&PROP_MOVS))
		return;
	globalSim->CreateLine(startPos.X, startPos.Y, endPos.X, endPos.Y, currentBrush->GetRadius().X, currentBrush->GetRadius().Y, ID, get_brush_flags());
}

void Tool::DrawRect(Brush* brush, Point startPos, Point endPos)
{
	globalSim->CreateBox(startPos.X, startPos.Y, endPos.X, endPos.Y, ID, get_brush_flags());
}

int Tool::FloodFill(Point position)
{
	return globalSim->FloodParts(position.X, position.Y, ID, -1, get_brush_flags());
}

Tool* Tool::Sample(Point position)
{
	if (position.Y < 0 || position.Y >= YRES || position.X < 0 || position.X >= XRES)
		return this;

	int sample = pmap[position.Y][position.X];
	if (sample || (sample = photons[position.Y][position.X]))
	{
		if ((sample&0xFF) == PT_LIFE)
		{
			if (parts[sample>>8].ctype < NGOL)
				return GetToolFromIdentifier(golTypes[parts[sample>>8].ctype].identifier);
		}
		else
		{
			return GetToolFromIdentifier(globalSim->elements[sample&0xFF].Identifier);
		}
	}
	else if (bmap[position.Y/CELL][position.X/CELL] > 0 && bmap[position.Y/CELL][position.X/CELL] < WALLCOUNT)
	{
		return GetToolFromIdentifier(wallTypes[bmap[position.Y / CELL][position.X / CELL]].identifier);
	}
	return this;
}


ElementTool::ElementTool(int elementID):
	Tool(ELEMENT_TOOL, elementID, globalSim->elements[elementID].Identifier)
{

}
int ElementTool::GetID()
{
	if (type == ELEMENT_TOOL)
		return ID;
	else if (type == GOL_TOOL)
		return PT_LIFE;
	else
		return -1;
}

GolTool::GolTool(int golID):
	Tool(GOL_TOOL, golID, golTypes[golID].identifier)
{

}
int GolTool::GetID()
{
	if (type == GOL_TOOL)
		return ID;
	//else if (type == ELEMENT_TOOL)
	//	return PT_LIFE;
	else
		return -1;
}
int GolTool::DrawPoint(Brush* brush, Point position)
{
	return globalSim->CreateParts(position.X, position.Y, brush->GetRadius().X, brush->GetRadius().Y, PT_LIFE+(ID<<8), get_brush_flags(), true);
}
void GolTool::DrawLine(Brush* brush, Point startPos, Point endPos, bool held)
{
	globalSim->CreateLine(startPos.X, startPos.Y, endPos.X, endPos.Y, currentBrush->GetRadius().X, currentBrush->GetRadius().Y, PT_LIFE+(ID<<8), get_brush_flags());
}
void GolTool::DrawRect(Brush* brush, Point startPos, Point endPos)
{
	globalSim->CreateBox(startPos.X, startPos.Y, endPos.X, endPos.Y, PT_LIFE+(ID<<8), get_brush_flags());
}
int GolTool::FloodFill(Point position)
{
	return globalSim->FloodParts(position.X, position.Y, PT_LIFE+(ID<<8), -1, get_brush_flags());
}


WallTool::WallTool(int wallID):
	Tool(WALL_TOOL, wallID, wallTypes[wallID].identifier)
{

}
int WallTool::DrawPoint(Brush* brush, Point position)
{
	int rx = brush->GetRadius().X/CELL;
	int ry = brush->GetRadius().Y/CELL;
	int x = position.X/CELL-rx/2;
	int y = position.Y/CELL-ry/2;
	globalSim->CreateWallBox(x, y, x+rx, y+ry, ID);
	return 1;
}
void WallTool::DrawLine(Brush* brush, Point startPos, Point endPos, bool held)
{
	if (!held && ID == WL_FAN && bmap[startPos.Y/CELL][startPos.X/CELL] == WL_FAN)
	{
		float nfvx = (endPos.X-startPos.X)*0.005f;
		float nfvy = (endPos.Y-startPos.Y)*0.005f;
		globalSim->FloodWalls(startPos.X/CELL, startPos.Y/CELL, WL_FANHELPER, WL_FAN);
		for (int j=0; j<YRES/CELL; j++)
			for (int i=0; i<XRES/CELL; i++)
				if (bmap[j][i] == WL_FANHELPER)
				{
					fvx[j][i] = nfvx;
					fvy[j][i] = nfvy;
					bmap[j][i] = WL_FAN;
				}
	}
	else
	{
		int rx = brush->GetRadius().X/CELL;
		int ry = brush->GetRadius().Y/CELL;
		globalSim->CreateWallLine(startPos.X/CELL-rx/2, startPos.Y/CELL-ry/2, endPos.X/CELL-rx/2, endPos.Y/CELL-ry/2, rx, ry, ID);
	}
}
void WallTool::DrawRect(Brush* brush, Point startPos, Point endPos)
{
	globalSim->CreateWallBox(startPos.X/CELL, startPos.Y/CELL, endPos.X/CELL, endPos.Y/CELL, ID);
}
int WallTool::FloodFill(Point position)
{
	return globalSim->FloodWalls(position.X/CELL, position.Y/CELL, ID, -1);
}

StreamlineTool::StreamlineTool():
	WallTool(WL_STREAM)
{

}
int StreamlineTool::DrawPoint(Brush* brush, Point position)
{
	globalSim->CreateWall(position.X, position.Y, ID);
	return 0;
}
void StreamlineTool::DrawLine(Brush* brush, Point startPos, Point endPos, bool held)
{
	globalSim->CreateWallLine(startPos.X/CELL, startPos.Y/CELL, endPos.X/CELL, endPos.Y/CELL, 0, 0, ID);
}


ToolTool::ToolTool(int toolID):
	Tool(TOOL_TOOL, toolID, toolTypes[toolID].identifier)
{

}
int ToolTool::DrawPoint(Brush* brush, Point position)
{
	globalSim->CreateToolBrush(position.X, position.Y, brush->GetRadius().X, brush->GetRadius().Y, ID, toolStrength);
	return 0;
}
void ToolTool::DrawLine(Brush* brush, Point startPos, Point endPos, bool held)
{
	if (ID == TOOL_WIND && !held)
		return;
	globalSim->CreateToolLine(startPos.X, startPos.Y, endPos.X, endPos.Y, currentBrush->GetRadius().X, currentBrush->GetRadius().Y, ID, toolStrength);
}
void ToolTool::DrawRect(Brush* brush, Point startPos, Point endPos)
{
	globalSim->CreateToolBox(startPos.X, startPos.Y, endPos.X, endPos.Y, ID, toolStrength);
}

PropTool::PropTool():
ToolTool(TOOL_PROP),
propType(Integer),
propOffset(0)
{
	propValue.Integer = 0;
}
int PropTool::DrawPoint(Brush* brush, Point position)
{
	globalSim->CreatePropBrush(position.X, position.Y, brush->GetRadius().X, brush->GetRadius().Y, propType, propValue, propOffset);
	return 0;
}
void PropTool::DrawLine(Brush* brush, Point startPos, Point endPos, bool held)
{
	globalSim->CreatePropLine(startPos.X, startPos.Y, endPos.X, endPos.Y, currentBrush->GetRadius().X, currentBrush->GetRadius().Y, propType, propValue, propOffset);
}
void PropTool::DrawRect(Brush* brush, Point startPos, Point endPos)
{
	globalSim->CreatePropBox(startPos.X, startPos.Y, endPos.X, endPos.Y, propType, propValue, propOffset);
}
int PropTool::FloodFill(Point position)
{
	return globalSim->FloodProp(position.X, position.Y, propType, propValue, propOffset);
}


DecoTool::DecoTool(int decoID):
Tool(DECO_TOOL, decoID, decoTypes[decoID].identifier)
{

}
int DecoTool::DrawPoint(Brush* brush, Point position)
{
	unsigned int col = (ID == DECO_CLEAR) ? PIXRGB(0, 0, 0) : decocolor;
	globalSim->CreateDecoBrush(position.X, position.Y, brush->GetRadius().X, brush->GetRadius().Y, ID, col);
	return 0;
}
void DecoTool::DrawLine(Brush* brush, Point startPos, Point endPos, bool held)
{
	unsigned int col = (ID == DECO_CLEAR) ? PIXRGB(0, 0, 0) : decocolor;
	globalSim->CreateDecoLine(startPos.X, startPos.Y, endPos.X, endPos.Y, brush->GetRadius().X, brush->GetRadius().Y, ID, col);
}
void DecoTool::DrawRect(Brush* brush, Point startPos, Point endPos)
{
	unsigned int col = (ID == DECO_CLEAR) ? PIXRGB(0, 0, 0) : decocolor;
	globalSim->CreateDecoBox(startPos.X, startPos.Y, endPos.X, endPos.Y, ID, col);
}
int DecoTool::FloodFill(Point position)
{
	PropertyValue col;
	col.UInteger = (ID == DECO_CLEAR) ? PIXRGB(0, 0, 0) : decocolor;
	return globalSim->FloodProp(position.X, position.Y, UInteger, col, offsetof(particle, dcolour));
}
Tool* DecoTool::Sample(Point position)
{
	if (position.Y < 0 || position.Y >= YRES || position.X < 0 || position.X >= XRES)
		return this;

	unsigned int tempcolor = vid_buf[(position.Y)*(XRES + BARSIZE) + (position.X)];
	int cr = PIXR(tempcolor);
	int cg = PIXG(tempcolor);
	int cb = PIXB(tempcolor);
	if (cr || cg || cb)
	{
		if (cr && cr<255) cr++;
		if (cg && cg<255) cg++;
		if (cb && cb<255) cb++;
		decocolor = (255 << 24) | PIXRGB(cr, cg, cb);
		currR = PIXR(decocolor), currG = PIXG(decocolor), currB = PIXB(decocolor), currA = decocolor >> 24;
		RGB_to_HSV(currR, currG, currB, &currH, &currS, &currV);
	}
	return this;
}
