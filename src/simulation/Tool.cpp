#include "Tool.h"
#include "interface.h"
#include "powder.h"
#include "Simulation.h"
#include "WallNumbers.h"
#include "game/Brush.h"

Tool::Tool(int toolType, int toolID, std::string toolIdentifier):
	type(toolType),
	ID(toolID),
	identifier(toolIdentifier),
	strength(1.0f)
{

}

int Tool::DrawPoint(Brush* brush, Point position)
{
	if (type == ELEMENT_TOOL)
	{
		if (ID == PT_FIGH)
			create_part(-1, position.X, position.Y, PT_FIGH);
		else if (globalSim->elements[ID].Properties&PROP_MOVS)
			create_moving_solid(position.X, position.Y, brush->GetRadius().X, brush->GetRadius().Y, ID);
		else
			return create_parts(position.X, position.Y, brush->GetRadius().X, brush->GetRadius().Y, ID, get_brush_flags(), 1);
	}
	else if (type == GOL_TOOL)
		create_parts(position.X, position.Y, brush->GetRadius().X, brush->GetRadius().Y, PT_LIFE+(ID<<8), get_brush_flags(), 1);
	else if (GetWallID() == WL_SIGN+100 || MSIGN!=-1) // if sign tool is selected or a sign is being moved
	{
		add_sign_ui(vid_buf, position.X, position.Y);
	}
	else if (type == WALL_TOOL)
	{
		int rx = brush->GetRadius().X/CELL;
		int ry = brush->GetRadius().Y/CELL;
		int x = position.X/CELL-rx/2;
		int y = position.Y/CELL-ry/2;
		globalSim->CreateWallBox(x, y, x+rx, y+ry, ID);
	}
	else
		create_parts(position.X, position.Y, brush->GetRadius().X, brush->GetRadius().Y, ID, get_brush_flags(), 1);
	return 0;
}

int Tool::DrawLine(Brush* brush, Point startPos, Point endPos, bool held)
{
	if (type == GOL_TOOL)
		create_line(startPos.X, startPos.Y, endPos.X, endPos.Y, currentBrush->GetRadius().X, currentBrush->GetRadius().Y, PT_LIFE+(ID<<8), get_brush_flags());
	else if (!held && GetWallID() == WL_FAN && bmap[startPos.Y/CELL][startPos.X/CELL] == WL_FAN)
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
	else if (GetToolID() == SPC_WIND)
	{
		for (int j = -brush->GetRadius().Y; j <= brush->GetRadius().Y; j++)
			for (int i = -brush->GetRadius().X; i <= brush->GetRadius().X; i++)
				if (endPos.X+i>0 && endPos.Y+j>0 && endPos.X+i<XRES && endPos.Y+j<YRES && InCurrentBrush(i, j, brush->GetRadius().X, brush->GetRadius().Y))
				{
					vx[(endPos.Y+j)/CELL][(endPos.X+i)/CELL] += (endPos.X-startPos.X)*0.01f;
					vy[(endPos.Y+j)/CELL][(endPos.X+i)/CELL] += (endPos.Y-startPos.Y)*0.01f;
				}
	}
	else if (held && type == ELEMENT_TOOL && (globalSim->elements[ID].Properties&PROP_MOVS))
		return 0;
	else if (type == WALL_TOOL)
	{
		int rx = brush->GetRadius().X/CELL;
		int ry = brush->GetRadius().Y/CELL;
		globalSim->CreateWallLine(startPos.X/CELL-rx/2, startPos.Y/CELL-ry/2, endPos.X/CELL-rx/2, endPos.Y/CELL-ry/2, rx, ry, ID);
	}
	else
		create_line(startPos.X, startPos.Y, endPos.X, endPos.Y, currentBrush->GetRadius().X, currentBrush->GetRadius().Y, ID, get_brush_flags());
	return 0;
}

void Tool::DrawRect(Brush* brush, Point startPos, Point endPos)
{
	if (type == GOL_TOOL)
		create_box(startPos.X, startPos.Y, endPos.X, endPos.Y, PT_LIFE+(ID<<8), get_brush_flags());
	else if (type == WALL_TOOL)
	{
		globalSim->CreateWallBox(startPos.X/CELL, startPos.Y/CELL, endPos.X/CELL, endPos.Y/CELL, ID);
	}
	else
		create_box(startPos.X, startPos.Y, endPos.X, endPos.Y, ID, get_brush_flags());
}

int Tool::FloodFill(Point position)
{
	if (type == WALL_TOOL || GetToolID() == SPC_PROP)
	{
		if (ID != WL_STREAM+100)
			globalSim->FloodWalls(position.X/CELL, position.Y/CELL, ID, -1);
	}
	else if (type == DECO_TOOL)
	{
		unsigned int col = (ID == DECO_ERASE) ? decocolor : PIXRGB(0, 0, 0);
		flood_prop(position.X, position.Y, offsetof(particle, dcolour), &col, 0);
	}
	else if (type == GOL_TOOL)
		return FloodParts(position.X, position.Y, PT_LIFE+(ID<<8), -1, get_brush_flags());
	else if (type == ELEMENT_TOOL || type == TOOL_TOOL)
		return FloodParts(position.X, position.Y, ID, -1, get_brush_flags());
	return 0;
}
