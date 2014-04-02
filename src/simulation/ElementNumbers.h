/**
 * Powder Toy - element numbers
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

#if (!defined(ElementNumbers_H_Numbers) || (defined(ElementNumbers_Include_Decl) && !defined(ElementNumbers_H_Decl)) || (defined(ElementNumbers_Include_Call) && !defined(ElementNumbers_H_Call)))

//Defines for the number of elements that are rarely changed. Seems like it doesn't belong here ...
#define PT_NORMAL_NUM 177
#define PT_NUM 256

#undef ElementNumbers_Include_Numbers

#if (defined(ElementNumbers_Include_Decl) && !defined(ElementNumbers_H_Decl))
	#define ElementNumbers_H_Decl
#elif (defined(ElementNumbers_Include_Call) && !defined(ElementNumbers_H_Call))
	#define ElementNumbers_H_Call
#else
	#define ElementNumbers_H_Numbers
	#define ElementNumbers_Include_Numbers
	#define DEFINE_ELEMENT(name, id) PT_ ## name = id,
#endif

#ifdef ElementNumbers_Include_Numbers
enum element_ids
{
#endif


/* To make a new element, first add a new line at the end of this list.
 *
 * DEFINE_ELEMENT(THING, 111)
 *
 * The first argument is the name of the element (this doesn't have to
 * be the same as the name displayed in the game, it's just the name in
 * the code).
 * The second argument is the number used for the element. Normally,
 * this should be one higher than the number that was previously at the
 * end of the list.
 *
 * Then to define the element properties, make a new file in
 * src/simulation/elements. See the other element files for examples
 * of what to put in the new file.
 *
 * Once the DEFINE_ELEMENT() line has been added to this file, the
 * THING_init_element function will be called automatically when Powder
 * Toy starts.
 *
 * To refer to the element in the code, use PT_THING.
 */

