// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: st_stuff.h,v 1.2 2000/02/27 00:42:11 hurdler Exp $
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
// $Log: st_stuff.h,v $
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Status bar code.
//      Does the face/direction indicator animatin.
//      Does palette indicators as well (red pain/berserk, bright pickup)
//
//-----------------------------------------------------------------------------

#ifndef __STSTUFF_H__
#define __STSTUFF_H__

#include "doomtype.h"
#include "d_event.h"
#include "d_player.h"

                     //software mode : position according to resolution, not scaled
                     //hardware mode : original coords, scaled to current resolution, correct aspect
#define ST_Y         (rendermode==render_soft ? vid.height - ST_HEIGHT : BASEVIDHEIGHT - ST_HEIGHT)


//
// STATUS BAR
//

// Called by main loop.
boolean ST_Responder (event_t* ev);

// Called by main loop.
void ST_Ticker (void);

// Called by main loop.
void ST_Drawer (boolean fullscreen, boolean refresh);

// Called when the console player is spawned on each level.
void ST_Start (void);

// Called by startup code.
void ST_Init (void);

// Called by G_Responder() when pressing F12 while viewing a demo.
void ST_changeDemoView (void);

// Add status bar related commands & vars
void ST_AddCommands (void);



// need this for SCR_Recalc() coz widgets coords change with resolutions
extern boolean   st_recalc;

// States for status bar code.
typedef enum
{
    AutomapState,
    FirstPersonState

} st_stateenum_t;


// States for the chat code.
typedef enum
{
    StartChatState,
    WaitDestState,
    GetChatState

} st_chatstateenum_t;


boolean ST_Responder(event_t* ev);

// return if player a is in the same team of the player b
boolean ST_SameTeam(player_t *a,player_t *b);

// get the frags of the player
// only one function for calculation : more simple code
int  ST_PlayerFrags (int playernum);

//--------------------
// status bar overlay
//--------------------
extern boolean    st_overlay;   // sb overlay on or off when fullscreen
extern int        st_palette;   // view palette

void ST_overlayDrawer (int playernum);   // draw vital info overlay when fullscreen


#endif
