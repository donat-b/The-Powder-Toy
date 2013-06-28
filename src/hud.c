#include <hud.h>
#include <defines.h>
#include <luaconsole.h>
#include <powder.h>
#include <gravity.h>
#include <time.h>

char uitext[512] = "";
char heattext[256] = "";
char coordtext[128] = "";
char tempstring[256] = "";
char timeinfotext[512] = "";
char infotext[512] = "";
int wavelength_gfx = 0;
int quickoptionsToolTipFadeInvert, it_invert = 0;

int hud_modnormal[HUD_OPTIONS];
int hud_moddebug[HUD_OPTIONS];
int hud_normal[HUD_OPTIONS];
int hud_debug[HUD_OPTIONS];
int hud_current[HUD_OPTIONS];

void hud_defaults()
{
	int hud_modnormal2[HUD_OPTIONS] = {1,0,1,0,0,0,0,0,1,0,1,0,0,0,0,1,0,0,2,0,0,0,0,2,0,2,1,2,0,0,0,2,0,2,0,2,0,0,0,0,0,0,2,0,2,1,0,0,1,1};
	int hud_moddebug2[HUD_OPTIONS] =  {1,1,1,2,1,0,0,0,1,0,1,1,1,0,0,1,0,0,4,1,0,0,0,4,0,4,1,4,1,1,1,4,0,4,0,4,0,0,0,0,0,0,4,0,4,1,0,0,1,1};
	int hud_normal2[HUD_OPTIONS] =    {0,0,1,0,0,0,0,0,1,1,1,0,0,1,1,1,0,0,2,0,0,0,0,2,0,2,1,2,0,0,0,2,0,2,0,2,0,0,0,0,0,0,2,0,2,1,0,0,0,0};
	int hud_debug2[HUD_OPTIONS] =     {0,1,1,0,1,0,1,1,1,1,1,1,0,1,1,1,0,0,2,1,1,0,0,2,0,2,1,2,1,1,1,2,0,2,0,2,0,0,0,0,0,0,2,0,2,1,0,0,0,0};
	memcpy(hud_modnormal,hud_modnormal2,sizeof(hud_modnormal));
	memcpy(hud_moddebug,hud_moddebug2,sizeof(hud_moddebug));
	memcpy(hud_normal,hud_normal2,sizeof(hud_normal));
	memcpy(hud_debug,hud_debug2,sizeof(hud_debug));
}

void set_current_hud()
{
	if (alt_hud == 1)
	{
		if (!DEBUG_MODE)
			memcpy(hud_current,hud_modnormal,sizeof(hud_current));
		else
			memcpy(hud_current,hud_moddebug,sizeof(hud_current));
	}
	else
	{
		if (!DEBUG_MODE)
			memcpy(hud_current,hud_normal,sizeof(hud_current));
		else
			memcpy(hud_current,hud_debug,sizeof(hud_current));
	}
}