DEFINE_ELEMENT(NONE, 0)
DEFINE_ELEMENT(DUST, 1)
DEFINE_ELEMENT(WATR, 2)
DEFINE_ELEMENT(OIL , 3)
DEFINE_ELEMENT(FIRE, 4)
DEFINE_ELEMENT(STNE, 5)
DEFINE_ELEMENT(LAVA, 6)
DEFINE_ELEMENT(GUNP, 7)
DEFINE_ELEMENT(NITR, 8)
DEFINE_ELEMENT(CLNE, 9)
DEFINE_ELEMENT(GAS , 10)
DEFINE_ELEMENT(PLEX, 11)
DEFINE_ELEMENT(GOO , 12)
DEFINE_ELEMENT(ICEI, 13)
DEFINE_ELEMENT(METL, 14)
DEFINE_ELEMENT(SPRK, 15)
DEFINE_ELEMENT(SNOW, 16)
DEFINE_ELEMENT(WOOD, 17)
DEFINE_ELEMENT(NEUT, 18)
DEFINE_ELEMENT(PLUT, 19)
DEFINE_ELEMENT(PLNT, 20)
DEFINE_ELEMENT(ACID, 21)
DEFINE_ELEMENT(VOID, 22)
DEFINE_ELEMENT(WTRV, 23)
DEFINE_ELEMENT(CNCT, 24)
DEFINE_ELEMENT(DSTW, 25)
DEFINE_ELEMENT(SALT, 26)
DEFINE_ELEMENT(SLTW, 27)
DEFINE_ELEMENT(DMND, 28)
DEFINE_ELEMENT(BMTL, 29)
DEFINE_ELEMENT(BRMT, 30)
DEFINE_ELEMENT(PHOT, 31)
DEFINE_ELEMENT(URAN, 32)
DEFINE_ELEMENT(WAX , 33)
DEFINE_ELEMENT(MWAX, 34)
DEFINE_ELEMENT(PSCN, 35)
DEFINE_ELEMENT(NSCN, 36)
DEFINE_ELEMENT(LNTG, 37)
DEFINE_ELEMENT(INSL, 38)
DEFINE_ELEMENT(BHOL, 39)
DEFINE_ELEMENT(WHOL, 40)
DEFINE_ELEMENT(RBDM, 41)
DEFINE_ELEMENT(LRBD, 42)
DEFINE_ELEMENT(NTCT, 43)
DEFINE_ELEMENT(SAND, 44)
DEFINE_ELEMENT(GLAS, 45)
DEFINE_ELEMENT(PTCT, 46)
DEFINE_ELEMENT(BGLA, 47)
DEFINE_ELEMENT(THDR, 48)
DEFINE_ELEMENT(PLSM, 49)
DEFINE_ELEMENT(ETRD, 50)
DEFINE_ELEMENT(NICE, 51)
DEFINE_ELEMENT(NBLE, 52)
DEFINE_ELEMENT(BTRY, 53)
DEFINE_ELEMENT(LCRY, 54)
DEFINE_ELEMENT(STKM, 55)
DEFINE_ELEMENT(SWCH, 56)
DEFINE_ELEMENT(SMKE, 57)
DEFINE_ELEMENT(DESL, 58)
DEFINE_ELEMENT(COAL, 59)
DEFINE_ELEMENT(LO2 , 60)
DEFINE_ELEMENT(O2  , 61)
DEFINE_ELEMENT(INWR, 62)
DEFINE_ELEMENT(YEST, 63)
DEFINE_ELEMENT(DYST, 64)
DEFINE_ELEMENT(THRM, 65)
DEFINE_ELEMENT(GLOW, 66)
DEFINE_ELEMENT(BRCK, 67)
DEFINE_ELEMENT(HFLM, 68)
DEFINE_ELEMENT(FIRW, 69)
DEFINE_ELEMENT(FUSE, 70)
DEFINE_ELEMENT(FSEP, 71)
DEFINE_ELEMENT(AMTR, 72)
DEFINE_ELEMENT(BCOL, 73)
DEFINE_ELEMENT(PCLN, 74)
DEFINE_ELEMENT(HSWC, 75)
DEFINE_ELEMENT(IRON, 76)
DEFINE_ELEMENT(MORT, 77)
DEFINE_ELEMENT(LIFE, 78)
DEFINE_ELEMENT(DLAY, 79)
DEFINE_ELEMENT(CO2 , 80)
DEFINE_ELEMENT(DRIC, 81)
DEFINE_ELEMENT(CBNW, 82)
DEFINE_ELEMENT(STOR, 83)
DEFINE_ELEMENT(PVOD, 84)
DEFINE_ELEMENT(CONV, 85)
DEFINE_ELEMENT(CAUS, 86)
DEFINE_ELEMENT(LIGH, 87)
DEFINE_ELEMENT(TESC, 88)
DEFINE_ELEMENT(DEST, 89)
DEFINE_ELEMENT(SPNG, 90)
DEFINE_ELEMENT(RIME, 91)
DEFINE_ELEMENT(FOG , 92)
DEFINE_ELEMENT(BCLN, 93)
DEFINE_ELEMENT(LOVE, 94)
DEFINE_ELEMENT(DEUT, 95)
DEFINE_ELEMENT(WARP, 96)
DEFINE_ELEMENT(PUMP, 97)
DEFINE_ELEMENT(FWRK, 98)
DEFINE_ELEMENT(PIPE, 99)
DEFINE_ELEMENT(FRZZ, 100)
DEFINE_ELEMENT(FRZW, 101)
DEFINE_ELEMENT(GRAV, 102)
DEFINE_ELEMENT(BIZR, 103)
DEFINE_ELEMENT(BIZRG, 104)
DEFINE_ELEMENT(BIZRS, 105)
DEFINE_ELEMENT(INST, 106)
DEFINE_ELEMENT(ISOZ, 107)
DEFINE_ELEMENT(ISZS, 108)
DEFINE_ELEMENT(PRTI, 109)
DEFINE_ELEMENT(PRTO, 110)
DEFINE_ELEMENT(PSTE, 111)
DEFINE_ELEMENT(PSTS, 112)
DEFINE_ELEMENT(ANAR, 113)
DEFINE_ELEMENT(VINE, 114)
DEFINE_ELEMENT(INVIS, 115)
DEFINE_ELEMENT(EQUALVEL, 116)
DEFINE_ELEMENT(SPAWN2, 117)
DEFINE_ELEMENT(SPAWN, 118)
DEFINE_ELEMENT(SHLD1, 119)
DEFINE_ELEMENT(SHLD2, 120)
DEFINE_ELEMENT(SHLD3, 121)
DEFINE_ELEMENT(SHLD4, 122)
DEFINE_ELEMENT(LOLZ, 123)
DEFINE_ELEMENT(WIFI, 124)
DEFINE_ELEMENT(FILT, 125)
DEFINE_ELEMENT(ARAY, 126)
DEFINE_ELEMENT(BRAY, 127)
DEFINE_ELEMENT(STKM2, 128)
DEFINE_ELEMENT(BOMB, 129)
DEFINE_ELEMENT(C5  ,130)
DEFINE_ELEMENT(SING, 131)
DEFINE_ELEMENT(QRTZ, 132)
DEFINE_ELEMENT(PQRT, 133)
DEFINE_ELEMENT(EMP ,134)
DEFINE_ELEMENT(BREL, 135)
DEFINE_ELEMENT(ELEC, 136)
DEFINE_ELEMENT(ACEL, 137)
DEFINE_ELEMENT(DCEL, 138)
DEFINE_ELEMENT(BANG, 139)
DEFINE_ELEMENT(IGNT, 140)
DEFINE_ELEMENT(BOYL, 141)
DEFINE_ELEMENT(GEL , 142)
DEFINE_ELEMENT(TRON, 143)
DEFINE_ELEMENT(TTAN, 144)
DEFINE_ELEMENT(EXOT, 145)

