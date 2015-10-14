#include <sstream>
#include "Tool.h"
#include "defines.h"
#include "graphics.h"
#include "interface.h"
#include "powder.h"
#include "graphics/ARGBColour.h"
#include "Simulation.h"
#include "WallNumbers.h"
#include "ToolNumbers.h"
#include "GolNumbers.h"
#include "game/Brush.h"

Tool::Tool(int toolID, std::string toolIdentifier):
	identifier(toolIdentifier),
	type(INVALID_TOOL),
	ID(toolID)
{

}

Tool::Tool(int toolType, int toolID, std::string toolIdentifier):
	identifier(toolIdentifier),
	type(toolType),
	ID(toolID)
{

}

int Tool::DrawPoint(Simulation *sim, Brush *brush, Point position, float toolStrength)
{
	return sim->CreateParts(position.X, position.Y, ID, get_brush_flags(), true, brush);
}

void Tool::DrawLine(Simulation *sim, Brush *brush, Point startPos, Point endPos, bool held, float toolStrength)
{
	if (held)
	{
#ifndef NOMOD
		if (ID == PT_MOVS)
			return;
#endif
	}
	else
	{
		if (ID == PT_LIGH)
			return;
	}
	sim->CreateLine(startPos.X, startPos.Y, endPos.X, endPos.Y, ID, get_brush_flags(), brush);
}

void Tool::DrawRect(Simulation *sim, Brush *brush, Point startPos, Point endPos)
{
	switch (ID)
	{
	case PT_LIGH:
		return;
	case PT_TESC:
	{
		int radiusInfo = brush->GetRadius().X*4+brush->GetRadius().Y*4+7;
		sim->CreateBox(startPos.X, startPos.Y, endPos.X, endPos.Y, ID | (radiusInfo<<8), get_brush_flags());
		break;
	}
	default:
		sim->CreateBox(startPos.X, startPos.Y, endPos.X, endPos.Y, ID, get_brush_flags());
		break;
	}
}

int Tool::FloodFill(Simulation *sim, Brush *brush, Point position)
{
	switch (ID)
	{
	case PT_LIGH:
#ifndef NOMOD
	case PT_PWHT:
	case PT_MOVS:
#endif
		return 0;
	case PT_TESC:
	{
		int radiusInfo = brush->GetRadius().X*4+brush->GetRadius().Y*4+7;
		return sim->FloodParts(position.X, position.Y, ID | (radiusInfo<<8), -1, get_brush_flags());
	}
	default:
		return sim->FloodParts(position.X, position.Y, ID, -1, get_brush_flags());
	}
}

void Tool::Click(Simulation *sim, Point position)
{

}

Tool* Tool::Sample(Simulation *sim, Point position)
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
			return GetToolFromIdentifier(sim->elements[sample&0xFF].Identifier);
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


PlopTool::PlopTool(int elementID):
	ElementTool(elementID)
{

}
int PlopTool::DrawPoint(Simulation *sim, Brush* brush, Point position, float toolStrength)
{
	return 0;
}
void PlopTool::DrawLine(Simulation *sim, Brush *brush, Point startPos, Point endPos, bool held, float toolStrength)
{

}

void PlopTool::DrawRect(Simulation *sim, Brush *brush, Point startPos, Point endPos)
{

}