void hud_text_right(int x, int y)
{
	sprintf(heattext,""); sprintf(coordtext,"");
	if (y>=0 && y<YRES && x>=0 && x<XRES)
	{
		int cr,wl = 0; //cr is particle under mouse, for drawing HUD information
		char nametext[50] = "";
		if (photons[y][x]) {
			cr = photons[y][x];
		} else {
			cr = pmap[y][x];
			if ((cr&0xFF) == PT_PINV && parts[cr>>8].tmp2)
				cr = parts[cr>>8].tmp2;
		}
		if (!cr || !hud_current[10])
		{
			wl = bmap[y/CELL][x/CELL];
		}
		sprintf(heattext,""); sprintf(tempstring,"");
		if (cr)
		{
			if (hud_current[10])
			{
				if ((cr&0xFF)==PT_LIFE && parts[cr>>8].ctype>=0 && parts[cr>>8].ctype<NGOL)
				{
					if (hud_current[49] || !hud_current[11])
						sprintf(nametext, "%s, ", gmenu[parts[cr>>8].ctype].name);
					else
						sprintf(nametext, "%s (%s), ", ptypes[cr&0xFF].name, gmenu[parts[cr>>8].ctype].name);
				}
				else if (hud_current[13] && (cr&0xFF)==PT_LAVA && parts[cr>>8].ctype > 0 && parts[cr>>8].ctype < PT_NUM )
				{
					sprintf(nametext, "Molten %s, ", ptypes[parts[cr>>8].ctype].name);
				}
				else if (hud_current[14] && hud_current[11] && ((cr&0xFF)==PT_PIPE || (cr&0xFF)==PT_PPIP) && (parts[cr>>8].tmp&0xFF) > 0 && (parts[cr>>8].tmp&0xFF) < PT_NUM )
				{
					sprintf(nametext, "PIPE (%s), ", ptypes[parts[cr>>8].tmp&0xFF].name);
				}
				else if (hud_current[11])
				{
					int tctype = parts[cr>>8].ctype;
					if ((cr&0xFF)==PT_PIPE && hud_current[12]) //PIPE Overrides CTP2
					{
						tctype = parts[cr>>8].tmp&0xFF;
					}
					if (!hud_current[12] && (tctype>=PT_NUM || tctype<0 || (cr&0xFF)==PT_PHOT))
						tctype = 0;
					if (tctype >= 0 && tctype < PT_NUM)
						sprintf(nametext, "%s (%s), ", ptypes[cr&0xFF].name, ptypes[tctype].name);
					else
						sprintf(nametext, "%s (%d), ", ptypes[cr&0xFF].name, tctype);
				}
				else
				{
					sprintf(nametext, "%s, ", ptypes[cr&0xFF].name);
				}
			}
			else if (hud_current[11])
			{
				if (parts[cr>>8].ctype > 0 && parts[cr>>8].ctype < PT_NUM)
					sprintf(nametext,"Ctype: %s ", ptypes[parts[cr>>8].ctype].name);
				else if (hud_current[12])
					sprintf(nametext,"Ctype: %d ", parts[cr>>8].ctype);
			}
			else if (wl && hud_current[48])
			{
				sprintf(nametext, "%s, ", wtypes[wl-UI_ACTUALSTART].name);
			}
			else
				sprintf(nametext,"");
			strncpy(heattext,nametext,50);
			if (hud_current[15])
			{
				sprintf(tempstring,"Temp: %0.*f C, ",hud_current[18],parts[cr>>8].temp-273.15f);
				strappend(heattext,tempstring);
			}
			if (hud_current[16])
			{
				sprintf(tempstring,"Temp: %0.*f F, ",hud_current[18],((parts[cr>>8].temp-273.15f)*9/5)+32);
				strappend(heattext,tempstring);
			}
			if (hud_current[17])
			{
				sprintf(tempstring,"Temp: %0.*f K, ",hud_current[18],parts[cr>>8].temp);
				strappend(heattext,tempstring);
			}
			if (hud_current[19])
			{
				sprintf(tempstring,"Life: %d, ",parts[cr>>8].life);
				strappend(heattext,tempstring);
			}
			if (hud_current[20])
			{
				sprintf(tempstring,"Tmp: %d, ",parts[cr>>8].tmp);
				strappend(heattext,tempstring);
			}
			if (hud_current[21])
			{
				sprintf(tempstring,"Tmp2: %d, ",parts[cr>>8].tmp2);
				strappend(heattext,tempstring);
			}
			if (hud_current[46])
			{
				sprintf(tempstring,"Dcolor: 0x%.8X, ",parts[cr>>8].dcolour);
				strappend(heattext,tempstring);
			}
			if (hud_current[47])
			{
				sprintf(tempstring,"Flags: 0x%.8X, ",parts[cr>>8].flags);
				strappend(heattext,tempstring);
			}
			if (hud_current[22])
			{
				sprintf(tempstring,"X: %0.*f, Y: %0.*f, ",hud_current[23],parts[cr>>8].x,hud_current[23],parts[cr>>8].y);
				strappend(heattext,tempstring);
			}
			if (hud_current[24])
			{
				sprintf(tempstring,"Vx: %0.*f, Vy: %0.*f, ",hud_current[25],parts[cr>>8].vx,hud_current[25],parts[cr>>8].vy);
				strappend(heattext,tempstring);
			}
			if ((cr&0xFF)==PT_PHOT && hud_current[45]) wavelength_gfx = parts[cr>>8].ctype;
		}
		else if (wl && hud_current[48])
		{
			sprintf(heattext, "%s, ", wtypes[wl-UI_ACTUALSTART].name);
		}
		else
		{
			if (hud_current[10])
				sprintf(heattext,"Empty, ");
		}
		if (hud_current[26])
		{
			sprintf(tempstring,"Pressure: %0.*f, ",hud_current[27],pv[y/CELL][x/CELL]);
			strappend(heattext,tempstring);
		}
		if (strlen(heattext) > 1)
			heattext[strlen(heattext)-2] = '\0'; // delete comma and space at end

		if (hud_current[28] && cr)
		{
			if (hud_current[29] || (ngrav_enable && hud_current[30]))
				sprintf(tempstring,"#%d, ",cr>>8);
			else
				sprintf(tempstring,"#%d ",cr>>8);
			strappend(coordtext,tempstring);
		}
		if (hud_current[29])
		{
			sprintf(tempstring,"X:%d Y:%d ",x,y);
			strappend(coordtext,tempstring);
		}
		if (hud_current[30] && ngrav_enable)
		{
			sprintf(tempstring,"GX: %0.*f GY: %0.*f ",hud_current[31],gravx[((y/CELL)*(XRES/CELL))+(x/CELL)],hud_current[31],gravy[((y/CELL)*(XRES/CELL))+(x/CELL)]);
			strappend(coordtext,tempstring);
		}
		if (hud_current[34] && aheat_enable)
		{
			sprintf(tempstring,"A.Heat: %0.*f K ",hud_current[35],hv[y/CELL][x/CELL]);
			strappend(coordtext,tempstring);
		}
		if (hud_current[32])
		{
			sprintf(tempstring,"Pressure: %0.*f",hud_current[33],pv[y/CELL][x/CELL]);
			strappend(coordtext,tempstring);
		}
		if (hud_current[43])
		{
			sprintf(tempstring,"VX: %0.*f VY: %0.*f",hud_current[44],vx[y/CELL][x/CELL],hud_current[44],vy[y/CELL][x/CELL]);
			strappend(coordtext,tempstring);
		}
		if (strlen(coordtext) > 0 && coordtext[strlen(coordtext)-1] == ' ')
			coordtext[strlen(coordtext)-1] = 0;
	}
	else
	{
		if (hud_current[10])
			sprintf(heattext, "Empty");
		if (hud_current[29])
			sprintf(coordtext, "X:%d Y:%d", x, y);
	}
}

