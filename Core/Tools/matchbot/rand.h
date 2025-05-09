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

#ifndef __RAND_H__
#define __RAND_H__

#include <cstdlib>

class RandClass
{
public:
	RandClass(int start = 0);
	~RandClass()
	{}


	int Int(void);
	double Double(void);
	int Int(int low, int high);
	double Double(double low, double high);

private:

	unsigned int randomValue( void );
	unsigned int seed[6];

};

#endif /* __RAND_H__ */

