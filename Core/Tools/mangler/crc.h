/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __CRC_H__
#define __CRC_H__

void Build_Packet_CRC(unsigned char *buf, int len); // len includes 4-byte CRC at head
bool Passes_CRC_Check(unsigned char *buf, int len); // len includes 4-byte CRC at head
void Add_CRC(unsigned long *crc, unsigned long val);

#endif // __CRC_H__