void hud_text_left(float FPSB2, int it)
{
#ifdef BETA
	if (hud_current[0] && hud_current[1])
		sprintf(uitext, "Version %d Beta %d (%d) ", SAVE_VERSION, MINOR_VERSION, BUILD_NUM);
	else if (hud_current[0] && !hud_current[1])
		sprintf(uitext, "Version %d Beta %d ", SAVE_VERSION, MINOR_VERSION);
	else if (!hud_current[0] && hud_current[1])
		sprintf(uitext, "Beta Build %d ", BUILD_NUM);
#else
	if (hud_current[0] && hud_current[1])
		sprintf(uitext, "Version %d.%d (%d) ", SAVE_VERSION, MINOR_VERSION, BUILD_NUM);
	else if (hud_current[0] && !hud_current[1])
		sprintf(uitext, "Version %d.%d ", SAVE_VERSION, MINOR_VERSION);
	else if (!hud_current[0] && hud_current[1])
		sprintf(uitext, "Build %d ", BUILD_NUM);
#endif
	else
		sprintf(uitext,"");
	if (hud_current[36] || hud_current[37] || hud_current[38])
	{
		time_t time2 = time(0);
		char time[256], *timestr = "";
		sprintf(time,"%i",time2);

		if (strlen(uitext))
		{
			uitext[strlen(uitext)-1] = ',';
			strappend(uitext," ");
		}
		converttotime(time,&timestr,hud_current[36],hud_current[38],hud_current[37]);
		strappend(uitext,timestr);
		strappend(uitext,", ");
		free(timestr);
	}
	if (hud_current[2])
	{
		sprintf(tempstring,"FPS:%0.*f ",hud_current[3],FPSB2);
		strappend(uitext,tempstring);
	}
	if (hud_current[4])
	{
		sprintf(tempstring,"Parts:%d ",NUM_PARTS);
		strappend(uitext,tempstring);
	}
	if (hud_current[5])
	{
		sprintf(tempstring,"Generation:%d ",GENERATION);
		strappend(uitext,tempstring);
	}
	if (hud_current[6])
	{
		sprintf(tempstring,"Gravity:%d ",gravityMode);
		strappend(uitext,tempstring);
	}
	if (hud_current[7])
	{
		sprintf(tempstring,"Air:%d ",airMode);
		strappend(uitext,tempstring);
	}
	if (hud_current[39])
	{
		time_string(currentTime-totalafktime-afktime, timeinfotext, 2);
		sprintf(tempstring,"Time Played: %s ", timeinfotext);
		strappend(uitext,tempstring);
	}
	if (hud_current[40])
	{
		time_string(totaltime+currentTime-totalafktime-afktime, timeinfotext, 1);
		sprintf(tempstring,"Total Time Played: %s ", timeinfotext);
		strappend(uitext,tempstring);
	}
	if (hud_current[41] && frames)
	{
		sprintf(tempstring,"Average FPS: %0.*f ", hud_current[42], totalfps/frames);
		strappend(uitext,tempstring);
	}
	if (REPLACE_MODE && hud_current[8])
		strappend(uitext, "[REPLACE MODE] ");
	if (sdl_mod&(KMOD_CAPS) && hud_current[8])
		strappend(uitext, "[CAPS LOCK] ");
	if ((finding && finding != 8) && hud_current[8])
		strappend(uitext, "[FIND] ");
	if (GRID_MODE && hud_current[9])
	{
		sprintf(tempstring, "[GRID: %d] ", GRID_MODE);
		strappend(uitext, tempstring);
	}
	if (active_menu == SC_DECO && ISANIM)
	{
		sprintf(tempstring,"[Frame %i/%i] ",framenum+1,maxframes);
		strappend(uitext, tempstring);
		if (!sys_pause && !framerender)
			ISANIM = 0;
	}
#ifdef INTERNAL
	if (vs)
		strappend(uitext, "[FRAME CAPTURE]");
#endif
	if (strlen(uitext) > 0 && uitext[strlen(uitext)-1] == ' ')
		uitext[strlen(uitext)-1] = 0;
}

