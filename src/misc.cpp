/**
 * Powder Toy - miscellaneous functions
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#ifdef WIN
#include <shlobj.h>
#include <shlwapi.h>
#include <windows.h>
#include <direct.h>
#else
#include <unistd.h>
#endif
#ifdef MACOSX
#include <ApplicationServices/ApplicationServices.h>
#endif
#include <sstream>
#include <math.h>
#ifdef ANDROID
#include <SDL/SDL_screenkeyboard.h>
#endif

#ifdef LIN
#include "images.h"
#endif
#include "misc.h"
#include "defines.h"
#include "interface.h"
#include "graphics.h"
#include "powdergraphics.h"
#include "powder.h"
#include "gravity.h"
#include "hud.h"
#include "cJSON.h"
#include "update.h"

#include "game/Menus.h"
#include "simulation/Simulation.h"
#include "simulation/Tool.h"

#ifdef MACOSX
extern "C"
{
char * readUserPreferences();
void writeUserPreferences(const char * prefData);
char * readClipboard();
void writeClipboard(const char * clipboardData);
}
#endif

char *clipboard_text = NULL;
static char hex[] = "0123456789ABCDEF";

//Signum function
TPT_GNU_INLINE int isign(float i)
{
	if (i<0)
		return -1;
	if (i>0)
		return 1;
	return 0;
}

TPT_GNU_INLINE unsigned clamp_flt(float f, float min, float max)
{
	if (f<min)
		return 0;
	if (f>max)
		return 255;
	return (int)(255.0f*(f-min)/(max-min));
}

TPT_GNU_INLINE float restrict_flt(float f, float min, float max)
{
	if (f<min)
		return min;
	if (f>max)
		return max;
	return f;
}

char *mystrdup(const char *s)
{
	char *x;
	if (s)
	{
		x = (char*)malloc(strlen(s)+1);
		strcpy(x, s);
		return x;
	}
	return NULL;
}

void strlist_add(struct strlist **list, char *str)
{
	struct strlist *item = (struct strlist *)malloc(sizeof(struct strlist));
	item->str = mystrdup(str);
	item->next = *list;
	*list = item;
}

int strlist_find(struct strlist **list, char *str)
{
	struct strlist *item;
	for (item=*list; item; item=item->next)
		if (!strcmp(item->str, str))
			return 1;
	return 0;
}

void strlist_free(struct strlist **list)
{
	struct strlist *item;
	while (*list)
	{
		item = *list;
		*list = (*list)->next;
		free(item);
	}
}

void clean_text(char *text, int vwidth)
{
	if (vwidth >= 0 && textwidth(text) > vwidth)
		text[textwidthx(text, vwidth)] = 0;	

	for (unsigned i = 0; i < strlen(text); i++)
		if (text[i] < ' ' || text[i] >= 127)
			text[i] = ' ';
}

void save_console_history(cJSON **historyArray, command_history *commandList)
{
	if (!commandList)
		return;
	save_console_history(historyArray, commandList->prev_command);
	cJSON_AddItemToArray(*historyArray, cJSON_CreateString(commandList->command.str));
}

void cJSON_AddString(cJSON** obj, const char *name, int number)
{
	std::stringstream str;
	str << number;
	cJSON_AddStringToObject(*obj, name, str.str().c_str());
}

void save_presets(int do_update)
{
	char * outputdata;
	char mode[32];
	cJSON *root, *userobj, *versionobj, *recobj, *graphicsobj, *hudobj, *simulationobj, *consoleobj, *tmpobj;
#ifndef MACOSX
	FILE *f = fopen("powder.pref", "wb");
	if(!f)
		return;
#endif
	root = cJSON_CreateObject();
	
	cJSON_AddStringToObject(root, "Powder Toy Preferences", "Don't modify this file unless you know what you're doing. P.S: editing the admin/mod fields in your user info doesn't give you magical powers");
	
	//Tpt++ User Info
	if (svf_login)
	{
		cJSON_AddItemToObject(root, "User", userobj=cJSON_CreateObject());
		cJSON_AddStringToObject(userobj, "Username", svf_user);
		cJSON_AddNumberToObject(userobj, "ID", atoi(svf_user_id));
		cJSON_AddStringToObject(userobj, "SessionID", svf_session_id);
		cJSON_AddStringToObject(userobj, "SessionKey", svf_session_key);
		if (svf_admin)
		{
			cJSON_AddStringToObject(userobj, "Elevation", "Admin");
		}
		else if (svf_mod)
		{
			cJSON_AddStringToObject(userobj, "Elevation", "Mod");
		}
		else {
			cJSON_AddStringToObject(userobj, "Elevation", "None");
		}
	}

	//Tpt++ Renderer settings
	cJSON_AddItemToObject(root, "Renderer", graphicsobj=cJSON_CreateObject());
	cJSON_AddString(&graphicsobj, "ColourMode", colour_mode);
	if (DEBUG_MODE)
		cJSON_AddTrueToObject(graphicsobj, "DebugMode");
	else
		cJSON_AddFalseToObject(graphicsobj, "DebugMode");
	tmpobj = cJSON_CreateStringArray(NULL, 0);
	int i = 0;
	while (display_modes[i])
	{
		sprintf(mode, "%x", display_modes[i]);
		cJSON_AddItemToArray(tmpobj, cJSON_CreateString(mode));
		i++;
	}
	cJSON_AddItemToObject(graphicsobj, "DisplayModes", tmpobj);
	tmpobj = cJSON_CreateStringArray(NULL, 0);
	i = 0;
	while (render_modes[i])
	{
		sprintf(mode, "%x", render_modes[i]);
		cJSON_AddItemToArray(tmpobj, cJSON_CreateString(mode));
		i++;
	}
	cJSON_AddItemToObject(graphicsobj, "RenderModes", tmpobj);
	if (drawgrav_enable)
		cJSON_AddTrueToObject(graphicsobj, "GravityField");
	else
		cJSON_AddFalseToObject(graphicsobj, "GravityField");
	if (decorations_enable)
		cJSON_AddTrueToObject(graphicsobj, "Decorations");
	else
		cJSON_AddFalseToObject(graphicsobj, "Decorations");
	
	//Tpt++ Simulation setting(s)
	cJSON_AddItemToObject(root, "Simulation", simulationobj=cJSON_CreateObject());
	cJSON_AddString(&simulationobj, "EdgeMode", edgeMode);
	cJSON_AddString(&simulationobj, "NewtonianGravity", ngrav_enable);
	cJSON_AddString(&simulationobj, "AmbientHeat", aheat_enable);
	cJSON_AddString(&simulationobj, "PrettyPowder", pretty_powder);

	//Tpt++ install check, prevents annoyingness
	cJSON_AddTrueToObject(root, "InstallCheck");

	//Tpt++ console history
	cJSON_AddItemToObject(root, "Console", consoleobj=cJSON_CreateObject());
	tmpobj = cJSON_CreateStringArray(NULL, 0);
	save_console_history(&tmpobj, last_command);
	cJSON_AddItemToObject(consoleobj, "History", tmpobj);
	tmpobj = cJSON_CreateStringArray(NULL, 0);
	save_console_history(&tmpobj, last_command_result);
	cJSON_AddItemToObject(consoleobj, "HistoryResults", tmpobj);

	//Version Info
	cJSON_AddItemToObject(root, "version", versionobj=cJSON_CreateObject());
	cJSON_AddNumberToObject(versionobj, "major", SAVE_VERSION);
	cJSON_AddNumberToObject(versionobj, "minor", MINOR_VERSION);
	cJSON_AddNumberToObject(versionobj, "build", BUILD_NUM);
	if (do_update)
	{
		cJSON_AddTrueToObject(versionobj, "update");
	}
	else
	{
		cJSON_AddFalseToObject(versionobj, "update");
	}
	
	//Fav Menu/Records
	cJSON_AddItemToObject(root, "records", recobj=cJSON_CreateObject());
	cJSON_AddNumberToObject(recobj, "Favorited Elements", locked);
	char* tempFavMenu[18];
	for (int j = 0; j < 18; j++) //this might be better with JSON and not cJSON
	{
		tempFavMenu[j] = new char[favMenu[j].length()+1];
		strncpy(tempFavMenu[j], favMenu[j].c_str(), favMenu[j].length()+1);
	}
	cJSON_AddItemToObject(recobj, "Recent Elements", cJSON_CreateStringArray((const char**)tempFavMenu, 18));
	for (int j = 0; j < 18; j++)
		delete[] tempFavMenu[j];
	cJSON_AddNumberToObject(recobj, "Total Time Played", ((double)currentTime/1000)+((double)totaltime/1000)-((double)totalafktime/1000)-((double)afktime/1000));
	cJSON_AddNumberToObject(recobj, "Average FPS", totalfps/frames);
	cJSON_AddNumberToObject(recobj, "Number of frames", frames);
	cJSON_AddNumberToObject(recobj, "Total AFK Time", ((double)totalafktime/1000)+((double)afktime/1000)+((double)prevafktime/1000));
	cJSON_AddNumberToObject(recobj, "Times Played", timesplayed);

	//HUDs
	cJSON_AddItemToObject(root, "HUD", hudobj=cJSON_CreateObject());
	cJSON_AddItemToObject(hudobj, "normal", cJSON_CreateIntArray(normalHud, HUD_OPTIONS));
	cJSON_AddItemToObject(hudobj, "debug", cJSON_CreateIntArray(debugHud, HUD_OPTIONS));

	//General settings
	cJSON_AddStringToObject(root, "Proxy", http_proxy_string);
	cJSON_AddString(&root, "Scale", sdl_scale);
	if (kiosk_enable)
		cJSON_AddTrueToObject(root, "FullScreen");
	cJSON_AddString(&root, "FastQuit", fastquit);
	cJSON_AddString(&root, "WindowX", savedWindowX);
	cJSON_AddString(&root, "WindowY", savedWindowY);

	//additional settings from my mod
	cJSON_AddNumberToObject(root, "heatmode", heatmode);
	cJSON_AddNumberToObject(root, "autosave", autosave);
	cJSON_AddNumberToObject(root, "realistic", realistic);
	if (unlockedstuff & 0x01)
		cJSON_AddNumberToObject(root, "cracker_unlocked", 1);
	if (unlockedstuff & 0x08)
		cJSON_AddNumberToObject(root, "show_votes", 1);
	if (unlockedstuff & 0x10)
		cJSON_AddNumberToObject(root, "EXPL_unlocked", 1);
	if (old_menu)
		cJSON_AddNumberToObject(root, "old_menu", 1);
	if (finding & 0x8)
		cJSON_AddNumberToObject(root, "alt_find", 1);
	cJSON_AddNumberToObject(root, "dateformat", dateformat);
	cJSON_AddNumberToObject(root, "ShowIDs", show_ids);
	cJSON_AddNumberToObject(root, "decobox_hidden", decobox_hidden);
	cJSON_AddNumberToObject(root, "doUpdates2", doUpdates);
	
	outputdata = cJSON_Print(root);
	cJSON_Delete(root);
	
#ifdef MACOSX
	writeUserPreferences(outputdata);
#else
	fwrite(outputdata, 1, strlen(outputdata), f);
	fclose(f);
	free(outputdata);
#endif
}

void load_console_history(cJSON *tmpobj, command_history **last_command, int count)
{
	command_history *currentcommand = *last_command;
	for (int i = 0; i < count; i++)
	{
		currentcommand = (command_history*)malloc(sizeof(command_history));
		memset(currentcommand, 0, sizeof(command_history));
		currentcommand->prev_command = *last_command;
		ui_label_init(&currentcommand->command, 15, 0, 0, 0);
		strncpy(currentcommand->command.str, cJSON_GetArrayItem(tmpobj, i)->valuestring, 1023);
		*last_command = currentcommand;
	}
}

int cJSON_GetInt(cJSON **tmpobj)
{
	const char* ret = (*tmpobj)->valuestring;
	if (ret)
		return atoi(ret);
	else
		return 0;
}

void load_presets(void)
{
	int prefdatasize = 0;
#ifdef MACOSX
	char * prefdata = readUserPreferences();
#else
	char * prefdata = (char*)file_load("powder.pref", &prefdatasize);
#endif
	cJSON *root;
	if (prefdata && (root = cJSON_Parse(prefdata)))
	{
		cJSON *userobj, *versionobj, *recobj, *tmpobj, *graphicsobj, *hudobj, *simulationobj, *consoleobj;
		
		//Read user data
		userobj = cJSON_GetObjectItem(root, "User");
		if (userobj && (tmpobj = cJSON_GetObjectItem(userobj, "SessionKey")))
		{
			svf_login = 1;
			if ((tmpobj = cJSON_GetObjectItem(userobj, "Username")) && tmpobj->type == cJSON_String)
				strncpy(svf_user, tmpobj->valuestring, 63);
			else
				svf_user[0] = 0;
			if ((tmpobj = cJSON_GetObjectItem(userobj, "ID")) && tmpobj->type == cJSON_Number)
				sprintf(svf_user_id, "%i", tmpobj->valueint, 63);
			else
				svf_user_id[0] = 0;
			if ((tmpobj = cJSON_GetObjectItem(userobj, "SessionID")) && tmpobj->type == cJSON_String)
				strncpy(svf_session_id, tmpobj->valuestring, 63);
			else
				svf_session_id[0] = 0;
			if ((tmpobj = cJSON_GetObjectItem(userobj, "SessionKey")) && tmpobj->type == cJSON_String)
				strncpy(svf_session_key, tmpobj->valuestring, 63);
			else
				svf_session_key[0] = 0;
			if ((tmpobj = cJSON_GetObjectItem(userobj, "Elevation")) && tmpobj->type == cJSON_String)
			{
				if (!strcmp(tmpobj->valuestring, "Admin"))
				{
					svf_admin = 1;
					svf_mod = 0;
				}
				else if (!strcmp(tmpobj->valuestring, "Mod"))
				{
					svf_mod = 1;
					svf_admin = 0;
				}
				else
				{
					svf_admin = 0;
					svf_mod = 0;
				}
			}
		}
		else 
		{
			svf_login = 0;
			svf_user[0] = 0;
			svf_user_id[0] = 0;
			svf_session_id[0] = 0;
			svf_admin = 0;
			svf_mod = 0;
		}
		
		//Read version data
		versionobj = cJSON_GetObjectItem(root, "version");
		if (versionobj)
		{
			if (tmpobj = cJSON_GetObjectItem(versionobj, "major"))
				last_major = (unsigned char)tmpobj->valueint;
			if (tmpobj = cJSON_GetObjectItem(versionobj, "minor"))
				last_minor = (unsigned char)tmpobj->valueint;
			if (tmpobj = cJSON_GetObjectItem(versionobj, "build"))
				last_build = (unsigned char)tmpobj->valueint;
			if ((tmpobj = cJSON_GetObjectItem(versionobj, "update")) && tmpobj->type == cJSON_True)
				update_flag = 1;
			else
				update_flag = 0;
		}
		else
		{
			last_major = 0;
			last_minor = 0;
			last_build = 0;
			update_flag = 0;
		}
		
		//Read FavMenu/Records
		recobj = cJSON_GetObjectItem(root, "records");
		if (recobj)
		{
			if (tmpobj = cJSON_GetObjectItem(recobj, "Favorited Elements"))
				locked = tmpobj->valueint;
			if (tmpobj = cJSON_GetObjectItem(recobj, "Recent Elements"))
			{
				for (int i = 0; i < 18; i++)
				{
					std::string element = cJSON_GetArrayItem(tmpobj, i)->valuestring;
					if (!GetToolFromIdentifier(element))
					{
						if (i > 18-locked)
							locked--;
						continue;
					}
					for (int j = 0; j < 18; j++)
					{
						if (element == favMenu[j])
						{
							while (j < 17-i)
							{
								favMenu[j] = favMenu[j+1];
								j++;
							}
							break;
						}
					}
					favMenu[i] = element;
				}
			}
			if (tmpobj = cJSON_GetObjectItem(recobj, "Total Time Played"))
				totaltime = (int)((tmpobj->valuedouble)*1000);
			if (tmpobj = cJSON_GetObjectItem(recobj, "Average FPS"))
				totalfps = tmpobj->valuedouble;
			if (tmpobj = cJSON_GetObjectItem(recobj, "Number of frames"))
				frames = tmpobj->valueint; totalfps = totalfps * frames;
			if (tmpobj = cJSON_GetObjectItem(recobj, "Total AFK Time"))
				prevafktime = (int)((tmpobj->valuedouble)*1000);
			if (tmpobj = cJSON_GetObjectItem(recobj, "Times Played"))
				timesplayed = tmpobj->valueint;
		}

		//Read display settings
		graphicsobj = cJSON_GetObjectItem(root, "Renderer");
		if(graphicsobj)
		{
			if (tmpobj = cJSON_GetObjectItem(graphicsobj, "ColourMode"))
				colour_mode = cJSON_GetInt(&tmpobj);
			if (tmpobj = cJSON_GetObjectItem(graphicsobj, "DisplayModes"))
			{
				char temp[32];
				int count = cJSON_GetArraySize(tmpobj);
				free(display_modes);
				display_mode = 0;
				display_modes = (unsigned int*)calloc(count+1, sizeof(unsigned int));
				for (int i = 0; i < count; i++)
				{
					unsigned int mode;
					strncpy(temp, cJSON_GetArrayItem(tmpobj, i)->valuestring, 31);
					sscanf(temp, "%x", &mode);
					display_mode |= mode;
					display_modes[i] = mode;
				}
			}
			if (tmpobj = cJSON_GetObjectItem(graphicsobj, "RenderModes"))
			{
				char temp[32];
				int count = cJSON_GetArraySize(tmpobj);
				free(render_modes);
				render_mode = 0;
				render_modes = (unsigned int*)calloc(count+1, sizeof(unsigned int));
				for (int i = 0; i < count; i++)
				{
					unsigned int mode;
					strncpy(temp, cJSON_GetArrayItem(tmpobj, i)->valuestring, 31);
					sscanf(temp, "%x", &mode);
					render_mode |= mode;
					render_modes[i] = mode;
				}
			}
			if ((tmpobj = cJSON_GetObjectItem(graphicsobj, "Decorations")) && tmpobj->type == cJSON_True)
				decorations_enable = true;
			if ((tmpobj = cJSON_GetObjectItem(graphicsobj, "GravityField")) && tmpobj->type == cJSON_True)
				drawgrav_enable = tmpobj->valueint;
			if((tmpobj = cJSON_GetObjectItem(graphicsobj, "DebugMode")) && tmpobj->type == cJSON_True)
				DEBUG_MODE = tmpobj->valueint;
		}

		//Read simulation settings
		simulationobj = cJSON_GetObjectItem(root, "Simulation");
		if (simulationobj)
		{
			if(tmpobj = cJSON_GetObjectItem(simulationobj, "EdgeMode"))
			{
				edgeMode = (char)cJSON_GetInt(&tmpobj);
				if (edgeMode > 3)
					edgeMode = 0;
			}
			if((tmpobj = cJSON_GetObjectItem(simulationobj, "NewtonianGravity")) && tmpobj->valuestring && !strcmp(tmpobj->valuestring, "1"))
				start_grav_async();
			if(tmpobj = cJSON_GetObjectItem(simulationobj, "AmbientHeat"))
				aheat_enable = cJSON_GetInt(&tmpobj);
			if(tmpobj = cJSON_GetObjectItem(simulationobj, "PrettyPowder"))
				pretty_powder = cJSON_GetInt(&tmpobj);
		}

		//read console history
		consoleobj = cJSON_GetObjectItem(root, "Console");
		if (consoleobj)
		{
			if (tmpobj = cJSON_GetObjectItem(consoleobj, "History"))
			{
				int size = cJSON_GetArraySize(tmpobj);
				tmpobj = cJSON_GetObjectItem(consoleobj, "HistoryResults");
				//if results doesn't have the same number of items as history, don't load them. This might cause a crash, and wouldn't match anyway
				if (tmpobj && cJSON_GetArraySize(tmpobj) == size)
				{
					load_console_history(cJSON_GetObjectItem(consoleobj, "History"), &last_command, size);
					load_console_history(tmpobj, &last_command_result, cJSON_GetArraySize(tmpobj));
				}
			}
		}

		//Read HUDs
		hudobj = cJSON_GetObjectItem(root, "HUD");
		if (hudobj)
		{
			if (tmpobj = cJSON_GetObjectItem(hudobj, "normal"))
			{
				int count = std::min(HUD_OPTIONS,cJSON_GetArraySize(tmpobj));
				for (int i = 0; i < count; i++)
				{
					normalHud[i] = cJSON_GetArrayItem(tmpobj, i)->valueint;
				}
			}
			if (tmpobj = cJSON_GetObjectItem(hudobj, "debug"))
			{
				int count = std::min(HUD_OPTIONS,cJSON_GetArraySize(tmpobj));
				for (int i = 0; i < count; i++)
				{
					debugHud[i] = cJSON_GetArrayItem(tmpobj, i)->valueint;
				}
			}
		}
		
		//Read general settings
		if ((tmpobj = cJSON_GetObjectItem(root, "Proxy")) && tmpobj->type == cJSON_String)
			strncpy(http_proxy_string, tmpobj->valuestring, 255);
		else
			http_proxy_string[0] = 0;
		if (tmpobj = cJSON_GetObjectItem(root, "Scale"))
		{
			sdl_scale = cJSON_GetInt(&tmpobj);
			if (sdl_scale <= 0)
				sdl_scale = 1;
		}
		if (tmpobj = cJSON_GetObjectItem(root, "Fullscreen"))
		{
			kiosk_enable = tmpobj->valueint;
			if (kiosk_enable)
				set_scale(sdl_scale, kiosk_enable);
		}
		if (tmpobj = cJSON_GetObjectItem(root, "FastQuit"))
			fastquit = cJSON_GetInt(&tmpobj);
		if (tmpobj = cJSON_GetObjectItem(root, "WindowX"))
			savedWindowX = cJSON_GetInt(&tmpobj);
		if (tmpobj = cJSON_GetObjectItem(root, "WindowY"))
			savedWindowY = cJSON_GetInt(&tmpobj);

		//Read some extra mod settings
		if (tmpobj = cJSON_GetObjectItem(root, "heatmode"))
			heatmode = tmpobj->valueint;
		if (tmpobj = cJSON_GetObjectItem(root, "autosave"))
			autosave = tmpobj->valueint;
		if (tmpobj = cJSON_GetObjectItem(root, "autosave"))
			autosave = tmpobj->valueint;
		/*if(tmpobj = cJSON_GetObjectItem(root, "realistic"))
		{
			realistic = tmpobj->valueint;
			if (realistic)
				ptypes[PT_FIRE].hconduct = 1;
		}*/
		if (tmpobj = cJSON_GetObjectItem(root, "cracker_unlocked"))
		{
			unlockedstuff |= 0x01;
			menuSections[SC_CRACKER]->enabled = true;
		}
		if (tmpobj = cJSON_GetObjectItem(root, "show_votes"))
			unlockedstuff |= 0x08;
		if (tmpobj = cJSON_GetObjectItem(root, "EXPL_unlocked"))
		{
			unlockedstuff |= 0x10;
			ptypes[PT_EXPL].enabled = 1;
			globalSim->elements[PT_EXPL].MenuVisible = 1;
			globalSim->elements[PT_EXPL].Enabled = 1;
			FillMenus();
		}
