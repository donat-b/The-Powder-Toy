/**
 * Powder Toy - Heatmap Data
 *
 * Copyright (c) 2010 Simon Robertshaw
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */
unsigned char color_data[] = {0xD9,0xFF,0xFE,0xD4,0xFB,0xFC,0xCF,0xF6,0xF9,0xC9,0xF1,0xF6,0xC2,0xEB,0xF4,0xBB,0xE5,0xF0,0xB3,0xDE,0xED,0xAA,0xD6,0xE9,0xA1,0xCF,0xE5,0x98,0xC7,0xE2,0x8E,0xBF,0xDD,0x85,0xB6,0xD9,0x7B,0xAD,0xD5,0x71,0xA5,0xD0,0x67,0x9C,0xCC,0x5D,0x93,0xC7,0x53,0x8A,0xC3,0x4A,0x82,0xBE,0x41,0x79,0xB9,0x38,0x71,0xB5,0x30,0x69,0xB0,0x28,0x61,0xAC,0x21,0x5A,0xA8,0x1B,0x53,0xA3,0x16,0x4D,0x9F,0x12,0x47,0x9B,0x0F,0x42,0x98,0x0F,0x3D,0x94,0x0F,0x39,0x90,0x0F,0x34,0x8C,0x0F,0x30,0x89,0x0F,0x2C,0x85,0x0F,0x28,0x81,0x0F,0x24,0x7D,0x0F,0x20,0x7A,0x0F,0x1D,0x76,0x0F,0x1A,0x72,0x0F,0x16,0x6F,0x0F,0x13,0x6B,0x0F,0x11,0x67,0x10,0x0E,0x64,0x12,0x0C,0x60,0x15,0x0A,0x5D,0x18,0x08,0x59,0x1B,0x06,0x56,0x1D,0x04,0x53,0x20,0x03,0x4F,0x23,0x02,0x4C,0x26,0x01,0x49,0x29,0x01,0x46,0x2C,0x00,0x43,0x2F,0x00,0x40,0x30,0x00,0x3F,0x31,0x00,0x3F,0x31,0x00,0x3E,0x32,0x00,0x3E,0x32,0x00,0x3D,0x33,0x00,0x3D,0x33,0x00,0x3C,0x34,0x00,0x3C,0x34,0x00,0x3B,0x35,0x01,0x3B,0x36,0x01,0x3B,0x36,0x01,0x3A,0x37,0x01,0x3A,0x37,0x01,0x39,0x38,0x01,0x39,0x39,0x01,0x38,0x39,0x02,0x38,0x3A,0x02,0x37,0x3B,0x02,0x37,0x3B,0x02,0x36,0x3C,0x02,0x36,0x3C,0x03,0x35,0x3D,0x03,0x35,0x3E,0x03,0x34,0x3F,0x03,0x34,0x3F,0x04,0x34,0x40,0x04,0x33,0x41,0x04,0x33,0x41,0x04,0x32,0x42,0x05,0x32,0x43,0x05,0x31,0x43,0x05,0x31,0x44,0x06,0x30,0x45,0x06,0x30,0x46,0x06,0x30,0x46,0x07,0x2F,0x47,0x07,0x2F,0x48,0x08,0x2E,0x49,0x08,0x2E,0x49,0x08,0x2E,0x4A,0x09,0x2D,0x4B,0x09,0x2D,0x4C,0x09,0x2C,0x4C,0x0A,0x2C,0x4D,0x0A,0x2B,0x4E,0x0B,0x2B,0x4F,0x0B,0x2B,0x4F,0x0C,0x2A,0x50,0x0C,0x2A,0x51,0x0D,0x29,0x52,0x0D,0x29,0x53,0x0D,0x29,0x53,0x0E,0x28,0x54,0x0E,0x28,0x55,0x0F,0x27,0x56,0x0F,0x27,0x57,0x10,0x27,0x58,0x10,0x26,0x58,0x11,0x26,0x59,0x11,0x26,0x5A,0x12,0x25,0x5B,0x13,0x25,0x5C,0x13,0x24,0x5D,0x14,0x24,0x5D,0x14,0x24,0x5E,0x15,0x23,0x5F,0x15,0x23,0x60,0x16,0x23,0x61,0x16,0x22,0x62,0x17,0x22,0x62,0x18,0x22,0x63,0x18,0x21,0x64,0x19,0x21,0x65,0x19,0x21,0x66,0x1A,0x20,0x67,0x1B,0x20,0x67,0x1B,0x20,0x68,0x1C,0x1F,0x69,0x1C,0x1F,0x6A,0x1D,0x1F,0x6B,0x1E,0x1E,0x6C,0x1E,0x1E,0x6D,0x1F,0x1E,0x6D,0x1F,0x1D,0x6E,0x20,0x1D,0x6F,0x21,0x1D,0x70,0x21,0x1C,0x71,0x22,0x1C,0x72,0x23,0x1C,0x73,0x23,0x1B,0x73,0x24,0x1B,0x74,0x25,0x1B,0x75,0x25,0x1A,0x76,0x26,0x1A,0x77,0x27,0x1A,0x78,0x27,0x1A,0x78,0x28,0x19,0x79,0x29,0x19,0x7A,0x29,0x19,0x7B,0x2A,0x18,0x7C,0x2B,0x18,0x7D,0x2B,0x18,0x7E,0x2C,0x18,0x7E,0x2D,0x17,0x7F,0x2D,0x17,0x80,0x2E,0x17,0x81,0x2F,0x16,0x82,0x2F,0x16,0x82,0x30,0x16,0x83,0x31,0x16,0x84,0x31,0x15,0x85,0x32,0x15,0x86,0x33,0x15,0x87,0x33,0x15,0x87,0x34,0x14,0x88,0x35,0x14,0x89,0x36,0x14,0x8A,0x36,0x14,0x8A,0x37,0x13,0x8B,0x38,0x13,0x8C,0x38,0x13,0x8D,0x39,0x13,0x8E,0x3A,0x13,0x8E,0x3A,0x12,0x8F,0x3B,0x12,0x90,0x3C,0x12,0x91,0x3C,0x12,0x91,0x3D,0x11,0x92,0x3E,0x11,0x93,0x3E,0x11,0x94,0x3F,0x11,0x94,0x40,0x11,0x95,0x40,0x10,0x96,0x41,0x10,0x97,0x42,0x10,0x97,0x42,0x10,0x98,0x43,0x10,0x99,0x44,0x0F,0x99,0x44,0x0F,0x9A,0x45,0x0F,0x9A,0x45,0x0F,0x9B,0x46,0x0F,0x9C,0x47,0x0F,0x9C,0x47,0x0E,0x9D,0x48,0x0E,0x9D,0x48,0x0E,0x9E,0x49,0x0E,0x9E,0x49,0x0E,0x9F,0x4A,0x0D,0xA0,0x4B,0x0D,0xA0,0x4B,0x0D,0xA1,0x4C,0x0D,0xA1,0x4C,0x0D,0xA2,0x4D,0x0C,0xA2,0x4E,0x0C,0xA3,0x4E,0x0C,0xA4,0x4F,0x0C,0xA4,0x4F,0x0C,0xA5,0x50,0x0C,0xA5,0x51,0x0C,0xA6,0x51,0x0C,0xA7,0x52,0x0C,0xA7,0x53,0x0C,0xA8,0x53,0x0C,0xA8,0x54,0x0C,0xA9,0x55,0x0C,0xAA,0x55,0x0C,0xAA,0x56,0x0C,0xAB,0x57,0x0C,0xAB,0x57,0x0C,0xAC,0x58,0x0C,0xAC,0x59,0x0C,0xAD,0x59,0x0C,0xAE,0x5A,0x0C,0xAE,0x5B,0x0C,0xAF,0x5B,0x0C,0xAF,0x5C,0x0C,0xB0,0x5D,0x0C,0xB1,0x5D,0x0C,0xB1,0x5E,0x0C,0xB2,0x5F,0x0C,0xB2,0x5F,0x0C,0xB3,0x60,0x0C,0xB4,0x61,0x0C,0xB4,0x61,0x0C,0xB5,0x62,0x0C,0xB5,0x63,0x0C,0xB6,0x64,0x0C,0xB6,0x64,0x0C,0xB7,0x65,0x0C,0xB8,0x66,0x0C,0xB8,0x66,0x0C,0xB9,0x67,0x0C,0xB9,0x68,0x0C,0xBA,0x69,0x0C,0xBB,0x69,0x0C,0xBB,0x6A,0x0C,0xBC,0x6B,0x0C,0xBC,0x6B,0x0C,0xBD,0x6C,0x0C,0xBD,0x6D,0x0C,0xBE,0x6E,0x0C,0xBF,0x6E,0x0C,0xBF,0x6F,0x0C,0xC0,0x70,0x0C,0xC0,0x71,0x0C,0xC1,0x71,0x0C,0xC2,0x72,0x0C,0xC2,0x73,0x0C,0xC3,0x74,0x0C,0xC3,0x74,0x0C,0xC4,0x75,0x0C,0xC4,0x76,0x0C,0xC5,0x76,0x0C,0xC5,0x77,0x0C,0xC6,0x78,0x0C,0xC7,0x79,0x0C,0xC7,0x79,0x0C,0xC8,0x7A,0x0C,0xC8,0x7B,0x0C,0xC9,0x7C,0x0C,0xC9,0x7C,0x0C,0xCA,0x7D,0x0C,0xCB,0x7E,0x0C,0xCB,0x7F,0x0C,0xCC,0x7F,0x0C,0xCC,0x80,0x0C,0xCD,0x81,0x0C,0xCD,0x82,0x0C,0xCE,0x82,0x0C,0xCE,0x83,0x0C,0xCF,0x84,0x0C,0xCF,0x85,0x0C,0xD0,0x85,0x0C,0xD0,0x86,0x0C,0xD1,0x87,0x0C,0xD2,0x88,0x0C,0xD2,0x88,0x0C,0xD3,0x89,0x0C,0xD3,0x8A,0x0C,0xD4,0x8B,0x0C,0xD4,0x8B,0x0C,0xD5,0x8C,0x0C,0xD5,0x8D,0x0C,0xD6,0x8D,0x0C,0xD6,0x8E,0x0C,0xD7,0x8F,0x0C,0xD7,0x90,0x0C,0xD8,0x90,0x0C,0xD8,0x91,0x0C,0xD9,0x92,0x0C,0xD9,0x93,0x0C,0xDA,0x93,0x0C,0xDA,0x94,0x0C,0xDB,0x95,0x0C,0xDB,0x95,0x0C,0xDC,0x96,0x0C,0xDC,0x97,0x0C,0xDD,0x98,0x0C,0xDD,0x98,0x0C,0xDE,0x99,0x0C,0xDE,0x9A,0x0C,0xDE,0x9A,0x0C,0xDF,0x9B,0x0C,0xDF,0x9C,0x0C,0xE0,0x9D,0x0C,0xE0,0x9D,0x0C,0xE1,0x9E,0x0C,0xE1,0x9F,0x0C,0xE2,0x9F,0x0C,0xE2,0xA0,0x0C,0xE3,0xA1,0x0C,0xE3,0xA1,0x0C,0xE3,0xA2,0x0C,0xE4,0xA3,0x0C,0xE4,0xA3,0x0C,0xE5,0xA4,0x0C,0xE5,0xA5,0x0C,0xE5,0xA5,0x0C,0xE6,0xA6,0x0C,0xE6,0xA7,0x0C,0xE7,0xA7,0x0C,0xE7,0xA8,0x0C,0xE8,0xA9,0x0C,0xE8,0xA9,0x0C,0xE8,0xAA,0x0C,0xE9,0xAB,0x0C,0xE9,0xAB,0x0C,0xE9,0xAC,0x0C,0xEA,0xAC,0x0C,0xEA,0xAD,0x0C,0xEB,0xAE,0x0C,0xEB,0xAE,0x0C,0xEB,0xAF,0x0C,0xEC,0xB0,0x0C,0xEC,0xB0,0x0C,0xEC,0xB1,0x0C,0xED,0xB1,0x0C,0xED,0xB2,0x0C,0xED,0xB3,0x0C,0xEE,0xB3,0x0C,0xEE,0xB4,0x0C,0xEE,0xB4,0x0C,0xEF,0xB5,0x0C,0xEF,0xB5,0x0C,0xEF,0xB6,0x0C,0xF0,0xB7,0x0C,0xF0,0xB7,0x0C,0xF0,0xB8,0x0D,0xF1,0xB8,0x0E,0xF1,0xB9,0x0F,0xF1,0xBA,0x10,0xF1,0xBA,0x11,0xF2,0xBB,0x12,0xF2,0xBB,0x13,0xF2,0xBC,0x14,0xF3,0xBD,0x15,0xF3,0xBD,0x16,0xF3,0xBE,0x17,0xF3,0xBE,0x18,0xF4,0xBF,0x19,0xF4,0xC0,0x1B,0xF4,0xC0,0x1C,0xF4,0xC1,0x1D,0xF5,0xC1,0x1E,0xF5,0xC2,0x1F,0xF5,0xC3,0x21,0xF5,0xC3,0x22,0xF6,0xC4,0x23,0xF6,0xC4,0x25,0xF6,0xC5,0x26,0xF6,0xC6,0x28,0xF7,0xC6,0x29,0xF7,0xC7,0x2A,0xF7,0xC7,0x2C,0xF7,0xC8,0x2D,0xF7,0xC8,0x2F,0xF8,0xC9,0x31,0xF8,0xCA,0x32,0xF8,0xCA,0x34,0xF8,0xCB,0x35,0xF8,0xCB,0x37,0xF8,0xCC,0x38,0xF9,0xCC,0x3A,0xF9,0xCD,0x3C,0xF9,0xCE,0x3D,0xF9,0xCE,0x3F,0xF9,0xCF,0x41,0xF9,0xCF,0x43,0xFA,0xD0,0x44,0xFA,0xD0,0x46,0xFA,0xD1,0x48,0xFA,0xD2,0x4A,0xFA,0xD2,0x4B,0xFA,0xD3,0x4D,0xFA,0xD3,0x4F,0xFA,0xD4,0x51,0xFB,0xD4,0x53,0xFB,0xD5,0x54,0xFB,0xD5,0x56,0xFB,0xD6,0x58,0xFB,0xD7,0x5A,0xFB,0xD7,0x5C,0xFB,0xD8,0x5E,0xFB,0xD8,0x60,0xFB,0xD9,0x62,0xFC,0xD9,0x64,0xFC,0xDA,0x65,0xFC,0xDA,0x67,0xFC,0xDB,0x69,0xFC,0xDB,0x6B,0xFC,0xDC,0x6D,0xFC,0xDC,0x6F,0xFC,0xDD,0x71,0xFC,0xDD,0x73,0xFC,0xDE,0x75,0xFC,0xDE,0x77,0xFC,0xDF,0x79,0xFC,0xDF,0x7B,0xFD,0xE0,0x7D,0xFD,0xE0,0x7F,0xFD,0xE1,0x81,0xFD,0xE1,0x83,0xFD,0xE2,0x85,0xFD,0xE2,0x87,0xFD,0xE3,0x89,0xFD,0xE3,0x8B,0xFD,0xE4,0x8D,0xFD,0xE4,0x8F,0xFD,0xE5,0x91,0xFD,0xE5,0x93,0xFD,0xE6,0x95,0xFD,0xE6,0x97,0xFD,0xE7,0x99,0xFD,0xE7,0x9B,0xFD,0xE8,0x9C,0xFD,0xE8,0x9E,0xFD,0xE9,0xA0,0xFD,0xE9,0xA2,0xFD,0xEA,0xA4,0xFD,0xEA,0xA6,0xFD,0xEA,0xA8,0xFE,0xEB,0xAA,0xFE,0xEB,0xAC,0xFE,0xEC,0xAE,0xFE,0xEC,0xB0,0xFE,0xED,0xB2,0xFE,0xED,0xB3,0xFE,0xED,0xB5,0xFE,0xEE,0xB7,0xFE,0xEE,0xB9,0xFE,0xEF,0xBB,0xFE,0xEF,0xBD,0xFE,0xF0,0xBE,0xFE,0xF0,0xC0,0xFE,0xF0,0xC2,0xFE,0xF1,0xC4,0xFE,0xF1,0xC6,0xFE,0xF2,0xC7,0xFE,0xF2,0xC9,0xFE,0xF2,0xCB,0xFE,0xF3,0xCD,0xFE,0xF3,0xCE,0xFE,0xF4,0xD0,0xFE,0xF4,0xD2,0xFE,0xF4,0xD3,0xFE,0xF5,0xD5,0xFE,0xF5,0xD6,0xFE,0xF5,0xD8,0xFE,0xF6,0xDA,0xFE,0xF6,0xDB,0xFE,0xF6,0xDD,0xFE,0xF7,0xDE,0xFE,0xF7,0xE0,0xFE,0xF7,0xE1,0xFE,0xF8,0xE3,0xFE,0xF8,0xE4,0xFE,0xF8,0xE5,0xFE,0xF9,0xE7,0xFE,0xF9,0xE8,0xFE,0xF9,0xE9,0xFE,0xFA,0xEB,0xFE,0xFA,0xEC,0xFE,0xFA,0xED,0xFE,0xFB,0xEF,0xFE,0xFB,0xF0,0xFE,0xFB,0xF1,0xFE,0xFC,0xF2,0xFF,0xFC,0xF3,0xFF,0xFC,0xF5,0xFF,0xFC,0xF6,0xFF,0xFD,0xF7,0xFF,0xFD,0xF8,0xFF,0xFD,0xF9,0xFF,0xFE,0xFA,0xFF,0xFE,0xFB,0xFF,0xFE,0xFC,0xFF,0xFE,0xFC,0xFF,0xFF,0xFD,0xFF,0xFF,0xFE};
unsigned char plasma_data[] = {0x00,0x00,0x00,0x03,0x00,0x00,0x05,0x00,0x00,0x09,0x00,0x00,0x0E,0x00,0x00,0x12,0x00,0x00,0x17,0x00,0x00,0x1C,0x00,0x00,0x22,0x00,0x00,0x27,0x00,0x00,0x2C,0x00,0x00,0x32,0x00,0x00,0x37,0x00,0x00,0x3C,0x00,0x00,0x41,0x00,0x00,0x45,0x00,0x00,0x4A,0x00,0x00,0x4D,0x00,0x00,0x51,0x00,0x00,0x53,0x00,0x00,0x55,0x00,0x00,0x55,0x00,0x02,0x55,0x02,0x03,0x55,0x03,0x06,0x55,0x03,0x07,0x55,0x05,0x09,0x55,0x06,0x0C,0x55,0x06,0x0F,0x55,0x07,0x10,0x55,0x09,0x13,0x55,0x0A,0x16,0x55,0x0C,0x1A,0x55,0x0C,0x1D,0x54,0x0E,0x20,0x53,0x10,0x23,0x54,0x11,0x26,0x53,0x13,0x2A,0x52,0x14,0x2E,0x51,0x15,0x31,0x50,0x17,0x35,0x50,0x19,0x38,0x4E,0x1A,0x3D,0x4D,0x1C,0x40,0x4D,0x1D,0x44,0x4C,0x1F,0x47,0x4B,0x21,0x4C,0x4A,0x23,0x4F,0x49,0x24,0x54,0x48,0x25,0x57,0x47,0x28,0x5B,0x46,0x29,0x5F,0x45,0x2B,0x62,0x44,0x2D,0x66,0x44,0x2E,0x6A,0x43,0x30,0x6E,0x42,0x32,0x72,0x41,0x33,0x76,0x40,0x35,0x79,0x3F,0x38,0x7C,0x3F,0x39,0x7F,0x3E,0x3B,0x83,0x3D,0x3D,0x86,0x3C,0x3F,0x8A,0x3B,0x40,0x8C,0x3B,0x43,0x8F,0x3B,0x44,0x92,0x3A,0x46,0x95,0x39,0x48,0x98,0x39,0x4A,0x9B,0x39,0x4C,0x9C,0x39,0x4E,0x9F,0x3A,0x4F,0xA1,0x39,0x51,0xA3,0x39,0x52,0xA4,0x39,0x54,0xA6,0x39,0x56,0xA7,0x39,0x57,0xA8,0x39,0x58,0xA8,0x3A,0x5A,0xA9,0x3A,0x5C,0xA8,0x3A,0x5D,0xA8,0x3B,0x5F,0xA8,0x3C,0x61,0xA8,0x3D,0x62,0xA8,0x3D,0x64,0xA9,0x3E,0x66,0xA8,0x3E,0x67,0xA9,0x3F,0x68,0xA8,0x40,0x6A,0xA8,0x41,0x6C,0xA8,0x42,0x6E,0xA8,0x42,0x70,0xA8,0x43,0x71,0xA8,0x44,0x73,0xA8,0x45,0x74,0xA9,0x46,0x76,0xA8,0x48,0x79,0xA8,0x49,0x7A,0xA7,0x4A,0x7C,0xA6,0x4A,0x7D,0xA5,0x4C,0x7F,0xA4,0x4D,0x81,0xA3,0x4E,0x83,0xA2,0x4E,0x85,0xA1,0x50,0x86,0xA0,0x51,0x88,0x9F,0x52,0x8A,0x9D,0x53,0x8B,0x9D,0x55,0x8D,0x9B,0x56,0x8F,0x9A,0x57,0x91,0x98,0x58,0x92,0x98,0x5A,0x94,0x97,0x5B,0x96,0x95,0x5C,0x97,0x94,0x5E,0x99,0x93,0x5F,0x9A,0x91,0x60,0x9C,0x90,0x61,0x9D,0x90,0x62,0x9F,0x8E,0x64,0xA1,0x8D,0x65,0xA2,0x8C,0x67,0xA3,0x8B,0x68,0xA5,0x8A,0x6A,0xA6,0x89,0x6A,0xA7,0x88,0x6B,0xA9,0x87,0x6D,0xAB,0x86,0x6E,0xAB,0x86,0x6F,0xAE,0x85,0x71,0xAE,0x85,0x72,0xAF,0x85,0x73,0xB0,0x84,0x75,0xB2,0x83,0x75,0xB2,0x83,0x77,0xB4,0x83,0x77,0xB5,0x83,0x79,0xB6,0x83,0x7A,0xB6,0x83,0x7B,0xB8,0x83,0x7D,0xB9,0x84,0x7E,0xB9,0x83,0x7F,0xBA,0x84,0x7F,0xBB,0x85,0x82,0xBC,0x85,0x83,0xBD,0x86,0x84,0xBD,0x87,0x85,0xBD,0x87,0x86,0xBE,0x88,0x88,0xBF,0x88,0x89,0xBF,0x89,0x8A,0xC0,0x8B,0x8B,0xC0,0x8C,0x8D,0xC1,0x8D,0x8F,0xC1,0x8E,0x90,0xC2,0x8F,0x91,0xC1,0x90,0x92,0xC2,0x91,0x94,0xC3,0x93,0x95,0xC3,0x93,0x97,0xC3,0x95,0x98,0xC3,0x96,0x99,0xC3,0x98,0x9B,0xC4,0x99,0x9C,0xC3,0x9A,0x9E,0xC4,0x9B,0x9F,0xC4,0x9D,0xA0,0xC4,0x9E,0xA2,0xC4,0x9F,0xA4,0xC4,0xA0,0xA5,0xC4,0xA2,0xA6,0xC4,0xA4,0xA8,0xC4,0xA5,0xA9,0xC5,0xA6,0xAA,0xC5,0xA8,0xAB,0xC4,0xA9,0xAC,0xC4,0xAA,0xAE,0xC4,0xAC,0xAF,0xC4,0xAD,0xB0,0xC4,0xAE,0xB2,0xC4,0xB0,0xB3,0xC4,0xB1,0xB4,0xC4,0xB3,0xB5,0xC5,0xB4,0xB6,0xC4,0xB6,0xB7,0xC4,0xB7,0xB8,0xC4,0xB7,0xBA,0xC4,0xB9,0xBA,0xC4,0xBA,0xBC,0xC4,0xBB,0xBC,0xC4,0xBC,0xBE,0xC4,0xBD,0xBF,0xC4,0xBE,0xBF,0xC4,0xBF,0xC0,0xC4,0xC0,0xC1,0xC4,0xC1,0xC2,0xC4,0xC2,0xC2,0xC4,0xC3,0xC4,0xC4,0xC3,0xC4,0xC4,0xC4};