void draw_hud(int it)
{
	int heatlength = textwidth(heattext);
	int coordlength = textwidth(coordtext);
	int heatx, heaty, alpha;
	quickoptionsToolTipFadeInvert = 255 - (quickoptionsToolTipFade*20);
	it_invert = 50 - it;
	if(it_invert < 0)
		it_invert = 0;
	if(it_invert > 50)
		it_invert = 50;
	if (strlen(uitext) > 0)
	{
		fillrect(vid_buf, 12, 12, textwidth(uitext)+8, 15, 0, 0, 0, (int)(it_invert*2.5));
		drawtext(vid_buf, 16, 16, uitext, 32, 216, 255, it_invert * 4);
	}
	if (sdl_zoom_trig||zoom_en)
	{
		if (zoom_x<XRES/2)
			heatx = XRES-16-heatlength;
		else
			heatx = 16;
		heaty = 270;
		alpha = 127;
	}
	else
	{
		heatx = XRES-16-heatlength;
		heaty = 16;
		alpha = (int)(quickoptionsToolTipFadeInvert*0.5);
	}
	if (strlen(heattext) > 0)
	{
		fillrect(vid_buf, heatx-4, heaty-4, heatlength+8, 15, 0, 0, 0, alpha);
		drawtext(vid_buf, heatx, heaty, heattext, 255, 255, 255, (int)(alpha*1.5f));
		if (wavelength_gfx)
			draw_wavelengths(vid_buf,heatx-4,heaty-5,2,wavelength_gfx);
	}
	if (DEBUG_MODE && strlen(coordtext) > 0)
	{
		if ((sdl_zoom_trig||zoom_en) && zoom_x>=XRES/2)
		{
			if (coordlength > heatlength)
			{
				fillrect(vid_buf, 19+heatlength, 277, coordlength-heatlength, 15, 0, 0, 0, alpha);
				fillrect(vid_buf, 12, 280, heatlength+8, 12, 0, 0, 0, alpha);
			}
			else
				fillrect(vid_buf, 12, 280, coordlength+8, 12, 0, 0, 0, alpha);
			drawtext(vid_buf, 16, 281, coordtext, 255, 255, 255, (int)(alpha*1.5f));
		}
		else
		{
			if (coordlength > heatlength)
			{
				fillrect(vid_buf, XRES-20-coordlength, heaty+7, coordlength-heatlength+1, 15, 0, 0, 0, alpha);
				fillrect(vid_buf, XRES-20-heatlength, heaty+10, heatlength+8, 12, 0, 0, 0, alpha);
			}
			else
				fillrect(vid_buf, XRES-20-coordlength, heaty+10, coordlength+8, 12, 0, 0, 0, alpha);
			drawtext(vid_buf, XRES-16-coordlength, heaty+11, coordtext, 255, 255, 255, (int)(alpha*1.5f));
		}
	}
	wavelength_gfx = 0;
}