#ifndef TOUCHUI
		if (tmpobj = cJSON_GetObjectItem(root, "old_menu"))
			old_menu = 1;
#endif
		if (tmpobj = cJSON_GetObjectItem(root, "alt_find"))
			finding |= 0x8;
		if (tmpobj = cJSON_GetObjectItem(root, "dateformat"))
			dateformat = tmpobj->valueint;
		if (tmpobj = cJSON_GetObjectItem(root, "ShowIDs"))
			show_ids = tmpobj->valueint;
		if (tmpobj = cJSON_GetObjectItem(root, "decobox_hidden"))
			decobox_hidden = tmpobj->valueint;
		if (tmpobj = cJSON_GetObjectItem(root, "doUpdates2"))
			doUpdates = tmpobj->valueint;

		cJSON_Delete(root);
		free(prefdata);
	}
	else
		firstRun = true;
}

int sregexp(const char *str, char *pattern)
{
	int result;
	regex_t patternc;
	if (regcomp(&patternc, pattern,  0)!=0)
		return 1;
	result = regexec(&patternc, str, 0, NULL, 0);
	regfree(&patternc);
	return result;
}

int load_string(FILE *f, char *str, int max)
{
	int li;
	unsigned char lb[2];
	fread(lb, 2, 1, f);
	li = lb[0] | (lb[1] << 8);
	if (li > max)
	{
		str[0] = 0;
		return 1;
	}
	fread(str, li, 1, f);
	str[li] = 0;
	return 0;
}

