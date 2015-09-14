/**
 * Powder Toy - Update Helper
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
#ifndef WIN
#include <sys/param.h>
#endif
#if !defined(MACOSX) && !defined(BSD)
#include <malloc.h>
#endif
#include <string.h>

#ifdef WIN
#include <windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif
#ifdef MACOSX
#include <mach-o/dyld.h>
#include <errno.h>
#endif

#include "update.h"
#include "interface.h"
#include "misc.h"
#include "common/Platform.h"

bool confirm_update(const char *changelog)
{
	if (confirm_ui(vid_buf, "\bwDo you want to update Jacob1's Mod?", changelog, "\btUpdate"))
	{
		char *tmp;
		int len;
		if (doUpdates == 2)
			tmp = download_ui(vid_buf, update_uri_alt, &len);
		else
			tmp = download_ui(vid_buf, update_uri, &len);

		if (tmp)
		{
			save_presets(1);
			if (update_start(tmp, len))
			{
				update_cleanup();
				save_presets(0);
				error_ui(vid_buf, 0, "Update failed - try downloading a new version.");
			}
			else
				return false;
		}
	}
	return true;
}

int update_start(char *data, int len)
{
	char *self=Platform::ExecutableName(), *temp;
#ifdef WIN
	char *p;
#endif
	FILE *f;
	int res = 1;

	if (!self)
		return 1;

#ifdef WIN
	temp = (char*)malloc(strlen(self)+12);
	strcpy(temp, self);
	p = temp + strlen(temp) - 4;
	if (_stricmp(p, ".exe"))
		p += 4;
	strcpy(p, "_update.exe");

	if (!MoveFile(self, temp))
		goto fail;

	f = fopen(self, "wb");
	if (!f)
		goto fail;
	if (fwrite(data, 1, len, f) != len)
	{
		fclose(f);
		DeleteFile(self);
		goto fail;
	}
	fclose(f);

	if ((int)ShellExecute(NULL, "open", self, NULL, NULL, SW_SHOWNORMAL) <= 32)
	{
		DeleteFile(self);
		goto fail;
	}

	return 0;
#else
	temp = (char*)malloc(strlen(self)+8);
	strcpy(temp, self);
	strcat(temp, "-update");

	f = fopen(temp, "w");
	if (!f)
		goto fail;
	if (fwrite(data, 1, len, f) != len)
	{
		fclose(f);
		unlink(temp);
		goto fail;
	}
	fclose(f);

	if (chmod(temp, 0755))
	{
		unlink(temp);
		goto fail;
	}

	if (rename(temp, self))
	{
		unlink(temp);
		goto fail;
	}

	execl(self, "powder-update", NULL);
#endif

fail:
	free(temp);
	free(self);
	return res;
}

int update_finish(void)
{
#ifdef WIN
	char *temp, *self=Platform::ExecutableName(), *p;
	int timeout = 60, err;

	temp = (char*)malloc(strlen(self)+12);
	strcpy(temp, self);
	p = temp + strlen(temp) - 4;
	if (_stricmp(p, ".exe"))
		p += 4;
	strcpy(p, "_update.exe");

	while (!DeleteFile(temp))
	{
		err = GetLastError();
		if (err == ERROR_FILE_NOT_FOUND)
		{
			// just as well, then
			free(temp);
			return 0;
		}
		Sleep(500);
		timeout--;
		if (timeout <= 0)
		{
			free(temp);
			return 1;
		}
	}
	free(temp);
#endif
	return 0;
}

void update_cleanup(void)
{
#ifdef WIN
	update_finish();
#endif
}
