/**
 * Powder Toy - Update Helper (header)
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
#ifndef UPDATE_H
#define UPDATE_H

#ifdef WIN
#ifdef _64BIT
	#define UPDATE_ARCH "Windows64"
#else
	#define UPDATE_ARCH "Windows32"
#endif
#elif LIN
#ifdef _64BIT
	#define UPDATE_ARCH "Linux64"
#else
	#define UPDATE_ARCH "Linux32"
#endif
#elif MACOSX
	#define UPDATE_ARCH "MacOSX"
#else
	#define UPDATE_ARCH "Unknown"
#endif

#if defined X86_SSE3
	#define UPDATE_CPU "SSE3"
#elif defined X86_SSE2
	#define UPDATE_CPU "SSE2"
#elif defined X86_SSE
	#define UPDATE_CPU "SSE"
#else
	#define UPDATE_CPU "Unknown"
#endif

#ifndef UPDATESERVER
	#define UPDATESERVER "178.219.36.155" // change this to check for updates on a different server
	#define UPDATESERVERALT "mniip.com" // alternate update server (same as above)
#endif

const char update_uri[] = "http://" UPDATESERVER "/jacob1/update.lua?Action=Download&Architecture=" UPDATE_ARCH "&InstructionSet=" UPDATE_CPU;
const char changelog_uri[] = "http://" UPDATESERVER "/jacob1/update.lua?Action=CheckVersion&Architecture=" UPDATE_ARCH "&InstructionSet=" UPDATE_CPU;
const char update_uri_alt[] = "http://" UPDATESERVERALT "/jacob1/update.lua?Action=Download&Architecture=" UPDATE_ARCH "&InstructionSet=" UPDATE_CPU;
const char changelog_uri_alt[] = "http://" UPDATESERVERALT "/jacob1/update.lua?Action=CheckVersion&Architecture=" UPDATE_ARCH "&InstructionSet=" UPDATE_CPU;

char *exe_name(void);
bool confirm_update(const char *changelog);
int update_start(char *data, int len);
int update_finish(void);
void update_cleanup(void);

#endif