void strcaturl(char *dst, char *src)
{
	char *d;
	unsigned char *s;

	for (d=dst; *d; d++) ;

	for (s=(unsigned char *)src; *s; s++)
	{
		if ((*s>='0' && *s<='9') ||
		        (*s>='a' && *s<='z') ||
		        (*s>='A' && *s<='Z'))
			*(d++) = *s;
		else
		{
			*(d++) = '%';
			*(d++) = hex[*s>>4];
			*(d++) = hex[*s&15];
		}
	}
	*d = 0;
}

void strappend(char *dst, const char *src)
{
	char *d;
	unsigned char *s;

	for (d=dst; *d; d++) ;

	for (s=(unsigned char *)src; *s; s++)
	{
		*(d++) = *s;
	}
	*d = 0;
}

void *file_load(const char *fn, int *size)
{
	FILE *f = fopen(fn, "rb");
	void *s;

	if (!f)
		return NULL;
	fseek(f, 0, SEEK_END);
	*size = ftell(f);
	fseek(f, 0, SEEK_SET);
	s = malloc(*size);
	if (!s)
	{
		fclose(f);
		return NULL;
	}
	fread(s, *size, 1, f);
	fclose(f);
	return s;
}

int file_exists(const char *filename)
{
	int exists = 0;
#ifdef WIN
	struct _stat s;
	if(_stat(filename, &s) == 0)
#else
	struct stat s;
	if(stat(filename, &s) == 0)
#endif
	{
		if(s.st_mode & S_IFDIR)
		{
			exists = 1;
		}
		else if(s.st_mode & S_IFREG)
		{
			exists = 1;
		}
		else
		{
			exists = 1;
		}
	}
	else
	{
		exists = 0;
	}
	return exists;
}