//draws the photon colors in the HUD
void draw_wavelengths(pixel *vid, int x, int y, int h, int wl)
{
	int i,cr,cg,cb,j;
	int tmp;
	fillrect(vid,x-1,y-1,30+1,h+1,64,64,64,255); // coords -1 size +1 to work around bug in fillrect - TODO: fix fillrect
	for (i=0; i<30; i++)
	{
		if ((wl>>i)&1)
		{
			// Need a spread of wavelengths to get a smooth spectrum, 5 bits seems to work reasonably well
			if (i>2) tmp = 0x1F << (i-2);
			else tmp = 0x1F >> (2-i);
			cg = 0;
			cb = 0;
			cr = 0;
			for (j=0; j<12; j++) {
				cr += (tmp >> (j+18)) & 1;
				cb += (tmp >>  j)     & 1;
			}
			for (j=0; j<13; j++)
				cg += (tmp >> (j+9))  & 1;
			tmp = 624/(cr+cg+cb+1);
			cr *= tmp;
			cg *= tmp;
			cb *= tmp;
			for (j=0; j<h; j++) blendpixel(vid,x+29-i,y+j,cr>255?255:cr,cg>255?255:cg,cb>255?255:cb,255);
		}
	}
}

void draw_info()
{
	int ytop = 230, num_parts = 0, totalselected = 0, i, x, y;
	float totaltemp = 0, totalpressure = 0;
	for (i=0; i<NPART; i++)
	{
		if (parts[i].type)
		{
			totaltemp += parts[i].temp;
			num_parts++;
		}
		if (parts[i].type == sl)
			totalselected++;
	}
	for (y=0; y<YRES/CELL; y++)
		for (x=0; x<XRES/CELL; x++)
		{
			totalpressure += pv[y][x];
		}

		time_string(currentTime-totalafktime-afktime, timeinfotext, 0);
		sprintf(infotext,"Time Played: %s", timeinfotext);
		fillrect(vid_buf, 12, ytop-4, textwidth(infotext)+8, 15, 0, 0, 0, 140);
		drawtext(vid_buf, 16, ytop, infotext, 255, 255, 255, 200);
		time_string(totaltime+currentTime-totalafktime-afktime, timeinfotext, 0);
		sprintf(infotext,"Total Time Played: %s", timeinfotext);
		fillrect(vid_buf, 12, ytop+10, textwidth(infotext)+8, 15, 0, 0, 0, 140);
		drawtext(vid_buf, 16, ytop+14, infotext, 255, 255, 255, 200);
		time_string(totalafktime+afktime+prevafktime, timeinfotext, 0);
		sprintf(infotext,"Total AFK Time: %s", timeinfotext);
		fillrect(vid_buf, 12, ytop+24, textwidth(infotext)+8, 15, 0, 0, 0, 140);
		drawtext(vid_buf, 16, ytop+28, infotext, 255, 255, 255, 200);
		if (frames)
		{
			sprintf(infotext,"Average FPS: %f", totalfps/frames);
			fillrect(vid_buf, 12, ytop+38, textwidth(infotext)+8, 15, 0, 0, 0, 140);
			drawtext(vid_buf, 16, ytop+42, infotext, 255, 255, 255, 200);
		}
		sprintf(infotext,"Max FPS: %f", maxfps);
		fillrect(vid_buf, 12, ytop+52, textwidth(infotext)+8, 15, 0, 0, 0, 140);
		drawtext(vid_buf, 16, ytop+56, infotext, 255, 255, 255, 200);
		sprintf(infotext,"Number of Times Played: %i", timesplayed);
		fillrect(vid_buf, 12, ytop+66, textwidth(infotext)+8, 15, 0, 0, 0, 140);
		drawtext(vid_buf, 16, ytop+70, infotext, 255, 255, 255, 200);
		if (timesplayed)
		{
			time_string((totaltime+currentTime-totalafktime-afktime)/timesplayed, timeinfotext, 0);
			sprintf(infotext,"Average Time Played: %s", timeinfotext);
			fillrect(vid_buf, 12, ytop+80, textwidth(infotext)+8, 15, 0, 0, 0, 140);
			drawtext(vid_buf, 16, ytop+84, infotext, 255, 255, 255, 200);
		}
		if (num_parts)
		{
			sprintf(infotext,"Average Temp: %f C", totaltemp/num_parts-273.15f);
			fillrect(vid_buf, 12, ytop+94, textwidth(infotext)+8, 15, 0, 0, 0, 140);
			drawtext(vid_buf, 16, ytop+98, infotext, 255, 255, 255, 200);
		}
		sprintf(infotext,"Average Pressure: %f", totalpressure/(XRES*YRES/CELL/CELL));
		fillrect(vid_buf, 12, ytop+108, textwidth(infotext)+8, 15, 0, 0, 0, 140);
		drawtext(vid_buf, 16, ytop+112, infotext, 255, 255, 255, 200);
		if (num_parts && sl >= 0 && sl < PT_NUM)
		{
			if (sl != 0)
				sprintf(infotext,"%%%s: %f", ptypes[sl].name,(float)totalselected/num_parts*100);
			else
				sprintf(infotext,"%%Empty: %f", (float)totalselected/XRES/YRES*100);
			fillrect(vid_buf, 12, ytop+122, textwidth(infotext)+8, 15, 0, 0, 0, 140);
			drawtext(vid_buf, 16, ytop+126, infotext, 255, 255, 255, 200);
		}
}