DEFINE_ELEMENT(EMBR, 147)
DEFINE_ELEMENT(H2  , 148)
DEFINE_ELEMENT(SOAP, 149)
DEFINE_ELEMENT(NBHL, 150)
DEFINE_ELEMENT(NWHL, 151)
DEFINE_ELEMENT(MERC, 152)
DEFINE_ELEMENT(PBCN, 153)
DEFINE_ELEMENT(GPMP, 154)
DEFINE_ELEMENT(CLST, 155)
DEFINE_ELEMENT(WIRE, 156)
DEFINE_ELEMENT(GBMB, 157)
DEFINE_ELEMENT(FIGH, 158)
DEFINE_ELEMENT(FRAY, 159)
DEFINE_ELEMENT(REPL, 160)
DEFINE_ELEMENT(PPIP, 161)
DEFINE_ELEMENT(DTEC, 162)
DEFINE_ELEMENT(DMG,  163)
DEFINE_ELEMENT(TSNS, 164)
DEFINE_ELEMENT(VIBR, 165)
DEFINE_ELEMENT(BVBR, 166)
DEFINE_ELEMENT(CRAY, 167)
DEFINE_ELEMENT(PSTN, 168)
DEFINE_ELEMENT(FRME, 169)
DEFINE_ELEMENT(GOLD, 170)
DEFINE_ELEMENT(TUNG, 171)
DEFINE_ELEMENT(PSNS, 172)
DEFINE_ELEMENT(PROT, 173)
DEFINE_ELEMENT(VIRS, 174)
DEFINE_ELEMENT(VRSS, 175)
DEFINE_ELEMENT(VRSG, 176)

DEFINE_ELEMENT(MOVS, PT_NORMAL_NUM)
DEFINE_ELEMENT(ANIM, PT_NORMAL_NUM+1)
DEFINE_ELEMENT(INDI, PT_NORMAL_NUM+2)
DEFINE_ELEMENT(OTWR, PT_NORMAL_NUM+3)
DEFINE_ELEMENT(PPTI, PT_NORMAL_NUM+4)
DEFINE_ELEMENT(PPTO, PT_NORMAL_NUM+5)
//VIRS, VRSS, VRSG, CURE
DEFINE_ELEMENT(BUTN, PT_NORMAL_NUM+6)
DEFINE_ELEMENT(PINV, PT_NORMAL_NUM+7)
DEFINE_ELEMENT(RAZR, PT_NORMAL_NUM+8)
DEFINE_ELEMENT(COND, PT_NORMAL_NUM+9)
DEFINE_ELEMENT(PWHT, PT_NORMAL_NUM+10)
DEFINE_ELEMENT(EXPL, PT_NORMAL_NUM+11)
DEFINE_ELEMENT(GRVT, PT_NORMAL_NUM+12)

// New elements go above this line


#ifdef ElementNumbers_Include_Numbers
};

#endif
#undef DEFINE_ELEMENT
#endif