matrix2d m2d_multiply_m2d(matrix2d m1, matrix2d m2)
{
	matrix2d result = {
		m1.a*m2.a+m1.b*m2.c, m1.a*m2.b+m1.b*m2.d,
		m1.c*m2.a+m1.d*m2.c, m1.c*m2.b+m1.d*m2.d
	};
	return result;
}
vector2d m2d_multiply_v2d(matrix2d m, vector2d v)
{
	vector2d result = {
		m.a*v.x+m.b*v.y,
		m.c*v.x+m.d*v.y
	};
	return result;
}
matrix2d m2d_multiply_float(matrix2d m, float s)
{
	matrix2d result = {
		m.a*s, m.b*s,
		m.c*s, m.d*s,
	};
	return result;
}

vector2d v2d_multiply_float(vector2d v, float s)
{
	vector2d result = {
		v.x*s,
		v.y*s
	};
	return result;
}

vector2d v2d_add(vector2d v1, vector2d v2)
{
	vector2d result = {
		v1.x+v2.x,
		v1.y+v2.y
	};
	return result;
}
vector2d v2d_sub(vector2d v1, vector2d v2)
{
	vector2d result = {
		v1.x-v2.x,
		v1.y-v2.y
	};
	return result;
}

matrix2d m2d_new(float me0, float me1, float me2, float me3)
{
	matrix2d result = {me0,me1,me2,me3};
	return result;
}
vector2d v2d_new(float x, float y)
{
	vector2d result = {x, y};
	return result;
}