void draw_lua_logs()
{
	int i;
	for (i = 0; i < 20; i++)
	{
		if (log_history[i])
		{
			int alpha = log_history_times[i]*5>255?255:log_history_times[i]*5;
			drawtext_outline(vid_buf, 16, (YRES-16)-i*12, log_history[i], 255, 255, 255, alpha, 0, 0, 0, alpha);
			log_history_times[i]--;
			if (log_history_times[i] < 0)
			{
				free(log_history[i]);
				log_history[i] = NULL;
			}
		}
	}
}

void time_string(int currtime, char *string, int length)
{
	int years, days, hours, minutes, seconds, milliseconds;
	if (length == 0)
	{
		years = currtime/31557600000;
		currtime = currtime%31557600000;
		days = currtime/86400000;
		currtime = currtime%86400000;
	}
	if (length <= 1)
	{
		hours = currtime/3600000;
		currtime = currtime%3600000;
	}
	minutes = currtime/60000;
	currtime = currtime%60000;
	seconds = currtime/1000;
	currtime = currtime%1000;
	milliseconds = currtime;
	if (length == 0)
		sprintf(string,"%i year%s, %i day%s, %i hour%s, %i minute%s, %i second%s, %i millisecond%s",years,(years == 1)?"":"s",days,(days == 1)?"":"s",hours,(hours == 1)?"":"s",minutes,(minutes == 1)?"":"s",seconds,(seconds == 1)?"":"s",milliseconds,(milliseconds == 1)?"":"s");
	else if (length == 1)
		sprintf(string,"%i hour%s, %i minute%s, %i second%s",hours,(hours == 1)?"":"s",minutes,(minutes == 1)?"":"s",seconds,(seconds == 1)?"":"s");
	else if (length == 2)
		sprintf(string,"%i minute%s, %i second%s",minutes,(minutes == 1)?"":"s",seconds,(seconds == 1)?"":"s");
}
