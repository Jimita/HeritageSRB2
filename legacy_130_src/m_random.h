// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: m_random.h,v 1.2 2000/02/27 00:42:10 hurdler Exp $
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//
// $Log: m_random.h,v $
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//
//    
//-----------------------------------------------------------------------------


#ifndef __M_RANDOM__
#define __M_RANDOM__


#include "doomtype.h"



// Returns a number from 0 to 255,
// from a lookup table.
UINT8 M_Random (void);

#if 0
#define P_Random() P_Random2(__FILE__,__LINE__)
UINT8 P_Random2 (char *a,int b);
#else
// As M_Random, but used only by the play simulation.
UINT8 P_Random (void);
#endif

// Fix randoms for demos.
void M_ClearRandom (void);

UINT8 P_GetRandIndex(void);

void P_SetRandIndex(UINT8 rindex);

#endif