char * clipboardtext = NULL;
void clipboard_push_text(char * text)
{
	if (clipboardtext)
	{
		free(clipboardtext);
		clipboardtext = NULL;
	}
	clipboardtext = mystrdup(text);
#ifdef MACOSX
	writeClipboard(text);
#elif WIN
	if (OpenClipboard(NULL))
	{
		HGLOBAL cbuffer;
		char * glbuffer;

		EmptyClipboard();

		cbuffer = GlobalAlloc(GMEM_DDESHARE, strlen(text)+1);
		glbuffer = (char*)GlobalLock(cbuffer);

		strcpy(glbuffer, text);

		GlobalUnlock(cbuffer);
		SetClipboardData(CF_TEXT, cbuffer);
		CloseClipboard();
	}
#elif defined(LIN) && defined(SDL_VIDEO_DRIVER_X11)
	if (clipboard_text!=NULL) {
		free(clipboard_text);
		clipboard_text = NULL;
	}
	clipboard_text = mystrdup(text);
	sdl_wminfo.info.x11.lock_func();
	XSetSelectionOwner(sdl_wminfo.info.x11.display, XA_CLIPBOARD, sdl_wminfo.info.x11.window, CurrentTime);
	XFlush(sdl_wminfo.info.x11.display);
	sdl_wminfo.info.x11.unlock_func();
#else
	printf("Not implemented: put text on clipboard \"%s\"\n", text);
#endif
}