int PlopTool::FloodFill(Simulation *sim, Brush *brush, Point position)
{
	return 0;
}
void PlopTool::Click(Simulation *sim, Point position)
{
	sim->part_create(-1, position.X, position.Y, ID);
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
int GolTool::DrawPoint(Simulation *sim, Brush* brush, Point position, float toolStrength)
{
	return sim->CreateParts(position.X, position.Y, PT_LIFE | (ID<<8), get_brush_flags(), true, brush);
}
void GolTool::DrawLine(Simulation *sim, Brush *brush, Point startPos, Point endPos, bool held, float toolStrength)
{
	sim->CreateLine(startPos.X, startPos.Y, endPos.X, endPos.Y, PT_LIFE | (ID<<8), get_brush_flags(), brush);
}
void GolTool::DrawRect(Simulation *sim, Brush *brush, Point startPos, Point endPos)
{
	sim->CreateBox(startPos.X, startPos.Y, endPos.X, endPos.Y, PT_LIFE | (ID<<8), get_brush_flags());
}
int GolTool::FloodFill(Simulation *sim, Brush *brush, Point position)
{
	return sim->FloodParts(position.X, position.Y, PT_LIFE+(ID<<8), -1, get_brush_flags());
}


WallTool::WallTool(int wallID):
	Tool(WALL_TOOL, wallID, wallTypes[wallID].identifier)
{

}
int WallTool::DrawPoint(Simulation *sim, Brush* brush, Point position, float toolStrength)
{
	int rx = brush->GetRadius().X/CELL;
	int ry = brush->GetRadius().Y/CELL;
	int x = position.X/CELL;
	int y = position.Y/CELL;
	sim->CreateWallBox(x-rx, y-ry, x+rx, y+ry, ID);
	return 1;
}
void WallTool::DrawLine(Simulation *sim, Brush *brush, Point startPos, Point endPos, bool held, float toolStrength)
{
	if (!held && ID == WL_FAN && bmap[startPos.Y/CELL][startPos.X/CELL] == WL_FAN)
	{
		float nfvx = (endPos.X-startPos.X)*0.005f;
		float nfvy = (endPos.Y-startPos.Y)*0.005f;
		sim->FloodWalls(startPos.X/CELL, startPos.Y/CELL, WL_FANHELPER, WL_FAN);
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
		sim->CreateWallLine(startPos.X/CELL, startPos.Y/CELL, endPos.X/CELL, endPos.Y/CELL, brush->GetRadius().X/CELL, brush->GetRadius().Y/CELL, ID);
	}
}
void WallTool::DrawRect(Simulation *sim, Brush *brush, Point startPos, Point endPos)
{
	sim->CreateWallBox(startPos.X/CELL, startPos.Y/CELL, endPos.X/CELL, endPos.Y/CELL, ID);
}
int WallTool::FloodFill(Simulation *sim, Brush *brush, Point position)
{
	return sim->FloodWalls(position.X/CELL, position.Y/CELL, ID, -1);
}

StreamlineTool::StreamlineTool():
	WallTool(WL_STREAM)
{

}
int StreamlineTool::DrawPoint(Simulation *sim, Brush* brush, Point position, float toolStrength)
{
	sim->CreateWall(position.X, position.Y, ID);
	return 0;
}
void StreamlineTool::DrawLine(Simulation *sim, Brush *brush, Point startPos, Point endPos, bool held, float toolStrength)
{
	sim->CreateWallLine(startPos.X/CELL, startPos.Y/CELL, endPos.X/CELL, endPos.Y/CELL, 0, 0, ID);
}
int StreamlineTool::FloodFill(Simulation *sim, Brush *brush, Point position)
{
	return 0;
}

ToolTool::ToolTool(int toolID):
	Tool(TOOL_TOOL, toolID, toolTypes[toolID].identifier)
{

}
int ToolTool::DrawPoint(Simulation *sim, Brush* brush, Point position, float toolStrength)
{
	if (ID == TOOL_SIGN)
		return 1;
	sim->CreateToolBrush(position.X, position.Y, ID, toolStrength, brush);
	return 0;
}
void ToolTool::DrawLine(Simulation *sim, Brush *brush, Point startPos, Point endPos, bool held, float toolStrength)
{
	if (ID == TOOL_SIGN)
		return;
	if (ID == TOOL_WIND)
		toolStrength = held ? 0.01f : 0.002f;
	sim->CreateToolLine(startPos.X, startPos.Y, endPos.X, endPos.Y, ID, toolStrength, brush);
}
void ToolTool::DrawRect(Simulation *sim, Brush *brush, Point startPos, Point endPos)
{
	if (ID == TOOL_SIGN)
		return;
	sim->CreateToolBox(startPos.X, startPos.Y, endPos.X, endPos.Y, ID, toolStrength);
}
int ToolTool::FloodFill(Simulation *sim, Brush *brush, Point position)
{
	return 0;
}
void ToolTool::Click(Simulation *sim, Point position)
{
	if (ID == TOOL_SIGN)
	{
		openSign = true;
	}
}

PropTool::PropTool():
ToolTool(TOOL_PROP),
propType(Integer),
propOffset(0)
{
	propValue.Integer = 0;
}
int PropTool::DrawPoint(Simulation *sim, Brush* brush, Point position, float toolStrength)
{
	sim->CreatePropBrush(position.X, position.Y, propType, propValue, propOffset, brush);
	return 0;
}
void PropTool::DrawLine(Simulation *sim, Brush *brush, Point startPos, Point endPos, bool held, float toolStrength)
{
	sim->CreatePropLine(startPos.X, startPos.Y, endPos.X, endPos.Y, propType, propValue, propOffset, brush);
}
void PropTool::DrawRect(Simulation *sim, Brush *brush, Point startPos, Point endPos)
{
	sim->CreatePropBox(startPos.X, startPos.Y, endPos.X, endPos.Y, propType, propValue, propOffset);
}
int PropTool::FloodFill(Simulation *sim, Brush *brush, Point position)
{
	return sim->FloodProp(position.X, position.Y, propType, propValue, propOffset);
}


DecoTool::DecoTool(int decoID):
Tool(DECO_TOOL, decoID, decoTypes[decoID].identifier)
{

}
int DecoTool::DrawPoint(Simulation *sim, Brush* brush, Point position, float toolStrength)
{
	ARGBColour col = (ID == DECO_CLEAR) ? COLARGB(0, 0, 0, 0) : decocolor;
	sim->CreateDecoBrush(position.X, position.Y, ID, col, brush);
	return 0;
}
void DecoTool::DrawLine(Simulation *sim, Brush *brush, Point startPos, Point endPos, bool held, float toolStrength)
{
	ARGBColour col = (ID == DECO_CLEAR) ? COLARGB(0, 0, 0, 0) : decocolor;
	sim->CreateDecoLine(startPos.X, startPos.Y, endPos.X, endPos.Y, ID, col, brush);
}
void DecoTool::DrawRect(Simulation *sim, Brush *brush, Point startPos, Point endPos)
{
	ARGBColour col = (ID == DECO_CLEAR) ? COLARGB(0, 0, 0, 0) : decocolor;
	sim->CreateDecoBox(startPos.X, startPos.Y, endPos.X, endPos.Y, ID, col);
}
int DecoTool::FloodFill(Simulation *sim, Brush *brush, Point position)
{
	pixel rep = vid_buf[position.X+position.Y*(XRES+BARSIZE)];
	unsigned int col = (ID == DECO_CLEAR) ? COLARGB(0, 0, 0, 0) : decocolor;
	sim->FloodDeco(vid_buf, position.X, position.Y, col, PIXCONV(rep));
}
Tool* DecoTool::Sample(Simulation *sim, Point position)
{
	if (position.Y < 0 || position.Y >= YRES || position.X < 0 || position.X >= XRES)
		return this;

	int cr = PIXR(sampleColor);
	int cg = PIXG(sampleColor);
	int cb = PIXB(sampleColor);

	if (cr && cr<255) cr++;
	if (cg && cg<255) cg++;
	if (cb && cb<255) cb++;
	decocolor = COLARGB(255, cr, cg, cb);
	currR = cr, currG = cg, currB = cb, currA = 255;
	RGB_to_HSV(currR, currG, currB, &currH, &currS, &currV);
	return this;
}
