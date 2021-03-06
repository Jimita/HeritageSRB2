// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: d_netcmd.h,v 1.4 2000/04/07 23:11:17 metzgermeister Exp $
//
// Copyright (C) 1998-2000 by DooM Legacy Team.
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
// $Log: d_netcmd.h,v $
// Revision 1.4  2000/04/07 23:11:17  metzgermeister
// added mouse move
//
// Revision 1.3  2000/03/06 15:50:02  hurdler
// Add Bell Kin's changes
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      host/client network commands
//      commands are executed through the command buffer
//      like console commands
//
//-----------------------------------------------------------------------------


#ifndef __D_NETCMD__
#define __D_NETCMD__

#include "command.h"

// console vars
extern consvar_t   cv_playername;
extern consvar_t   cv_playercolor;
extern consvar_t   cv_usemouse;
extern consvar_t   cv_usejoystick;
#ifdef LJOYSTICK
extern consvar_t   cv_joyport;
#endif
extern consvar_t   cv_joyscale;
extern consvar_t   cv_autoaim;
extern consvar_t   cv_controlperkey;

// splitscreen with seconde mouse
extern consvar_t   cv_mouse2port;
extern consvar_t   cv_usemouse2;
#ifdef LMOUSE2
extern consvar_t   cv_mouse2opt;
#endif
extern consvar_t   cv_invertmouse2;
extern consvar_t   cv_alwaysfreelook2;
extern consvar_t   cv_mousemove2;
extern consvar_t   cv_mousesens2;
extern consvar_t   cv_mlooksens2;

// normaly in p_mobj but the .h in not read !
extern consvar_t   cv_itemrespawntime;
extern consvar_t   cv_itemrespawn;
extern consvar_t   cv_respawnmonsters;
extern consvar_t   cv_respawnmonsterstime;

extern consvar_t   cv_flagtime; // Tails 08-03-2001

// added 16-6-98 : splitscreen
extern consvar_t   cv_splitscreen;

// 02-08-98      : r_things.c
extern consvar_t   cv_skin;

// secondary splitscreen player
extern consvar_t   cv_playername2;
extern consvar_t   cv_playercolor2;
extern consvar_t   cv_skin2;

extern consvar_t   cv_teamplay;
extern consvar_t   cv_teamdamage;
extern consvar_t   cv_fraglimit;
extern consvar_t   cv_timelimit;
extern consvar_t   cv_allowexitlevel;

extern consvar_t   cv_analog; // Analog Test Tails 06-10-2001
extern consvar_t   cv_preferredteam; // Tails 07-22-2001

// Lactozilla: Heritage
extern consvar_t   cv_heritage_playermovement;
extern consvar_t   cv_heritage_directionchar;

extern consvar_t   cv_chasecam; // declare the cam var! Tails 01-06-2000
extern consvar_t   cv_homing; // Tails 07-02-2001
extern consvar_t   cv_nights; // Tails 07-02-2001
extern consvar_t   cv_numsnow; // Tails 12-25-2001

extern consvar_t   cv_netstat;
extern consvar_t   cv_translucency;
extern consvar_t   cv_splats;
extern consvar_t   cv_screenslink;

typedef enum {
    XD_NAMEANDCOLOR=1,
    XD_WEAPONPREF,
    XD_EXIT,
    XD_QUIT,
    XD_KICK,
    XD_NETVAR,
    XD_SAY,
    XD_MAP,
    XD_EXITLEVEL,
    XD_LOADGAME,
    XD_SAVEGAME,
    XD_PAUSE,
    XD_ADDPLAYER,
    MAXNETXCMD
} netxcmd_t;

// add game commands, needs cleanup
void D_RegisterClientCommands (void);
void D_SendPlayerConfig(void);

void SendWeaponPref(void);
void SendNameAndColor(void);
void SendNameAndColor2(void);

#endif