char * clipboard_pull_text()
{
#ifdef MACOSX
	char * data = readClipboard();
	if (!data)
		return mystrdup("");
	return mystrdup(data);
#elif WIN
	if (OpenClipboard(NULL))
	{
		HANDLE cbuffer;
		char * glbuffer;

		cbuffer = GetClipboardData(CF_TEXT);
		glbuffer = (char*)GlobalLock(cbuffer);
		GlobalUnlock(cbuffer);
		CloseClipboard();
		if(glbuffer!=NULL){
			return mystrdup(glbuffer);
		} //else {
		//	return "";
		//}
	}
#elif defined(LIN) && defined(SDL_VIDEO_DRIVER_X11)
	char *text = NULL;
	Window selectionOwner;
	sdl_wminfo.info.x11.lock_func();
	selectionOwner = XGetSelectionOwner(sdl_wminfo.info.x11.display, XA_CLIPBOARD);
	if (selectionOwner != None)
	{
		unsigned char *data = NULL;
		Atom type;
		int format, result;
		unsigned long len, bytesLeft;
		XConvertSelection(sdl_wminfo.info.x11.display, XA_CLIPBOARD, XA_UTF8_STRING, XA_CLIPBOARD, sdl_wminfo.info.x11.window, CurrentTime);
		XFlush(sdl_wminfo.info.x11.display);
		sdl_wminfo.info.x11.unlock_func();
		while (1)
		{
			SDL_Event event;
			SDL_WaitEvent(&event);
			if (event.type == SDL_SYSWMEVENT)
			{
				XEvent xevent = event.syswm.msg->event.xevent;
				if (xevent.type == SelectionNotify && xevent.xselection.requestor == sdl_wminfo.info.x11.window)
					break;
				else
					EventProcess(event);
			}
		}
		sdl_wminfo.info.x11.lock_func();
		XGetWindowProperty(sdl_wminfo.info.x11.display, sdl_wminfo.info.x11.window, XA_CLIPBOARD, 0, 0, 0, AnyPropertyType, &type, &format, &len, &bytesLeft, &data);
		if (data)
		{
			XFree(data);
			data = NULL;
		}
		if (bytesLeft)
		{
			result = XGetWindowProperty(sdl_wminfo.info.x11.display, sdl_wminfo.info.x11.window, XA_CLIPBOARD, 0, bytesLeft, 0, AnyPropertyType, &type, &format, &len, &bytesLeft, &data);
			if (result == Success)
			{
				text = strdup((const char*) data);
				XFree(data);
			}
			else
			{
				printf("Failed to pull from clipboard\n");
				return mystrdup("?");
			}
		}
		else
			return mystrdup("");
		XDeleteProperty(sdl_wminfo.info.x11.display, sdl_wminfo.info.x11.window, XA_CLIPBOARD);
	}
	sdl_wminfo.info.x11.unlock_func();
	return text;
#else
	printf("Not implemented: get text from clipboard\n");
#endif
	if (clipboardtext)
		return mystrdup(clipboardtext);
	return "";
}

int register_extension()
{
#ifdef WIN
	int returnval;
	LONG rresult;
	HKEY newkey;
	char *currentfilename = exe_name();
	char *iconname = NULL;
	char *opencommand = NULL;
	char *protocolcommand = NULL;
	//char AppDataPath[MAX_PATH];
	char *AppDataPath = NULL;
	iconname = (char*)malloc(strlen(currentfilename)+6);
	sprintf(iconname, "%s,-102", currentfilename);
	
	//Create Roaming application data folder
	/*if(!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA|CSIDL_FLAG_CREATE, NULL, 0, AppDataPath))) 
	{
		returnval = 0;
		goto finalise;
	}*/
	
	AppDataPath = (char*)_getcwd(NULL, 0);

	//Move Game executable into application data folder
	//TODO: Implement
	
	opencommand = (char*)malloc(strlen(currentfilename)+53+strlen(AppDataPath));
	protocolcommand = (char*)malloc(strlen(currentfilename)+55+strlen(AppDataPath));
	/*if((strlen(AppDataPath)+strlen(APPDATA_SUBDIR "\\Powder Toy"))<MAX_PATH)
	{
		strappend(AppDataPath, APPDATA_SUBDIR);
		_mkdir(AppDataPath);
		strappend(AppDataPath, "\\Powder Toy");
		_mkdir(AppDataPath);
	} else {
		returnval = 0;
		goto finalise;
	}*/
	sprintf(opencommand, "\"%s\" open \"%%1\" ddir \"%s\"", currentfilename, AppDataPath);
	sprintf(protocolcommand, "\"%s\" ddir \"%s\" ptsave \"%%1\"", currentfilename, AppDataPath);

	//Create protocol entry
	rresult = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Classes\\ptsave", 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &newkey, NULL);
	if (rresult != ERROR_SUCCESS) {
		returnval = 0;
		goto finalise;
	}
	rresult = RegSetValueEx(newkey, 0, 0, REG_SZ, (LPBYTE)"Powder Toy Save", strlen("Powder Toy Save")+1);
	if (rresult != ERROR_SUCCESS) {
		RegCloseKey(newkey);
		returnval = 0;
		goto finalise;
	}
	rresult = RegSetValueEx(newkey, (LPCSTR)"URL Protocol", 0, REG_SZ, (LPBYTE)"", strlen("")+1);
	if (rresult != ERROR_SUCCESS) {
		RegCloseKey(newkey);
		returnval = 0;
		goto finalise;
	}
	RegCloseKey(newkey);
	
	
	//Set Protocol DefaultIcon
	rresult = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Classes\\ptsave\\DefaultIcon", 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &newkey, NULL);
	if (rresult != ERROR_SUCCESS) {
		returnval = 0;
		goto finalise;
	}
	rresult = RegSetValueEx(newkey, 0, 0, REG_SZ, (LPBYTE)iconname, strlen(iconname)+1);
	if (rresult != ERROR_SUCCESS) {
		RegCloseKey(newkey);
		returnval = 0;
		goto finalise;
	}
	RegCloseKey(newkey);	
	
	//Set Protocol Launch command
	rresult = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Classes\\ptsave\\shell\\open\\command", 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &newkey, NULL);
	if (rresult != ERROR_SUCCESS) {
		returnval = 0;
		goto finalise;
	}
	rresult = RegSetValueEx(newkey, 0, 0, REG_SZ, (LPBYTE)protocolcommand, strlen(protocolcommand)+1);
	if (rresult != ERROR_SUCCESS) {
		RegCloseKey(newkey);
		returnval = 0;
		goto finalise;
	}
	RegCloseKey(newkey);
	
	//Create extension entry
	rresult = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Classes\\.cps", 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &newkey, NULL);
	if (rresult != ERROR_SUCCESS) {
		returnval = 0;
		goto finalise;
	}
	rresult = RegSetValueEx(newkey, 0, 0, REG_SZ, (LPBYTE)"PowderToySave", strlen("PowderToySave")+1);
	if (rresult != ERROR_SUCCESS) {
		RegCloseKey(newkey);
		returnval = 0;
		goto finalise;
	}
	RegCloseKey(newkey);

	rresult = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Classes\\.stm", 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &newkey, NULL);
	if (rresult != ERROR_SUCCESS) {
		returnval = 0;
		goto finalise;
	}
	rresult = RegSetValueEx(newkey, 0, 0, REG_SZ, (LPBYTE)"PowderToySave", strlen("PowderToySave")+1);
	if (rresult != ERROR_SUCCESS) {
		RegCloseKey(newkey);
		returnval = 0;
		goto finalise;
	}
	RegCloseKey(newkey);
	
	//Create program entry
	rresult = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Classes\\PowderToySave", 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &newkey, NULL);
	if (rresult != ERROR_SUCCESS) {
		returnval = 0;
		goto finalise;
	}
	rresult = RegSetValueEx(newkey, 0, 0, REG_SZ, (LPBYTE)"Powder Toy Save", strlen("Powder Toy Save")+1);
	if (rresult != ERROR_SUCCESS) {
		RegCloseKey(newkey);
		returnval = 0;
		goto finalise;
	}
	RegCloseKey(newkey);

	//Set DefaultIcon
	rresult = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Classes\\PowderToySave\\DefaultIcon", 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &newkey, NULL);
	if (rresult != ERROR_SUCCESS) {
		returnval = 0;
		goto finalise;
	}
	rresult = RegSetValueEx(newkey, 0, 0, REG_SZ, (LPBYTE)iconname, strlen(iconname)+1);
	if (rresult != ERROR_SUCCESS) {
		RegCloseKey(newkey);
		returnval = 0;
		goto finalise;
	}
	RegCloseKey(newkey);

	//Set Launch command
	rresult = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Classes\\PowderToySave\\shell\\open\\command", 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &newkey, NULL);
	if (rresult != ERROR_SUCCESS) {
		returnval = 0;
		goto finalise;
	}
	rresult = RegSetValueEx(newkey, 0, 0, REG_SZ, (LPBYTE)opencommand, strlen(opencommand)+1);
	if (rresult != ERROR_SUCCESS) {
		RegCloseKey(newkey);
		returnval = 0;
		goto finalise;
	}
	RegCloseKey(newkey);
	
	returnval = 1;
	finalise:

	if(iconname) free(iconname);
	if(opencommand) free(opencommand);
	if(currentfilename) free(currentfilename);
	if(protocolcommand) free(protocolcommand);
	
	return returnval;
#elif LIN
	std::string filename = exe_name(), pathname = filename.substr(0, filename.rfind('/'));
	for (int i = 0; i < filename.size(); i++)
	{
		if (filename[i] == '\'')
		{
			filename.insert(i, "'\\'");
			i += 3;
		}
	}
	filename.insert(filename.size(), "'");
	filename.insert(0, "'");
	FILE *f;
	char *mimedata =
"<?xml version=\"1.0\"?>\n"
"	<mime-info xmlns='http://www.freedesktop.org/standards/shared-mime-info'>\n"
"	<mime-type type=\"application/vnd.powdertoy.save\">\n"
"		<comment>Powder Toy save</comment>\n"
"		<glob pattern=\"*.cps\"/>\n"
"		<glob pattern=\"*.stm\"/>\n"
"	</mime-type>\n"
"</mime-info>\n";
	f = fopen("powdertoy-save.xml", "wb");
	if (!f)
		return 0;
	fwrite(mimedata, 1, strlen(mimedata), f);
	fclose(f);

	char *protocolfiledata_tmp =
"[Desktop Entry]\n"
"Type=Application\n"
"Name=Powder Toy\n"
"Comment=Physics sandbox game\n"
"MimeType=x-scheme-handler/ptsave;\n"
"NoDisplay=true\n"
"Categories=Game\n";
	std::stringstream protocolfiledata;
	protocolfiledata << protocolfiledata_tmp << "Exec=" << filename <<" ptsave %u\nPath=" << pathname << "\n";
	f = fopen("powdertoy-tpt-ptsave.desktop", "wb");
	if (!f)
		return 0;
	fwrite(protocolfiledata.str().c_str(), 1, strlen(protocolfiledata.str().c_str()), f);
	fclose(f);
	system("xdg-desktop-menu install powdertoy-tpt-ptsave.desktop");

	char *desktopfiledata_tmp =
"[Desktop Entry]\n"
"Type=Application\n"
"Name=Powder Toy\n"
"Comment=Physics sandbox game\n"
"MimeType=application/vnd.powdertoy.save;\n"
"NoDisplay=true\n"
"Categories=Game\n";
	std::stringstream desktopfiledata;
	desktopfiledata << desktopfiledata_tmp << "Exec=" << filename <<" open %f\nPath=" << pathname << "\n";
	f = fopen("powdertoy-tpt.desktop", "wb");
	if (!f)
		return 0;
	fwrite(desktopfiledata.str().c_str(), 1, strlen(desktopfiledata.str().c_str()), f);
	fclose(f);
	system("xdg-mime install powdertoy-save.xml");
	system("xdg-desktop-menu install powdertoy-tpt.desktop");
	f = fopen("powdertoy-save-32.png", "wb");
	if (!f)
		return 0;
	fwrite(icon_doc_32_png, 1, icon_doc_32_png_size, f);
	fclose(f);
	f = fopen("powdertoy-save-16.png", "wb");
	if (!f)
		return 0;
	fwrite(icon_doc_16_png, 1, icon_doc_16_png_size, f);
	fclose(f);
	system("xdg-icon-resource install --noupdate --context mimetypes --size 32 powdertoy-save-32.png application-vnd.powdertoy.save");
	system("xdg-icon-resource install --noupdate --context mimetypes --size 16 powdertoy-save-16.png application-vnd.powdertoy.save");
	system("xdg-icon-resource forceupdate");
	system("xdg-mime default powdertoy-tpt.desktop application/vnd.powdertoy.save");
	system("xdg-mime default powdertoy-tpt-ptsave.desktop x-scheme-handler/ptsave");
	unlink("powdertoy-save-32.png");
	unlink("powdertoy-save-16.png");
	unlink("powdertoy-save.xml");
	unlink("powdertoy-tpt.desktop");
	unlink("powdertoy-tpt-ptsave.desktop");
	return 1;
#elif defined MACOSX
	return 0;
#endif
}

void HSV_to_RGB(int h,int s,int v,int *r,int *g,int *b)//convert 0-255(0-360 for H) HSV values to 0-255 RGB
{
	float hh, ss, vv, c, x;
	int m;
	hh = h/60.0f;//normalize values
	ss = s/255.0f;
	vv = v/255.0f;
	c = vv * ss;
	x = c * ( 1 - fabs(fmod(hh,2.0f) -1) );
	if(hh<1){
		*r = (int)(c*255.0);
		*g = (int)(x*255.0);
		*b = 0;
	}
	else if(hh<2){
		*r = (int)(x*255.0);
		*g = (int)(c*255.0);
		*b = 0;
	}
	else if(hh<3){
		*r = 0;
		*g = (int)(c*255.0);
		*b = (int)(x*255.0);
	}
	else if(hh<4){
		*r = 0;
		*g = (int)(x*255.0);
		*b = (int)(c*255.0);
	}
	else if(hh<5){
		*r = (int)(x*255.0);
		*g = 0;
		*b = (int)(c*255.0);
	}
	else if(hh<6){
		*r = (int)(c*255.0);
		*g = 0;
		*b = (int)(x*255.0);
	}
	m = (int)((vv-c)*255.0);
	*r += m;
	*g += m;
	*b += m;
}

void RGB_to_HSV(int r,int g,int b,int *h,int *s,int *v)//convert 0-255 RGB values to 0-255(0-360 for H) HSV
{
	float rr, gg, bb, a,x,c,d;
	rr = r/255.0f;//normalize values
	gg = g/255.0f;
	bb = b/255.0f;
	a = std::min(rr,gg);
	a = std::min(a,bb);
	x = std::max(rr,gg);
	x = std::max(x,bb);
	if (r == g && g == b)//greyscale
	{
		*h = 0;
		*s = 0;
		*v = (int)(a*255.0);
	}
	else
	{
		int min = (r < g) ? ((r < b) ? 0 : 2) : ((g < b) ? 1 : 2);
		c = (min==0) ? gg-bb : ((min==2) ? rr-gg : bb-rr);
		d = (float)((min==0) ? 3 : ((min==2) ? 1 : 5));
		*h = (int)(60.0*(d - c/(x - a)));
		*s = (int)(255.0*((x - a)/x));
		*v = (int)(255.0*x);
	}
}

bool ShowOnScreenKeyboard(const char *str)
{
#ifdef ANDROID
	// keyboard without text input is a lot nicer
	// but for some reason none of the keys work, and mouse input is never sent through
	// unless you try to press a key where for some reason it clicks the thing under that key
	//SDL_ANDROID_ToggleScreenKeyboardWithoutTextInput();
	//ignoreMouseClicks = true;

	// blocking fullscreen keyboard
	SDL_ANDROID_ToggleScreenKeyboardTextInput(str);
	return true;
#endif
	return false;
}

bool IsOnScreenKeyboardShown()
{
#ifdef ANDROID
	return SDL_IsScreenKeyboardShown(NULL);
#endif
	return false;
}

Tool* GetToolFromIdentifier(std::string identifier)
{
	for (int i = 0; i < SC_TOTAL; i++)
	{
		for (unsigned int j = 0; j < menuSections[i]->tools.size(); j++)
		{
			if  (identifier == menuSections[i]->tools[j]->GetIdentifier())
				return menuSections[i]->tools[j];
		}
	}
	return NULL;
}

std::string URLEncode(std::string source)
{
	char * src = (char *)source.c_str();
	char * dst = new char[(source.length()*3)+2];
	std::fill(dst, dst+(source.length()*3)+2, 0);

	char *d;
	unsigned char *s;

	for (d=dst; *d; d++) ;

	for (s=(unsigned char *)src; *s; s++)
	{
		if ((*s>='0' && *s<='9') ||
				(*s>='a' && *s<='z') ||
				(*s>='A' && *s<='Z'))
			*(d++) = *s;
		else
		{
			*(d++) = '%';
			*(d++) = hex[*s>>4];
			*(d++) = hex[*s&15];
		}
	}
	*d = 0;

	std::string finalString(dst);
	delete[] dst;
	return finalString;
}

void membwand(void * destv, void * srcv, size_t destsize, size_t srcsize)
{
	size_t i;
	unsigned char * dest = (unsigned char*)destv;
	unsigned char * src = (unsigned char*)srcv;
	if (srcsize==destsize)
	{
		for(i = 0; i < destsize; i++){
			dest[i] = dest[i] & src[i];
		}
	}
	else
	{
		for(i = 0; i < destsize; i++){
			dest[i] = dest[i] & src[i%srcsize];
		}
	}
}
vector2d v2d_zero = {0,0};
matrix2d m2d_identity = {1,0,0,1};
