// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: wi_stuff.c,v 1.3 2000/04/16 18:38:07 bpereira Exp $
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
// $Log: wi_stuff.c,v $
// Revision 1.3  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Intermission screens.
//
//-----------------------------------------------------------------------------


#include "doomdef.h"
#include "wi_stuff.h"
#include "d_main.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "m_random.h"
#include "r_local.h"
#include "s_sound.h"
#include "st_stuff.h"
#include "i_video.h"
#include "v_video.h"
#include "z_zone.h"
#include "console.h"

//
// Data needed to add patches to full screen intermission pics.
// Patches are statistics messages, and animations.
// Loads of by-pixel layout and placement, offsets etc.
//

// GLOBAL LOCATIONS
#define WI_TITLEY               2
#define WI_SPACINGY             16 //TODO: was 33

// SINGPLE-PLAYER STUFF
#define SP_STATSX               68
#define SP_STATSY               84

#define SP_TIMEX                17 // Tails 03-14-2000
#define SP_TIMEY                26 //(BASEVIDHEIGHT-32) // Tails 03-14-2000


// NET GAME STUFF
#define NG_STATSY               50
#define NG_STATSX               (32 + SHORT(star->width)/2 + 32*!dofrags)

#define NG_SPACINGX             64


// DEATHMATCH STUFF
#define DM_MATRIXX              16
#define DM_MATRIXY              24

#define DM_SPACINGX             32

#define DM_TOTALSX              269

#define DM_KILLERSX             0
#define DM_KILLERSY             100
#define DM_VICTIMSX             5
#define DM_VICTIMSY             50
// in sec
#define DM_WAIT                 20

//
// Animation.
// There is another anim_t used in p_spec.
//
typedef struct
{
	// following must be initialized to zero before use!

	// next value of bcnt (used in conjunction with period)
	int         nexttic;

	// last drawn animation frame
	int         lastdrawn;

	// next frame number to animate
	int         ctr;

	// used by RANDOM and LEVEL when animating
	int         state;
} anim_t;

//
// GENERAL DATA
//

//
// Locally used stuff.
//
#define FB 0

// used to accelerate or skip a stage
static int              acceleratestage;

// wbs->pnum
static int              me;

 // specifies current state
static stateenum_t      state;

// contains information passed into intermission
static wbstartstruct_t* wbs;

static wbplayerstruct_t* plrs;  // wbs->plyr[]

// used for general timing
static int              cnt;

// used for timing of background animation
static int              bcnt;

// signals to refresh everything for one frame
static int              firstrefresh;

static int              cnt_fscore[MAXPLAYERS]; // Tails 03-14-2000
static int              cnt_score[MAXPLAYERS]; // Score Tails 03-09-2000
static int              cnt_timebonus[MAXPLAYERS]; // Time Bonus Tails 03-10-2000
static int              cnt_ringbonus[MAXPLAYERS]; // Ring Bonus Tails 03-10-2000
static int              cnt_pause;

//
//      GRAPHICS
//

// background (map of levels).
static char             bgname[9];

// %, : graphics
static patch_t*         percent;
static patch_t*         colon;

// 0-9 graphic
static patch_t*         num[10];

// minus sign
static patch_t*         wiminus;

// "Finished!" graphics
static patch_t*         finished;

// "Entering" graphic
static patch_t*         entering;

// "secret"
static patch_t*         sp_secret;

 // "Kills", "Scrt", "Items", "Frags"
static patch_t*         cscore; // Tails 08-11-2001
static patch_t*         cemerald; // Tails 08-12-2001
static patch_t*         gotemall; // Tails 08-12-2001
static patch_t*         nowsuper; // Tails 08-12-2001
static patch_t*         chaos1; // Tails 08-12-2001
static patch_t*         chaos2; // Tails 08-12-2001
static patch_t*         chaos3; // Tails 08-12-2001
static patch_t*         chaos4; // Tails 08-12-2001
static patch_t*         chaos5; // Tails 08-12-2001
static patch_t*         chaos6; // Tails 08-12-2001
static patch_t*         chaos7; // Tails 08-12-2001
static patch_t*         kills;
static patch_t*         secret;
static patch_t*         items;
static patch_t*         frags;
static patch_t*         fscore; // Tails 03-14-2000
static patch_t*			haspassed; // Tails
static patch_t*			soncpass; // SONIC letters Tails 11-15-2000
static patch_t*			tailpass; // TAILS letters Tails 11-15-2000
static patch_t*			knuxpass; // KNUCKLES letters Tails 11-15-2000
static patch_t*			youpass; // YOU letters Tails 11-15-2000
static patch_t*			act1; // Tails 11-21-2000
static patch_t*			act2; // Tails 11-21-2000
static patch_t*			act3; // Tails 11-21-2000
//static patch_t*         result; // Tails 05-06-2001

// Time sucks.
static patch_t*         time;
static patch_t*         sucks;

// "killers", "victims"
static patch_t*         killers;
static patch_t*         victims;

// "Total", your face, your dead face
static patch_t*         total;
static patch_t*         star;
static patch_t*         bstar;

//added:08-02-98: use STPB0 for all players, but translate the colors
static patch_t*         stpb;

int blinker; // Tails 08-12-2001

//
// CODE
//

static void WI_unloadData(void);

// slam background

void WI_slamBackground(void)
{
	if (rendermode==render_soft) {
		memcpy(screens[0], screens[1], vid.width * vid.height);
		V_MarkRect (0, 0, vid.width, vid.height);
	} else {
		V_DrawScaledPatch(0, 0, 1+V_NOSCALESTART, W_CachePatchName(bgname, PU_CACHE));
	}
}

// The ticker is used to detect keys
//  because of timing issues in netgames.
boolean WI_Responder(event_t* ev)
{
	return false;
}

//
// Draws a number.
// If digits > 0, then use that many digits minimum,
//  otherwise only use as many as necessary.
// Returns new x position.
//

int
WI_drawNum
( int           x,
  int           y,
  int           n,
  int           digits )
{

	int         fontwidth = SHORT(num[0]->width);
	int         neg;
	int         temp;

	if (digits < 0)
	{
		if (!n)
		{
			// make variable-length zeros 1 digit long
			digits = 1;
		}
		else
		{
			// figure out # of digits in #
			digits = 0;
			temp = n;

			while (temp)
			{
				temp /= 10;
				digits++;
			}
		}
	}

	neg = n < 0;
	if (neg)
		n = -n;

	// if non-number, do not draw it
	if (n == 1994)
		return 0;

	// draw the new number
	while (digits--)
	{
		x -= fontwidth;
		V_DrawScaledPatch(x, y, FB, num[ n % 10 ]);
		n /= 10;
	}

	// draw a minus sign if necessary
	if (neg)
		V_DrawScaledPatch(x-=8, y, FB, wiminus);

	return x;

}

void
WI_drawPercent
( int           x,
  int           y,
  int           p )
{
	if (p < 0)
		return;

//    V_DrawScaledPatch(x, y, FB, percent); // Tails
	WI_drawNum(x, y, p, -1);
}

void WI_End(void)
{
	WI_unloadData();
}

// used for write introduce next level
void WI_initNoState(void)
{
	state = NoState;
	acceleratestage = 0;
	cnt = 10;
}

void WI_updateNoState(void)
{
	if (--cnt==0)
	{
		WI_End();
		G_NextLevel();
	}
}

void WI_drawNoState(void)
{
	if (cnt<=0)  // all removed no draw !!!
		return;

	WI_slamBackground();
}


static int              dm_frags[MAXPLAYERS][MAXPLAYERS];
static int              dm_totals[MAXPLAYERS];

void WI_initDeathmatchStats(void)
{

	int         i;
	int         j;

	state = StatCount;
	acceleratestage = 0;

	cnt_pause = TICRATE*DM_WAIT;

	for (i=0 ; i<MAXPLAYERS ; i++)
		 if (playeringame[i])
		 {
			 for(j=0; j<MAXPLAYERS; j++)
				 if( playeringame[j] )
					 dm_frags[i][j] = plrs[i].frags[j];

			 dm_totals[i] = ST_PlayerFrags(i);
	}
}

void WI_updateDeathmatchStats(void)
{
	if (D_AutoPause())
		return;

	if (cnt_pause>0) cnt_pause--;
	if (cnt_pause==0)
	{
		S_StartSound(0, sfx_pop);

		WI_initNoState();
	}
}


//  Quick-patch for the Cave party 19-04-1998 !!
//
void WI_drawRancking(char *title,int x,int y,fragsort_t *fragtable
						 , int scorelines, boolean large, int white)
{
	int   i,j;
	int   color;
	char  num[12];
	int   plnum;
	int   frags;
	fragsort_t temp;

	// sort the frags count
	for (i=0; i<scorelines; i++)
		for(j=0; j<scorelines-1-i; j++)
			if( fragtable[j].count < fragtable[j+1].count )
			{
				temp = fragtable[j];
				fragtable[j] = fragtable[j+1];
				fragtable[j+1] = temp;
			}

	if(title)
		V_DrawString (x, y-14, title);
	// draw rankings
	for (i=0; i<scorelines; i++)
	{
		frags = fragtable[i].count;
		plnum = fragtable[i].num;

		// draw color background
		color = fragtable[i].color;
		if (!color)
			color = *( (UINT8 *)colormaps + 0x78 );
		else
			color = *( (UINT8 *)translationtables - 256 + (color<<8) + 0x78 );
		V_DrawFill (x-1,y-1,large ? 40 : 26,9,color);

		// draw frags count
		sprintf(num,"%3i", frags );
		V_DrawString (x+(large ? 32 : 24)-V_StringWidth(num), y, num);

		// draw name
		if (plnum == white && cv_gametype.value != 4) // Tails 08-03-2001
			V_DrawStringWhite (x+(large ? 64 : 29), y, fragtable[i].name);
		else
			V_DrawString (x+(large ? 64 : 29), y, fragtable[i].name);

		y += 12;
		if (y>=BASEVIDHEIGHT)
			break;            // dont draw past bottom of screen
	}
}

#define RANKINGY 60

void WI_drawDeathmatchStats(void)
{
	int          i,j;
	int          scorelines;
	int          whiteplayer;
	fragsort_t   fragtab[MAXPLAYERS];
	char         *timeleft;

	WI_slamBackground();

	//Fab:25-04-98: when you play, you quickly see your frags because your
	//  name is displayed white, when playback demo, you quicly see who's the
	//  view.
	whiteplayer = demoplayback ? displayplayer : consoleplayer;

	// count frags for each present player
	scorelines = 0;
	for (i=0; i<MAXPLAYERS; i++)
		if (playeringame[i])
		{
			fragtab[scorelines].count = dm_totals[i];
			fragtab[scorelines].num   = i;
			fragtab[scorelines].color = players[i].skincolor;
			fragtab[scorelines].name  = player_names[i];
			scorelines++;
		}
	WI_drawRancking("Frags",5,RANKINGY,fragtab,scorelines,false,whiteplayer);

	// count buchholz
	scorelines = 0;
	for (i=0; i<MAXPLAYERS; i++)
		if (playeringame[i])
		{
			fragtab[scorelines].count = 0;
			for (j=0; j<MAXPLAYERS; j++)
				if (playeringame[j] && i!=j)
					 fragtab[scorelines].count+= dm_frags[i][j]*dm_totals[j];

			fragtab[scorelines].num = i;
			fragtab[scorelines].color = players[i].skincolor;
			fragtab[scorelines].name  = player_names[i];
			scorelines++;
		}
	WI_drawRancking("Buchholz",85,RANKINGY,fragtab,scorelines,false,whiteplayer);

	// count individuel
	scorelines = 0;
	for (i=0; i<MAXPLAYERS; i++)
		if (playeringame[i])
		{
			fragtab[scorelines].count = 0;
			for (j=0; j<MAXPLAYERS; j++)
				if (playeringame[j] && i!=j)
				{
					 if(dm_frags[i][j]>dm_frags[j][i])
						 fragtab[scorelines].count+=3;
					 else
						 if(dm_frags[i][j]==dm_frags[j][i])
							  fragtab[scorelines].count+=1;
				}

			fragtab[scorelines].num = i;
			fragtab[scorelines].color = players[i].skincolor;
			fragtab[scorelines].name  = player_names[i];
			scorelines++;
		}
	WI_drawRancking("indiv.",165,RANKINGY,fragtab,scorelines,false,whiteplayer);

	// count deads
	scorelines = 0;
	for (i=0; i<MAXPLAYERS; i++)
		if (playeringame[i])
		{
			fragtab[scorelines].count = 0;
			for (j=0; j<MAXPLAYERS; j++)
				if (playeringame[j])
					 fragtab[scorelines].count+=dm_frags[j][i];
			fragtab[scorelines].num   = i;
			fragtab[scorelines].color = players[i].skincolor;
			fragtab[scorelines].name  = player_names[i];

			scorelines++;
		}
	WI_drawRancking("deads",245,RANKINGY,fragtab,scorelines,false,whiteplayer);

	timeleft=va("start in %d",cnt_pause/TICRATE);
	V_DrawStringWhite (200, 30, timeleft);
}

boolean teamingame(int teamnum)
{
   int i;

   if (cv_teamplay.value == 1)
   {
	   for(i=0;i<MAXPLAYERS;i++)
		  if(playeringame[i] && players[i].skincolor==teamnum)
			  return true;
   }
   else
   if (cv_teamplay.value == 2)
   {
	   for(i=0;i<MAXPLAYERS;i++)
		  if(playeringame[i] && players[i].skin==teamnum)
			  return true;
   }
   return false;
}

void WI_drawTeamsStats(void)
{
	int          i,j;
	int          scorelines;
	int          whiteplayer;
	fragsort_t   fragtab[MAXPLAYERS];

	WI_slamBackground();

	//Fab:25-04-98: when you play, you quickly see your frags because your
	//  name is displayed white, when playback demo, you quicly see who's the
	//  view.
	if(cv_teamplay.value==1)
		whiteplayer = demoplayback ? players[displayplayer].skincolor
								   : players[consoleplayer].skincolor;
	else
		whiteplayer = demoplayback ? players[displayplayer].skin
								   : players[consoleplayer].skin;

	// count frags for each present player
	scorelines = HU_CreateTeamFragTbl(fragtab,dm_totals,dm_frags);

	WI_drawRancking("Frags",5,80,fragtab,scorelines,false,whiteplayer);

	// count buchholz
	scorelines = 0;
	for (i=0; i<MAXPLAYERS; i++)
		if (teamingame(i))
		{
			fragtab[scorelines].count = 0;
			for (j=0; j<MAXPLAYERS; j++)
				if (teamingame(j) && i!=j)
					fragtab[scorelines].count+= dm_frags[i][j]*dm_totals[j];

			fragtab[scorelines].num   = i;
			fragtab[scorelines].color = i;
			fragtab[scorelines].name  = team_names[i];
			scorelines++;
		}
	WI_drawRancking("Buchholz",85,80,fragtab,scorelines,false,whiteplayer);

	// count individuel
	scorelines = 0;
	for (i=0; i<MAXPLAYERS; i++)
		if (teamingame(i))
		{
			fragtab[scorelines].count = 0;
			for (j=0; j<MAXPLAYERS; j++)
				if (teamingame(j) && i!=j)
				{
					 if(dm_frags[i][j]>dm_frags[j][i])
						 fragtab[scorelines].count+=3;
					 else
						 if(dm_frags[i][j]==dm_frags[j][i])
							  fragtab[scorelines].count+=1;
				}

			fragtab[scorelines].num = i;
			fragtab[scorelines].color = i;
			fragtab[scorelines].name  = team_names[i];
			scorelines++;
		}
	WI_drawRancking("indiv.",165,80,fragtab,scorelines,false,whiteplayer);

	// count deads
	scorelines = 0;
	for (i=0; i<MAXPLAYERS; i++)
		if (teamingame(i))
		{
			fragtab[scorelines].count = 0;
			for (j=0; j<MAXPLAYERS; j++)
				if (teamingame(j))
					 fragtab[scorelines].count+=dm_frags[j][i];
			fragtab[scorelines].num   = i;
			fragtab[scorelines].color = i;
			fragtab[scorelines].name  = team_names[i];

			scorelines++;
		}
	WI_drawRancking("deads",245,80,fragtab,scorelines,false,whiteplayer);
}



void WI_ddrawDeathmatchStats(void)
{

	int         i;
	int         j;
	int         x;
	int         y;
	int         w;

	UINT8*       colormap;       //added:08-02-98:see below

	WI_slamBackground();

	// draw stat titles (top line)
	V_DrawScaledPatch(DM_TOTALSX-SHORT(total->width)/2,
				DM_MATRIXY-WI_SPACINGY+10,
				FB,
				total);

	V_DrawScaledPatch(DM_KILLERSX, DM_KILLERSY, FB, killers);
	V_DrawScaledPatch(DM_VICTIMSX, DM_VICTIMSY, FB, victims);

	// draw P?
	x = DM_MATRIXX + DM_SPACINGX;
	y = DM_MATRIXY;

	for (i=0 ; i<MAXPLAYERS ; i++)
	{
		if (playeringame[i])
		{
			//added:08-02-98: use V_DrawMappedPatch instead of
			//                    V_DrawScaledPatch, so that the
			// graphics are 'colormapped' to the player's colors!
			if (players[i].skincolor==0)
				colormap = colormaps;
			else
				colormap = (UINT8 *) translationtables - 256 + (players[i].skincolor<<8);

			V_DrawMappedPatch(x-SHORT(stpb->width)/2,
						DM_MATRIXY - WI_SPACINGY,
						FB,
						stpb,      //p[i], now uses a common STPB0 translated
						colormap); //      to the right colors

			V_DrawMappedPatch(DM_MATRIXX-SHORT(stpb->width)/2,
						y,
						FB,
						stpb,      //p[i]
						colormap);

			if (i == me)
			{
				V_DrawScaledPatch(x-SHORT(stpb->width)/2,
							DM_MATRIXY - WI_SPACINGY,
							FB,
							bstar);

				V_DrawScaledPatch(DM_MATRIXX-SHORT(stpb->width)/2,
							y,
							FB,
							star);
			}
		}

		x += DM_SPACINGX;
		y += WI_SPACINGY;
	}

	// draw stats
	y = DM_MATRIXY+10;
	w = SHORT(num[0]->width);

	for (i=0 ; i<MAXPLAYERS ; i++)
	{
		x = DM_MATRIXX + DM_SPACINGX;

		if (playeringame[i])
		{
			for (j=0 ; j<MAXPLAYERS ; j++)
			{
				if (playeringame[j])
					WI_drawNum(x+w, y, dm_frags[i][j], 2);

				x += DM_SPACINGX;
			}
			WI_drawNum(DM_TOTALSX+w, y, dm_totals[i], 2);
		}
		y += WI_SPACINGY;
	}
}

static int      cnt_frags[MAXPLAYERS];
static int      dofrags;
static int      ng_state;

void WI_initNetgameStats(void)
{
	int i;

	state = StatCount;
	acceleratestage = 0;
	ng_state = 1;

	cnt_pause = TICRATE;

	if(cv_gametype.value == 1 || cv_gametype.value == 3) // Match or Tag Tails 05-19-2001
		WI_initNoState();

	for (i=0 ; i<MAXPLAYERS ; i++)
	{
		if (!playeringame[i])
			continue;

		if (players[i].realtime / TICRATE < 30)
			players[i].timebonus = 50000;

		else if (players[i].realtime / TICRATE < 45 && players[i].realtime / TICRATE >= 30)
			players[i].timebonus = 10000;

		else if (players[i].realtime / TICRATE < 60 && players[i].realtime / TICRATE >= 45)
			players[i].timebonus = 5000;

		else if (players[i].realtime / TICRATE < 90 && players[i].realtime / TICRATE >= 60)
			players[i].timebonus = 4000;

		else if (players[i].realtime / TICRATE < 120 && players[i].realtime / TICRATE >= 90)
			players[i].timebonus = 3000;

		else if (players[i].realtime / TICRATE < 180 && players[i].realtime / TICRATE >= 120)
			players[i].timebonus = 2000;

		else if (players[i].realtime / TICRATE < 240 && players[i].realtime / TICRATE >= 180)
			players[i].timebonus = 1000;

		else if (players[i].realtime / TICRATE < 300 && players[i].realtime / TICRATE >= 240)
			players[i].timebonus = 500;

		else
			players[i].timebonus = 0;

		// End Time Bonus & Ring Bonus count settings Tails 03-10-2000

		cnt_fscore[i] = (plrs[i].sscore);  // Tails 03-14-2000
		cnt_score[i] = (plrs[i].sscore); // Score Tails 03-09-2000
		cnt_timebonus[i] = (players[i].timebonus); // Time Bonus Tails 03-10-2000
		cnt_ringbonus[i] = (100*(players[i].health - 1)); // Ring Bonus Tails 03-10-2000

		dofrags += ST_PlayerFrags(i);
	}

	dofrags = !!dofrags;
}



void WI_updateNetgameStats(void)
{
	int         i;
	int         fsum;
	int			z;
	int			numplayers;
	int			numdone;

	boolean     stillticking;

	if(cv_gametype.value == 2 || cv_gametype.value == 4) // Tails 05-19-2001
		ng_state = 10; // Tails 05-19-2001

	if (acceleratestage && ng_state != 10)
	{
		acceleratestage = 0;

		for (i=0 ; i<MAXPLAYERS ; i++)
		{
			if (!playeringame[i])
				continue;

			cnt_fscore[i] = (plrs[i].sscore + players[i].timebonus + (100*(players[i].health - 1))); // Tails 03-14-2000
			cnt_score[i] = (plrs[i].sscore + players[i].timebonus + (100*(players[i].health - 1))); // Score Tails 03-09-2000
			cnt_timebonus[i] = 0; // Time Bonus Tails 03-10-2000
			cnt_ringbonus[i] = 0; // Ring Bonus Tails 03-10-2000

// Gives lives for score Tails 04-11-2001
		if(cnt_fscore[i] >= 50000+players[i].xtralife2)
		{
			players[i].lives++;
			if(&players[i]==&players[consoleplayer])
			{
				S_StopMusic();
				S_ChangeMusic(mus_xtlife, false);
			}
			players[i].xtralife2 += 50000;
		}
// End score Tails 04-11-2001

			if (dofrags)
				cnt_frags[i] = ST_PlayerFrags(i);
		}
		S_StartSound(0, sfx_chchng); // Tails
		ng_state = 10;
	}

	if (ng_state == 2)
	{
		if(!(gamemap == SSSTAGE1 || gamemap == SSSTAGE2 || gamemap == SSSTAGE3
			|| gamemap == SSSTAGE4 || gamemap == SSSTAGE5 || gamemap == SSSTAGE6
			|| gamemap == SSSTAGE7))
		{
			stillticking = true;

			for (i=0 ; i<MAXPLAYERS ; i++)
			{
				if (!playeringame[i])
					continue;

				//			if(!stillticking)
				//				continue; // Don't do useless operations Tails 07-20-2001

				if((cnt_timebonus[i] > 0) && (cnt_ringbonus[i] > 0))
				{
					cnt_fscore[i] += 444; // Tails 03-14-2000
					cnt_score[i] += 444; // Score Tails 03-09-2000
				}
				else
				{
					cnt_fscore[i] += 222; // Tails 03-14-2000
					cnt_score[i] += 222; // Score Tails 03-09-2000
				}

				cnt_timebonus[i] -= 222; // Time Bonus Tails 03-10-2000
				cnt_ringbonus[i] -= 222; // Ring Bonus Tails 03-10-2000

				if (cnt_fscore[i] >= (plrs[i].sscore + players[i].timebonus + (100*(players[i].health - 1)))) // Score
					cnt_fscore[i] = (plrs[i].sscore + players[i].timebonus + (100*(players[i].health - 1)));  // Tails 03-09-2000

				if (cnt_score[i] >= (plrs[i].sscore + players[i].timebonus + (100*(players[i].health - 1)))) // Score
					cnt_score[i] = (plrs[i].sscore + players[i].timebonus + (100*(players[i].health - 1)));  // Tails 03-09-2000

				// Gives lives for score Tails 04-11-2001
				if(cnt_fscore[i] >= 50000+players[i].xtralife2)
				{
					players[i].lives++;
					if(&players[i]==&players[consoleplayer])
					{
						S_StopMusic();
						S_ChangeMusic(mus_xtlife, false);
					}
					players[i].xtralife2 += 50000;
				}
				// End score Tails 04-11-2001

				// start timebonus, ringbonus & stuff Tails 03-10-2000
				if (cnt_timebonus[i] <= 0)
				{
					cnt_timebonus[i] = 0;
				}

				if (cnt_ringbonus[i] <= 0)
				{
					cnt_ringbonus[i] = 0;
				}

				// end timebonus, ringbonus & stuff Tails 03-10-2000
				if(cnt_score[i] == (plrs[i].sscore + players[i].timebonus + (100*(players[i].health - 1))))  // Tails 03-09-2000
				{
					stillticking = false;
					players[i].score = cnt_fscore[i];
				}
				else
				{
					stillticking = true;
					if (!(bcnt&1) && !(cnt_score[me] == (plrs[me].sscore + players[me].timebonus + (100*(players[me].health - 1))))) // count sound faster! Tails 03-10-2000
						S_StartSound(0, sfx_menu1);
				}
			}

			if (!stillticking) // Wait for everybody! Tails 07-20-2001
			{
				numplayers = 0;
				numdone = 0;
				for(z=0; z<MAXPLAYERS; z++)
				{
					if(playeringame[z])
					{
						numplayers++;
						if((cnt_ringbonus[z] == 0) && (cnt_timebonus[z] == 0) && (cnt_score[z] == plrs[z].sscore + players[z].timebonus + (100*(players[z].health - 1))))
							numdone++;
					}
				}
				if(numdone == numplayers)
				{
					S_StartSound(0, sfx_chchng);
					ng_state = 10;
				}
			}
		}
		else // Special Stage Tails 08-12-2001
		{
			stillticking = true;

			for (i=0 ; i<MAXPLAYERS ; i++)
			{
				if (!playeringame[i])
					continue;

				cnt_fscore[i] += 222; // Tails 03-14-2000
				cnt_ringbonus[i] -= 222; // Ring Bonus Tails 03-10-2000

				if (cnt_fscore[i] >= (plrs[i].sscore + (100*(players[i].health - 1)))) // Score
					cnt_fscore[i] = (plrs[i].sscore + (100*(players[i].health - 1)));  // Tails 03-09-2000

				// Gives lives for score Tails 04-11-2001
				if(cnt_fscore[i] >= 50000+players[i].xtralife2)
				{
					players[i].lives++;
					if(&players[i]==&players[consoleplayer])
					{
						S_StopMusic();
						S_ChangeMusic(mus_xtlife, false);
					}
					players[i].xtralife2 += 50000;
				}
				// End score Tails 04-11-2001

				if (cnt_ringbonus[i] <= 0)
					cnt_ringbonus[i] = 0;

				if(cnt_fscore[i] == (plrs[i].sscore + (100*(players[i].health - 1))))  // Tails 03-09-2000
				{
					stillticking = false;
					players[i].score = cnt_fscore[i];
				}
				else
				{
					stillticking = true;
					if (!(bcnt&1) && !(cnt_fscore[me] == (plrs[me].sscore + (100*(players[me].health - 1))))) // count sound faster! Tails 03-10-2000
						S_StartSound(0, sfx_menu1);
				}
			}

			if (!stillticking) // Wait for everybody! Tails 07-20-2001
			{
				numplayers = 0;
				numdone = 0;
				for(z=0; z<MAXPLAYERS; z++)
				{
					if(playeringame[z])
					{
						numplayers++;
						if((cnt_ringbonus[z] == 0) && (cnt_fscore[z] == plrs[z].sscore + (100*(players[z].health - 1))))
							numdone++;
					}
				}
				if(numdone == numplayers)
				{
					S_StartSound(0, sfx_chchng);
					ng_state = 10;
				}

			}
		}
	}
	else if (ng_state == 4)
	{
		if (!(bcnt&3))
			S_StartSound(0, sfx_menu1);

		stillticking = false;

		for (i=0 ; i<MAXPLAYERS ; i++)
		{
			if (!playeringame[i])
				continue;
			else
				stillticking = true;
		}
		if (!stillticking)
		{
			ng_state++;
		}
	}
	else if (ng_state == 6)
	{
		if (!(bcnt&3))
			S_StartSound(0, sfx_menu1);

		stillticking = false;

		for (i=0 ; i<MAXPLAYERS ; i++)
		{
			if (!playeringame[i])
				continue;
		}

		if (!stillticking)
		{
			ng_state += 1 + 2*!dofrags;
		}
	}
	else if (ng_state == 8)
	{
		if (!(bcnt&1)) // count sound faster! Tails 03-10-2000
			S_StartSound(0, sfx_menu1);

		stillticking = false;

		for (i=0 ; i<MAXPLAYERS ; i++)
		{
			if (!playeringame[i])
				continue;

			cnt_frags[i] += 1;

			if (cnt_frags[i] >= (fsum = ST_PlayerFrags(i)))
				cnt_frags[i] = fsum;
			else
				stillticking = true;
		}

		if (!stillticking)
		{
			S_StartSound(0, sfx_pldeth);
			ng_state++;
		}
	}
	else if (ng_state == 10)
	{
		if (acceleratestage)
		{
			if(cv_gametype.value == 4)
			{
				int rings;
				rings = 0;
				for(i=0; i<MAXPLAYERS; i++)
				{
					if(playeringame[i] && players[i].ctfteam == 1)
						rings += players[i].health - 1;
				}
				if(rings >= 100)
				{
					players[0].redscore++;
				}
				rings = 0;
				for(i=0; i<MAXPLAYERS; i++)
				{
					if(playeringame[i] && players[i].ctfteam == 2)
						rings += players[i].health - 1;
				}
				if(rings >= 100)
				{
					players[0].bluescore++;
				}
			}
			if(!cv_gametype.value)
			{
				for(i=0; i<MAXPLAYERS; i++) // Chuck the tallied score into the player's score Tails 08-02-2001
					players[i].score = cnt_fscore[i];
			}
			WI_initNoState();
		}
	}
	else if (ng_state & 1)
	{
		if (!--cnt_pause)
		{
			ng_state++;
			cnt_pause = TICRATE;
		}
	}
}



void WI_drawNetgameStats(void)
{
	int         i;
	int         x;
	int         y; // used as a dummy now Tails 05-01-2001

	signed int topscore[4]; /* maximum value found -- OUTPUT VALUE */
	signed int etopscore[4]; /* element it was found in -- OUTPUT VALUE */
	signed int toptime[4]; /* maximum value found -- OUTPUT VALUE */
	signed int etoptime[4]; /* element it was found in -- OUTPUT VALUE */
	signed int topitembox[4]; /* maximum value found -- OUTPUT VALUE */
	signed int etopitembox[4]; /* element it was found in -- OUTPUT VALUE */
	signed int topring[4]; /* maximum value found -- OUTPUT VALUE */
	signed int etopring[4]; /* element it was found in -- OUTPUT VALUE */
	signed int toptotalring[4]; /* maximum value found -- OUTPUT VALUE */
	signed int etoptotalring[4]; /* element it was found in -- OUTPUT VALUE */

	signed int racerank[4]; // Tails 05-01-2001
	signed int eracerank[4]; // Tails 05-01-2001

	memset(racerank, 0x00, sizeof(racerank));
	memset(eracerank, 0x00, sizeof(eracerank));

	char splaynum[33];
	char sscore[33];
	char sseconds[33];
	char sminutes[33];
	char stics[33];
	char sring[33];
	char stotring[33];
	char sitembox[33];
	char setopscore[33];
	char setoptime[33];
	char setopitembox[33];
	char setopring[33];
	char setoptotalring[33];
	char sracescore[33];

	int lh;
	static patch_t* act; // Tails 11-21-2000

	lh = (3*SHORT(num[0]->height))/2;

	WI_slamBackground();

		if((gamemap == SSSTAGE1 || gamemap == SSSTAGE2 || gamemap == SSSTAGE3
		|| gamemap == SSSTAGE4 || gamemap == SSSTAGE5 || gamemap == SSSTAGE6
		|| gamemap == SSSTAGE7))
	{
		// Draw big CHAOS EMERALDS thing
		if(players[me].emerald1 && players[me].emerald2 && players[me].emerald3 && players[me].emerald4 && players[me].emerald5 && players[me].emerald6 && players[me].emerald7)
		{
			if(ng_state == 10)
				V_DrawScaledPatch(48, 32, FB, nowsuper);
			else
				V_DrawScaledPatch(70, 26, FB, gotemall);
		}
		else
			V_DrawScaledPatch(48, 26, FB, cemerald);

		if(blinker & 1)
		{
			// Display Chaos Emeralds collected
			if(players[me].emerald1)
				V_DrawScaledPatch(80, 92, FB, chaos1);
			if(players[me].emerald2)
				V_DrawScaledPatch(104, 92, FB, chaos2);
			if(players[me].emerald3)
				V_DrawScaledPatch(128, 92, FB, chaos3);
			if(players[me].emerald4)
				V_DrawScaledPatch(152, 92, FB, chaos4);
			if(players[me].emerald5)
				V_DrawScaledPatch(176, 92, FB, chaos5);
			if(players[me].emerald6)
				V_DrawScaledPatch(200, 92, FB, chaos6);
			if(players[me].emerald7)
				V_DrawScaledPatch(224, 92, FB, chaos7);
		}

		V_DrawScaledPatch(80, 132, FB, sp_secret);
		WI_drawPercent(232, 132, cnt_ringbonus[0]); // Ring Bonus Tails 03-10-2000

		V_DrawScaledPatch(80, 148, FB, cscore);
		WI_drawPercent(232, 148, cnt_fscore[0]); // Score Tails 03-10-2000
		blinker++; // Tails 08-12-2001
		return;
	}



	if(cv_gametype.value == 4)
	{
		char sscore[33];
		int rings;

		V_DrawString(160+64, 32, "Red");
		V_DrawString(72+64, 32, "Blue");
		V_DrawString(16, 48, "Score");
		sprintf(sscore, "%i", players[0].redscore);
		V_DrawString(176+64, 48, sscore);
		sprintf(sscore, "%i", players[0].bluescore);
		V_DrawString(92+64, 48, sscore);
		V_DrawString(16, 64, "Ring Bonus");
		V_DrawString(16, 80, "Total Score");
		sprintf(sscore, "%i", players[0].redscore);
		V_DrawString(176+64, 48, sscore);
		sprintf(sscore, "%i", players[0].bluescore);
		V_DrawString(92+64, 48, sscore);
		rings = 0;

		// Do Red Team's Score First
		for(i=0; i<MAXPLAYERS; i++)
		{
			if(playeringame[i] && players[i].ctfteam == 1)
				rings += players[i].health - 1;
		}
		if(rings >= 100)
		{
			sprintf(sscore, "%i", players[0].redscore+1);
			V_DrawString(176+64,64, "1");
			V_DrawString(176+64,80, sscore);
		}
		else
		{
			sprintf(sscore, "%i", players[0].redscore);
			V_DrawString(176+64,64, "0");
			V_DrawString(176+64,80, sscore);
		}
		rings = 0;
		// Now Do Blue Team's Score
		for(i=0; i<MAXPLAYERS; i++)
		{
			if(playeringame[i] && players[i].ctfteam == 2)
				rings += players[i].health - 1;
		}
		if(rings >= 100)
		{
			sprintf(sscore, "%i", players[0].bluescore+1);
			V_DrawString(92+64,64, "1");
			V_DrawString(92+64,80, sscore);
		}
		else
		{
			sprintf(sscore, "%i", players[0].bluescore);
			V_DrawString(92+64,64, "0");
			V_DrawString(92+64,80, sscore);
		}
		return;
	}

	else if(cv_gametype.value == 2)
	{
		if(gamemap == 1)
			V_DrawString(72,32, "Greenflower Zone * 1 *");
		else if (gamemap == 2)
			V_DrawString(72,32, "Greenflower Zone * 2 *");
		else if (gamemap == 3)
			V_DrawString(72,32, "Greenflower Zone * 3 *");
		else if (gamemap == 4)
			V_DrawString(72,32, "Techno Hill Zone * 1 *");
		else if (gamemap == 5)
			V_DrawString(72,32, "Techno Hill Zone * 2 *");
		else if (gamemap == 6)
			V_DrawString(72,32, "Techno Hill Zone * 3 *");
		else if (gamemap == 7)
			V_DrawString(88,32, "Deep Sea Zone * 1 *");
		else if (gamemap == 8)
			V_DrawString(88,32, "Deep Sea Zone * 2 *");
		else if (gamemap == 9)
			V_DrawString(88,32, "Deep Sea Zone * 3 *");
		else if (gamemap == 10)
			V_DrawString(87,32, "Mine Maze Zone * 1 *");
		else if (gamemap == 11)
			V_DrawString(87,32, "Mine Maze Zone * 2 *");
		else if (gamemap == 12)
			V_DrawString(87,32, "Mine Maze Zone * 3 *");
		else if (gamemap == 13)
			V_DrawString(64,32, "Rocky Mountain Zone * 1 *");
		else if (gamemap == 14)
			V_DrawString(64,32, "Rocky Mountain Zone * 2 *");
		else if (gamemap == 15)
			V_DrawString(64,32, "Rocky Mountain Zone * 3 *");
		else if (gamemap == 16)
			V_DrawString(72,32, "Red Volcano Zone * 1 *");
		else if (gamemap == 17)
			V_DrawString(72,32, "Red Volcano Zone * 2 *");
		else if (gamemap == 18)
			V_DrawString(72,32, "Red Volcano Zone * 3 *");
		else if (gamemap == 19)
			V_DrawString(87,32, "Dark City Zone * 1 *");
		else if (gamemap == 20)
			V_DrawString(87,32, "Dark City Zone * 2 *");
		else if (gamemap == 21)
			V_DrawString(87,32, "Dark City Zone * 3 *");
		else if (gamemap == 22)
			V_DrawString(87,32, "Doom Ship Zone * 1 *");
		else if (gamemap == 23)
			V_DrawString(87,32, "Doom Ship Zone * 2 *");
		else if (gamemap == 24)
			V_DrawString(87,32, "Doom Ship Zone * 3 *");
		else if (gamemap == 25)
			V_DrawString(88,32, "Egg Rock Zone * 1 *");
		else if (gamemap == 26)
			V_DrawString(88,32, "Egg Rock Zone * 2 *");
		else if (gamemap == 27)
			V_DrawString(88,32, "Egg Rock Zone * 3 *");
		else if (gamemap == 28)
			V_DrawString(74,32, "Final Fight Zone * 1 *");
		else if (gamemap == 29)
			V_DrawString(96,32, "Secret Zone * 1 *");
		else if (gamemap == 30)
			V_DrawString(96,32, "Secret Zone * 2 *");
		else if (gamemap == 31)
			V_DrawString(96,32, "Secret Zone * 3 *");
		else if (gamemap == 32)
			V_DrawString(96,32, "Secret Zone * 4 *");
		else
			V_DrawString(96,32, "* Custom Zone *");

		// Draw the side labels Tails 04-27-2001
		V_DrawString(8,80, "SCORE");
		V_DrawString(8,96, "TIME");
		V_DrawString(8,112,"RING");
		V_DrawString(8,128,"TOT. RING");
		V_DrawString(8,144,"ITEM BOX");
		V_DrawString(0,168,"* TOTAL *");

		topscore[0] = -1; /* cut'n'paste rather than a loop, since it's faster */
		topscore[1] = -1; /* 0s can change to -1 -- see later for details */
		topscore[2] = -1;
		topscore[3] = -1;
		etopscore[0] = -1; /* LEAVE THESE AS -1 !!! */
		etopscore[1] = -1;
		etopscore[2] = -1;
		etopscore[3] = -1;

		toptime[0] = players[0].realtime+1; /* cut'n'paste rather than a loop, since it's faster */
		toptime[1] = players[0].realtime+1; /* 0s can change to -1 -- see later for details */
		toptime[2] = players[0].realtime+1;
		toptime[3] = players[0].realtime+1;
		etoptime[0] = -1; /* LEAVE THESE AS -1 !!! */
		etoptime[1] = -1;
		etoptime[2] = -1;
		etoptime[3] = -1;

		topitembox[0] = -1; /* cut'n'paste rather than a loop, since it's faster */
		topitembox[1] = -1; /* 0s can change to -1 -- see later for details */
		topitembox[2] = -1;
		topitembox[3] = -1;
		etopitembox[0] = -1; /* LEAVE THESE AS -1 !!! */
		etopitembox[1] = -1;
		etopitembox[2] = -1;
		etopitembox[3] = -1;

		topring[0] = -1; /* cut'n'paste rather than a loop, since it's faster */
		topring[1] = -1; /* 0s can change to -1 -- see later for details */
		topring[2] = -1;
		topring[3] = -1;
		etopring[0] = -1; /* LEAVE THESE AS -1 !!! */
		etopring[1] = -1;
		etopring[2] = -1;
		etopring[3] = -1;

		toptotalring[0] = -1; /* cut'n'paste rather than a loop, since it's faster */
		toptotalring[1] = -1; /* 0s can change to -1 -- see later for details */
		toptotalring[2] = -1;
		toptotalring[3] = -1;
		etoptotalring[0] = -1; /* LEAVE THESE AS -1 !!! */
		etoptotalring[1] = -1;
		etoptotalring[2] = -1;
		etoptotalring[3] = -1;

		// Start Calculate Score Position First Tails 04-30-2001
		for (i=0; i<MAXPLAYERS; i++) /* grab highest ranking */
		{
			if (playeringame[i])
			{
				if (topscore[0]<players[i].score)
				{ topscore[0] = players[i].score; etopscore[0] = i; }
			}
		}

		for (i=0; i<MAXPLAYERS; i++) /* grab second ranking */
		{
			if (playeringame[i])
			{
				if (topscore[1]<players[i].score)
				{
					if (i!=etopscore[0]) /* ignore last player */
					{ topscore[1] = players[i].score; etopscore[1] = i; }
				}
			}
		}

		for (i=0; i<MAXPLAYERS; i++) /* grab third ranking */
		{
			if (playeringame[i])
			{
				if (topscore[2]<players[i].score)
				{
					if (i!=etopscore[0]) /* ignore last player */
					{
						if (i!=etopscore[1]) /* ignore last player */
						{ topscore[2] = players[i].score; etopscore[2] = i; }
					}
				}
			}
		}

		for (i=0; i<MAXPLAYERS; i++) /* grab fourth ranking */
		{
			if (playeringame[i])
			{
				if (topscore[3]<players[i].score)
				{
					if (i!=etopscore[0]) /* ignore last player */
					{
						if (i!=etopscore[1]) /* ignore last player */
						{
							if (i!=etopscore[2]) /* ignore last player */
							{ topscore[3] = players[i].score; etopscore[3] = i; }
						}
					}
				}
			}
		}
		// End Calculate Score Position First Tails 04-30-2001

		// Start Calculate Time Position Tails 04-30-2001
		for (i=0; i<MAXPLAYERS; i++) /* grab highest ranking */
		{
			if (playeringame[i])
			{
				if (toptime[0]>players[i].realtime)
				{ toptime[0] = players[i].realtime; etoptime[0] = i; }
			}
		}

		for (i=0; i<MAXPLAYERS; i++) /* grab second ranking */
		{
			if (playeringame[i])
			{
				if (toptime[1]>players[i].realtime)
				{
					if (i!=etoptime[0]) /* ignore last player */
					{ toptime[1] = players[i].realtime; etoptime[1] = i; }
				}
			}
		}

		for (i=0; i<MAXPLAYERS; i++) /* grab third ranking */
		{
			if (playeringame[i])
			{
				if (toptime[2]>players[i].realtime)
				{
					if (i!=etoptime[0]) /* ignore last player */
					{
						if (i!=etoptime[1]) /* ignore last player */
						{ toptime[2] = players[i].realtime; etoptime[2] = i; }
					}
				}
			}
		}

		for (i=0; i<MAXPLAYERS; i++) /* grab fourth ranking */
		{
			if (playeringame[i])
			{
				if (toptime[3]>players[i].realtime)
				{
					if (i!=etoptime[0]) /* ignore last player */
					{
						if (i!=etoptime[1]) /* ignore last player */
						{
							if (i!=etoptime[2]) /* ignore last player */
							{ toptime[3] = players[i].realtime; etoptime[3] = i; }
						}
					}
				}
			}
		}
		// End Calculate Time Position Tails 04-30-2001

		// Start Calculate Item Box Position Tails 04-30-2001
		for (i=0; i<MAXPLAYERS; i++) /* grab highest ranking */
		{
			if (playeringame[i])
			{
				if (topitembox[0]<players[i].numboxes)
				{ topitembox[0] = players[i].numboxes; etopitembox[0] = i; }
			}
		}

		for (i=0; i<MAXPLAYERS; i++) /* grab second ranking */
		{
			if (playeringame[i])
			{
				if (topitembox[1]<players[i].numboxes)
				{
					if (i!=etopitembox[0]) /* ignore last player */
					{ topitembox[1] = players[i].numboxes; etopitembox[1] = i; }
				}
			}
		}

		for (i=0; i<MAXPLAYERS; i++) /* grab third ranking */
		{
			if (playeringame[i])
			{
				if (topitembox[2]<players[i].numboxes)
				{
					if (i!=etopitembox[0]) /* ignore last player */
					{
						if (i!=etopitembox[1]) /* ignore last player */
						{ topitembox[2] = players[i].numboxes; etopitembox[2] = i; }
					}
				}
			}
		}

		for (i=0; i<MAXPLAYERS; i++) /* grab fourth ranking */
		{
			if (playeringame[i])
			{
				if (topitembox[3]<players[i].numboxes)
				{
					if (i!=etopitembox[0]) /* ignore last player */
					{
						if (i!=etopitembox[1]) /* ignore last player */
						{
							if (i!=etopitembox[2]) /* ignore last player */
							{ topitembox[3] = players[i].numboxes; etopitembox[3] = i; }
						}
					}
				}
			}
		}
		// End Calculate Item Box Position Tails 04-30-2001

		// Start Calculate Ring Position Tails 04-30-2001
		for (i=0; i<MAXPLAYERS; i++) /* grab highest ranking */
		{
			if (playeringame[i])
			{
				if (topring[0]<players[i].health)
				{ topring[0] = players[i].health; etopring[0] = i; }
			}
		}

		for (i=0; i<MAXPLAYERS; i++) /* grab second ranking */
		{
			if (playeringame[i])
			{
				if (topring[1]<players[i].health)
				{
					if (i!=etopring[0]) /* ignore last player */
					{ topring[1] = players[i].health; etopring[1] = i; }
				}
			}
		}

		for (i=0; i<MAXPLAYERS; i++) /* grab third ranking */
		{
			if (playeringame[i])
			{
				if (topring[2]<players[i].health)
				{
					if (i!=etopring[0]) /* ignore last player */
					{
						if (i!=etopring[1]) /* ignore last player */
						{ topring[2] = players[i].health; etopring[2] = i; }
					}
				}
			}
		}

		for (i=0; i<MAXPLAYERS; i++) /* grab fourth ranking */
		{
			if (playeringame[i])
			{
				if (topring[3]<players[i].health)
				{
					if (i!=etopring[0]) /* ignore last player */
					{
						if (i!=etopring[1]) /* ignore last player */
						{
							if (i!=etopring[2]) /* ignore last player */
							{ topring[3] = players[i].health; etopring[3] = i; }
						}
					}
				}
			}
		}
		// End Calculate Ring Position Tails 04-30-2001

		// Start Calculate Total Ring Position Tails 04-30-2001
		for (i=0; i<MAXPLAYERS; i++) /* grab highest ranking */
		{
			if (playeringame[i])
			{
				if (toptotalring[0]<players[i].totalring)
				{ toptotalring[0] = players[i].totalring; etoptotalring[0] = i; }
			}
		}

		for (i=0; i<MAXPLAYERS; i++) /* grab second ranking */
		{
			if (playeringame[i])
			{
				if (toptotalring[1]<players[i].totalring)
				{
					if (i!=etoptotalring[0]) /* ignore last player */
					{ toptotalring[1] = players[i].totalring; etoptotalring[1] = i; }
				}
			}
		}

		for (i=0; i<MAXPLAYERS; i++) /* grab third ranking */
		{
			if (playeringame[i])
			{
				if (toptotalring[2]<players[i].totalring)
				{
					if (i!=etoptotalring[0]) /* ignore last player */
					{
						if (i!=etoptotalring[1]) /* ignore last player */
						{ toptotalring[2] = players[i].totalring; etoptotalring[2] = i; }
					}
				}
			}
		}

		for (i=0; i<MAXPLAYERS; i++) /* grab fourth ranking */
		{
			if (playeringame[i])
			{
				if (toptotalring[3]<players[i].totalring)
				{
					if (i!=etoptotalring[0]) /* ignore last player */
					{
						if (i!=etoptotalring[1]) /* ignore last player */
						{
							if (i!=etoptotalring[2]) /* ignore last player */
							{ toptotalring[3] = players[i].totalring; etoptotalring[3] = i; }
						}
					}
				}
			}
		}
		// End Calculate Total Ring Position Tails 04-30-2001

		for (i=0; i<MAXPLAYERS; i++)
		{
			if(playeringame[i])
			{
				// Draw "Top 4" if i > 3
				if(i > 3)
					V_DrawString(140, 48, "TOP 4");

				players[i].racescore = 0;

				if((i == etopscore[0]) && !(topscore[0] == topscore[1]))
					players[i].racescore++;
				if((i == etoptime[0]) && !(toptime[0] == toptime[1]))
					players[i].racescore++;
				if((i == etopring[0]) && !(topring[0] == topring[1]))
					players[i].racescore++;
				if((i == etoptotalring[0]) && !(toptotalring[0] == toptotalring[1]))
					players[i].racescore++;
				if((i == etopitembox[0]) && !(topitembox[0] == topitembox[1]))
					players[i].racescore++;

				racerank[0] = -1;
				racerank[1] = - 1;
				racerank[2] = - 1;
				racerank[3] = - 1;
				eracerank[0] = -1;
				eracerank[1] = -1;
				eracerank[2] = -1;
				eracerank[3] = -1;
			}

			// Start Calculate Player Position Tails 04-30-2001
			for (y=0; y<MAXPLAYERS; y++) // grab highest ranking
			{
				if (playeringame[y])
				{
					if (racerank[0]<players[y].racescore)
					{ racerank[0] = players[y].racescore; eracerank[0] = y; }
				}
			}

			for (y=0; y<MAXPLAYERS; y++) // grab second ranking
			{
				if (playeringame[y])
				{
					if (racerank[1]<players[y].racescore)
					{
						if (y!=eracerank[0]) // ignore last player
						{ racerank[1] = players[y].racescore; eracerank[1] = y; }
					}
				}
			}

			for (y=0; y<MAXPLAYERS; y++) // grab third ranking
			{
				if (playeringame[y])
				{
					if (racerank[2]<players[y].racescore)
					{
						if (y!=eracerank[0]) // ignore last player
						{
							if (y!=eracerank[1]) // ignore last player
							{ racerank[2] = players[y].racescore; eracerank[2] = y; }
						}
					}
				}
			}

			for (y=0; y<MAXPLAYERS; y++) // grab fourth ranking
			{
				if (playeringame[y])
				{
					if (racerank[3]<players[y].racescore)
					{
						if (y!=eracerank[0]) // ignore last player
						{
							if (y!=eracerank[1]) // ignore last player
							{
								if (y!=eracerank[2]) // ignore last player
								{ racerank[3] = players[y].racescore; eracerank[3] = y; }
							}
						}
					}
				}
			}
			// End Calculate Player Position Tails 04-30-2001
		}

		// Now do all the fun score drawing stuff! Tails 05-01-2001
		V_DrawScaledPatch(112,8, FB, W_CachePatchName("RESULT", PU_STATIC)); // Tails 05-06-2001
		y = 0;
		i = eracerank[0];
		for(i=0; i<MAXPLAYERS; i++)
		{
			if(!playeringame[i])
				continue;

			sprintf(splaynum, "%i", i+1);
			sprintf(sscore, "%i", players[i].score);
			sprintf(sseconds, "%i", ((players[i].realtime/TICRATE) % 60));
			sprintf(sminutes, "%i", players[i].realtime/(60*TICRATE));
			sprintf(stics, "%i", players[i].realtime % TICRATE);

			if(players[i].health > 0)
				sprintf(sring, "%i", players[i].health - 1);
			else
				sprintf(sring, "%i", players[i].health);

			sprintf(stotring, "%i", players[i].totalring);
			sprintf(sitembox, "%i", players[i].numboxes);

			sprintf(setopscore, "%i", etopscore[0]+1);
			sprintf(setoptime, "%i", etoptime[0]+1);
			sprintf(setopitembox, "%i", etopitembox[0]+1);
			sprintf(setopring, "%i", etopring[0]+1);
			sprintf(setoptotalring, "%i", etoptotalring[0]+1);
			sprintf(sracescore, "%i", players[i].racescore);

			x = 104+64*y-8;

			V_DrawString(104-((int)strlen(player_names[i])*8)+64*y,60, player_names[i]);
			V_DrawString(104-((int)strlen(splaynum)*8)+64*y-8,68, splaynum);
			V_DrawString(104-((int)strlen("P")*8)+64*y,68, "P");

			V_DrawString(x-(int)strlen(sscore)*8+8,80, sscore);
			if((players[i].realtime % TICRATE) < 10)
			{
				V_DrawString(x-8, 96, "0");
				V_DrawString(x-(int)strlen(stics)*8+8,96, stics);
			}
			else
			{
				V_DrawString(x-(int)strlen(stics)*8+8,96, stics);
			}
			V_DrawString(x-16,96, ":");
			if(players[i].seconds < 10)
			{
				V_DrawString(x-32, 96, "0");
				V_DrawString(x-24-(int)strlen(sseconds)*8+8,96, sseconds);
			}
			else
			{
				V_DrawString(x-24-(int)strlen(sseconds)*8+8,96, sseconds);
			}
			V_DrawString(x-40,96, ":");
			V_DrawString(x-56-(int)strlen(sminutes)*8+16,96, sminutes);
			V_DrawString(x-(int)strlen(sring)*8+8,112, sring);
			V_DrawString(x-(int)strlen(stotring)*8+8,128, stotring);
			V_DrawString(x-(int)strlen(sitembox)*8+8,144, sitembox);

			V_DrawString(x-(int)strlen(sracescore)*8+8,168, sracescore);
	//		V_DrawString(x,168, sracescore);

			V_DrawString(304,64, "W");

			// if 0 and 1 are the same, draw TIED instead
			if(topscore[0] == topscore[1])
				V_DrawStringWhite(304,80, "T");
			else
			{
				V_DrawStringWhite(300,80, setopscore);
				V_DrawStringWhite(308,80, "P");
			}
			if(toptime[0] == toptime[1])
				V_DrawStringWhite(304,96, "T");
			else
			{
				V_DrawStringWhite(300,96, setoptime);
				V_DrawStringWhite(308,96, "P");
			}
			if(topring[0] == topring[1])
				V_DrawStringWhite(304,112, "T");
			else
			{
				V_DrawStringWhite(300,112,setopring);
				V_DrawStringWhite(308,112,"P");
			}
			if(toptotalring[0] == toptotalring[1])
				V_DrawStringWhite(304,128, "T");
			else
			{
				V_DrawStringWhite(300,128,setoptotalring);
				V_DrawStringWhite(308,128,"P");
			}
			if(topitembox[0] == topitembox[1])
				V_DrawStringWhite(304,144, "T");
			else
			{
				V_DrawStringWhite(300,144,setopitembox);
				V_DrawStringWhite(308,144,"P");
			}

			if(i == eracerank[3])
				break;
			else
			{
				if(i == eracerank[0])
					i = eracerank[1];
				else if (i == eracerank[1])
					i = eracerank[2];
				else if (i == eracerank[2])
					i = eracerank[3];
				y++;
			}
		}

		if(players[eracerank[0]].racescore == players[eracerank[1]].racescore)
			V_DrawStringWhite(160-((int)strlen("TIED")/2)*8, 184, "TIED");
		else
		{
			V_DrawStringWhite(160-((int)strlen(player_names[eracerank[0]])/2)*8, 180, player_names[eracerank[0]]);
			V_DrawStringWhite(160-((int)strlen(" WINS")/2)*8, 188, " WINS");
		}

		return;
	}

	// draw stat titles (top line)

	if(players[me].skin == 0)
		V_DrawScaledPatch(88,48, FB, soncpass); // Tails
	else if(players[me].skin == 1)
		V_DrawScaledPatch(88,48, FB, tailpass); // Tails
	else if(players[me].skin == 2)
		V_DrawScaledPatch(40,48, FB, knuxpass); // Tails
	else
		V_DrawScaledPatch(96,48, FB, youpass); // Tails

	V_DrawScaledPatch(72,48, FB, haspassed); // Tails

// Start act selection Tails 11-21-2000
	switch(gamemap)
	{
		case 1:
			act = act1;
			break;
		case 2:
			act = act2;
			break;
		case 3:
			act = act3;
			break;
		case 4:
			act = act1;
			break;
		case 5:
			act = act2;
			break;
		case 6:
			act = act3;
			break;
		case 7:
			act = act1;
			break;
		case 8:
			act = act2;
			break;
		case 9:
			act = act3;
			break;
		case 10:
			act = act1;
			break;
		case 11:
			act = act2;
			break;
		case 12:
			act = act3;
			break;
		case 13:
			act = act1;
			break;
		case 14:
			act = act2;
			break;
		case 15:
			act = act3;
			break;
		case 16:
			act = act1;
			break;
		case 17:
			act = act2;
			break;
		case 18:
			act = act3;
			break;
		case 19:
			act = act1;
			break;
		case 20:
			act = act2;
			break;
		case 21:
			act = act3;
			break;
		case 22:
			act = act1;
			break;
		case 23:
			act = act2;
			break;
		case 24:
			act = act3;
			break;
		case 25:
			act = act1;
			break;
		case 26:
			act = act2;
			break;
		case 27:
			act = act3;
			break;
		default:
			act = act1;
			break;
	}

	V_DrawScaledPatch (244, 56, FB, act); // Draw the act numeral Tails 11-21-2000

	// End act selection Tails 11-21-2000

	V_DrawScaledPatch(SP_STATSX, SP_STATSY+lh, FB, items);
	WI_drawPercent(BASEVIDWIDTH - SP_STATSX, SP_STATSY+lh, cnt_timebonus[me]); // Time Bonus Tails 03-10-2000

	V_DrawScaledPatch(SP_STATSX, SP_STATSY+2*lh, FB, sp_secret);
	WI_drawPercent(BASEVIDWIDTH - SP_STATSX, SP_STATSY+2*lh, cnt_ringbonus[me]); // Ring Bonus Tails 03-10-2000

	V_DrawScaledPatch(SP_STATSX+20, SP_STATSY+4*lh, FB, kills);
	WI_drawPercent(BASEVIDWIDTH - SP_STATSX, SP_STATSY+4*lh, cnt_score[me]); // Total Tails 03-10-2000

	V_DrawScaledPatch(16, 10, FB, fscore);
	WI_drawPercent(128, 10, cnt_fscore[me]); // Score Tails 03-10-2000

	V_DrawScaledPatch(SP_TIMEX, SP_TIMEY, FB, time);

	// Draw the time Tails 11-21-2000
	if(players[me].seconds < 10)
	{
		WI_drawPercent(104, 26, 0);
	}
	WI_drawPercent(112, 26, players[me].seconds);

	WI_drawPercent(88, 26, players[me].minutes);

	V_DrawScaledPatch (88,26, FB, colon); // colon location Tails 02-29-2000
}

static int      sp_state;

void WI_initStats(void)
{
	state = StatCount;
	acceleratestage = 0;
	sp_state = 1;

	if(!(gamemap == SSSTAGE1 || gamemap == SSSTAGE2 || gamemap == SSSTAGE3
		|| gamemap == SSSTAGE4 || gamemap == SSSTAGE5 || gamemap == SSSTAGE6
		|| gamemap == SSSTAGE7))
	{
		if (players[me].realtime / TICRATE < 30)
			players[me].timebonus = 50000;

		else if (players[me].realtime / TICRATE < 45 && players[me].realtime / TICRATE >= 30)
			players[me].timebonus = 10000;

		else if (players[me].realtime / TICRATE < 60 && players[me].realtime / TICRATE >= 45)
			players[me].timebonus = 5000;

		else if (players[me].realtime / TICRATE < 90 && players[me].realtime / TICRATE >= 60)
			players[me].timebonus = 4000;

		else if (players[me].realtime / TICRATE < 120 && players[me].realtime / TICRATE >= 90)
			players[me].timebonus = 3000;

		else if (players[me].realtime / TICRATE < 180 && players[me].realtime / TICRATE >= 120)
			players[me].timebonus = 2000;

		else if (players[me].realtime / TICRATE < 240 && players[me].realtime / TICRATE >= 180)
			players[me].timebonus = 1000;

		else if (players[me].realtime / TICRATE < 300 && players[me].realtime / TICRATE >= 240)
			players[me].timebonus = 500;

		else
			players[me].timebonus = 0;

		cnt_timebonus[0] = (players[me].timebonus); // Time Bonus Tails 03-10-2000
	}
	cnt_fscore[0] = (plrs[me].sscore); // Tails 03-14-2000
	cnt_score[0] = (plrs[me].sscore); // Score Tails 03-09-2000
	cnt_ringbonus[0] = (100*(players[me].health - 1)); // Ring Bonus Tails 03-10-2000
	cnt_pause = TICRATE;
}

void WI_updateStats(void)
{
	if (acceleratestage && sp_state != 10)
	{
		acceleratestage = 0;
		cnt_fscore[me] = (plrs[me].sscore + players[me].timebonus + (100*(players[me].health - 1))); // Tails 03-14-2000
		cnt_score[me] = (plrs[me].sscore + players[me].timebonus + (100*(players[me].health - 1))); // Score Tails 03-09-2000
		cnt_timebonus[me] = 0; // Time Bonus Tails 03-10-2000
		cnt_ringbonus[me] = 0; // Ring Bonus Tails 03-10-2000
		// Gives lives for score Tails 04-11-2001
		if(cnt_fscore[me] >= 50000+players[me].xtralife2)
		{
			players[me].lives++;
			S_StopMusic();
			S_ChangeMusic(mus_xtlife, false);
			players[me].xtralife2 += 50000;
		}
		// End score Tails 04-11-2001
		S_StartSound(0, sfx_chchng); // Tails
		sp_state = 10;
	}

	if (sp_state == 2)
	{
		if(!(gamemap == SSSTAGE1 || gamemap == SSSTAGE2 || gamemap == SSSTAGE3
			|| gamemap == SSSTAGE4 || gamemap == SSSTAGE5 || gamemap == SSSTAGE6
			|| gamemap == SSSTAGE7))
		{
			if((cnt_timebonus[me] > 0) && (cnt_ringbonus[me] > 0))
			{
				cnt_fscore[me] += 444; // Tails 03-14-2000
				cnt_score[me] += 444; // Score Tails 03-09-2000
			}
			else
			{
				cnt_fscore[me] += 222; // Tails 03-14-2000
				cnt_score[me] += 222; // Score Tails 03-09-2000
			}
			cnt_timebonus[me] -= 222; // Time Bonus Tails 03-10-2000
			cnt_ringbonus[me] -= 222; // Ring Bonus Tails 03-10-2000

			if (!(bcnt&1)) // count sound faster! Tails 03-10-2000
				S_StartSound(0, sfx_menu1);

			// Gives lives for score Tails 04-11-2001
			if(cnt_fscore[me] >= 50000+players[me].xtralife2)
			{
				players[me].lives++;
				S_StopMusic();
				S_ChangeMusic(mus_xtlife, false);
				players[me].xtralife2 += 50000;
			}
			// End score Tails 04-11-2001
			// start score, time bonus, ring bonus Tails 03-09-2000

			if (cnt_fscore[me] >= (plrs[me].sscore + players[me].timebonus + (100*(players[me].health - 1))))
			{
				cnt_fscore[me] = (plrs[me].sscore + players[me].timebonus + (100*(players[me].health - 1)));
			}

			if (cnt_score[me] >= (plrs[me].sscore + players[me].timebonus + (100*(players[me].health - 1))))
			{
				cnt_score[me] = (plrs[me].sscore + players[me].timebonus + (100*(players[me].health - 1)));
			}

			if (cnt_timebonus[me] <= 0)
			{
				cnt_timebonus[me] = 0;
			}

			if (cnt_ringbonus[me] <= 0)
			{
				cnt_ringbonus[me] = 0;
			}

			if ((cnt_ringbonus[me] == 0) && (cnt_timebonus[me] == 0) && (cnt_score[me] == plrs[me].sscore + players[me].timebonus + (100*(players[me].health - 1))))
			{
				S_StartSound(0, sfx_chchng); // Tails
				sp_state = 10;
			}
		}
		else
		{
			cnt_fscore[me] += 222; // Tails 03-14-2000
			cnt_ringbonus[me] -= 222; // Ring Bonus Tails 03-10-2000

			if (!(bcnt&1)) // count sound faster! Tails 03-10-2000
				S_StartSound(0, sfx_menu1);

			// Gives lives for score Tails 04-11-2001
			if(cnt_fscore[me] >= 50000+players[me].xtralife2)
			{
				players[me].lives++;
				S_StopMusic();
				S_ChangeMusic(mus_xtlife, false);
				players[me].xtralife2 += 50000;
			}
			// End score Tails 04-11-2001
			// start score, time bonus, ring bonus Tails 03-09-2000

			if (cnt_fscore[me] >= (plrs[me].sscore + (100*(players[me].health - 1))))
			{
				cnt_fscore[me] = (plrs[me].sscore + (100*(players[me].health - 1)));
			}

			if (cnt_ringbonus[me] <= 0)
			{
				cnt_ringbonus[me] = 0;
			}

			if ((cnt_ringbonus[me] == 0) && (cnt_fscore[me] == plrs[me].sscore + (100*(players[me].health - 1))))
			{
				S_StartSound(0, sfx_chchng); // Tails
				sp_state = 10;
			}
		}
	}
	else if (sp_state == 4)
	{
		if (!(bcnt&3))
			S_StartSound(0, sfx_menu1);
	}
	else if (sp_state == 6)
	{
		if (!(bcnt&3))
			S_StartSound(0, sfx_menu1);
	}
	else if (sp_state == 10)
	{
		if (acceleratestage)
		{
			players[me].score = cnt_fscore[me];
			WI_initNoState();
		}
	}
	else if (sp_state & 1)
	{
		if (!--cnt_pause)
		{
			sp_state++;
			cnt_pause = TICRATE;
		}
	}

}

void WI_drawStats(void)
{
	// line height
	int lh;
	static patch_t* act; // Tails 11-21-2000

	lh = (3*SHORT(num[0]->height))/2;

	WI_slamBackground();

	if(cv_gametype.value == 2) // Don't show 'Has Passed' stuff in Race Mode Tails 04-27-2001
		return;

	if(!(gamemap == SSSTAGE1 || gamemap == SSSTAGE2 || gamemap == SSSTAGE3
		|| gamemap == SSSTAGE4 || gamemap == SSSTAGE5 || gamemap == SSSTAGE6
		|| gamemap == SSSTAGE7))
	{
		if(players[me].skin == 0)
			V_DrawScaledPatch(88,48, FB, soncpass); // Tails
		else if(players[me].skin == 1)
			V_DrawScaledPatch(88,48, FB, tailpass); // Tails
		else if(players[me].skin == 2)
			V_DrawScaledPatch(40,48, FB, knuxpass); // Tails
		else
			V_DrawScaledPatch(96,48, FB, youpass); // Tails

		V_DrawScaledPatch(72,48, FB, haspassed); // Tails

		// Start act selection Tails 11-21-2000
		switch(gamemap)
		{
			case 1:
				act = act1;
				break;
			case 2:
				act = act2;
				break;
			case 3:
				act = act3;
				break;
			case 4:
				act = act1;
				break;
			case 5:
				act = act2;
				break;
			case 6:
				act = act3;
				break;
			case 7:
				act = act1;
				break;
			case 8:
				act = act2;
				break;
			case 9:
				act = act3;
				break;
			case 10:
				act = act1;
				break;
			case 11:
				act = act2;
				break;
			case 12:
				act = act3;
				break;
			case 13:
				act = act1;
				break;
			case 14:
				act = act2;
				break;
			case 15:
				act = act3;
				break;
			case 16:
				act = act1;
				break;
			case 17:
				act = act2;
				break;
			case 18:
				act = act3;
				break;
			case 19:
				act = act1;
				break;
			case 20:
				act = act2;
				break;
			case 21:
				act = act3;
				break;
			case 22:
				act = act1;
				break;
			case 23:
				act = act2;
				break;
			case 24:
				act = act3;
				break;
			case 25:
				act = act1;
				break;
			case 26:
				act = act2;
				break;
			case 27:
				act = act3;
				break;
			default:
				act = act1;
				break;
		}

		V_DrawScaledPatch (244, 56, FB, act); // Draw the act numeral Tails 11-21-2000

		// End act selection Tails 11-21-2000
		V_DrawScaledPatch(SP_STATSX, SP_STATSY+lh, FB, items);
		WI_drawPercent(BASEVIDWIDTH - SP_STATSX, SP_STATSY+lh, cnt_timebonus[0]); // Time Bonus Tails 03-10-2000

		V_DrawScaledPatch(SP_STATSX, SP_STATSY+2*lh, FB, sp_secret);
		WI_drawPercent(BASEVIDWIDTH - SP_STATSX, SP_STATSY+2*lh, cnt_ringbonus[0]); // Ring Bonus Tails 03-10-2000

		V_DrawScaledPatch(SP_STATSX+20, SP_STATSY+4*lh, FB, kills);
		WI_drawPercent(BASEVIDWIDTH - SP_STATSX, SP_STATSY+4*lh, cnt_score[0]); // Total Tails 03-10-2000

		V_DrawScaledPatch(16, 10, FB, fscore);
		WI_drawPercent(128, 10, cnt_fscore[0]); // Score Tails 03-10-2000

		V_DrawScaledPatch(SP_TIMEX, SP_TIMEY, FB, time);

		// Draw the time Tails 11-21-2000
		if(cv_timetic.value) // new TimeTic option Tails 04-02-2001
		{
			WI_drawPercent(112, 26, players[me].realtime);
		}
		else
		{
			if(players[me].seconds < 10)
			{
				WI_drawPercent(104, 26, 0);
			}

			WI_drawPercent(112, 26, players[me].seconds);

			WI_drawPercent(88, 26, players[me].minutes);

			V_DrawScaledPatch (88,26, FB, colon); // colon location Tails 02-29-2000
		}
	}
	else // Special Stage Drawer Tails 08-12-2001
	{
		// Draw big CHAOS EMERALDS thing
		if(players[me].emerald1 && players[me].emerald2 && players[me].emerald3 && players[me].emerald4 && players[me].emerald5 && players[me].emerald6 && players[me].emerald7)
		{
			if(sp_state == 10)
				V_DrawScaledPatch(48, 32, FB, nowsuper);
			else
				V_DrawScaledPatch(70, 26, FB, gotemall);
		}
		else
			V_DrawScaledPatch(48, 26, FB, cemerald);

		if(blinker & 1)
		{
			// Display Chaos Emeralds collected
			if(players[me].emerald1)
				V_DrawScaledPatch(80, 92, FB, chaos1);
			if(players[me].emerald2)
				V_DrawScaledPatch(104, 92, FB, chaos2);
			if(players[me].emerald3)
				V_DrawScaledPatch(128, 92, FB, chaos3);
			if(players[me].emerald4)
				V_DrawScaledPatch(152, 92, FB, chaos4);
			if(players[me].emerald5)
				V_DrawScaledPatch(176, 92, FB, chaos5);
			if(players[me].emerald6)
				V_DrawScaledPatch(200, 92, FB, chaos6);
			if(players[me].emerald7)
				V_DrawScaledPatch(224, 92, FB, chaos7);
		}

	V_DrawScaledPatch(80, 132, FB, sp_secret);
	WI_drawPercent(232, 132, cnt_ringbonus[0]); // Ring Bonus Tails 03-10-2000

	V_DrawScaledPatch(80, 148, FB, cscore);
	WI_drawPercent(232, 148, cnt_fscore[0]); // Score Tails 03-10-2000
	}

	blinker++; // Tails 08-12-2001
}

void WI_checkForAccelerate(void)
{
	int   i;
	player_t  *player;

	// check for button presses to skip delays
	for (i=0, player = players ; i<MAXPLAYERS ; i++, player++)
	{
		if (playeringame[i])
		{
			if (player->cmd.buttons & BT_JUMP) // Tails 07-20-2001
			{
				if (!player->jumpdown) // Tails 07-20-2001
					acceleratestage = 1;
				player->jumpdown = true; // Tails 07-20-2001
			}
			else
				player->jumpdown = false; // Tails 07-20-2001
			if (player->cmd.buttons & BT_USE)
			{
				if (!player->usedown)
					acceleratestage = 1;
				player->usedown = true;
			}
			else
				player->usedown = false;
		}
	}
}



// Updates stuff each tick
void WI_Ticker(void)
{
	// counter for general background animation
	bcnt++;

	if (bcnt == 1)
	{
		// intermission music
		S_ChangeMusic(mus_dm2int, false); // Tails 03-14-2000
	}

	WI_checkForAccelerate();

	switch (state)
	{
	  case StatCount:
		if (cv_deathmatch.value) WI_updateDeathmatchStats();
		else if (multiplayer) WI_updateNetgameStats();
		else WI_updateStats();
		break;

	  case NoState:
		WI_updateNoState();
		break;
	}

}

void WI_loadData(void)
{
	int         i;
	char        name[9];

	// choose the background of the intermission
	if(cv_gametype.value == 2) // Tails 04-27-2001
	{
		strlcpy(bgname, "SRB2BACK", sizeof(bgname)); // Tails 04-27-2001
	}
	else
	{
		if (gamemode == commercial){ // Tails 12-02-99
			if(gamemap == SSSTAGE1 || gamemap == SSSTAGE2 || gamemap == SSSTAGE3
				|| gamemap == SSSTAGE4 || gamemap == SSSTAGE5 || gamemap == SSSTAGE6
				|| gamemap == SSSTAGE7)
						strlcpy(bgname, "SPECIALB", sizeof(bgname)); // Tails 08-11-2001
			else
			{
				// Tails 12-02-99
				if (gamemap == 1)
					strlcpy(bgname, "SONICPIC", sizeof(bgname));
				else if (gamemap == 2)
					strlcpy(bgname, "TAILSPIC", sizeof(bgname));
				else if (gamemap == 3)
					strlcpy(bgname, "KNUXPIC", sizeof(bgname));
				else if (gamemap == 4)
					strlcpy(bgname, "EGGPIC", sizeof(bgname));
				else
					strlcpy(bgname, "INTERPIC", sizeof(bgname));
			}
		}
	}

	// background stored in backbuffer
	V_DrawScaledPatch(0, 0, 1, W_CachePatchName(bgname, PU_CACHE));

	// More hacks on minus sign.
	wiminus = W_CachePatchName("WIMINUS", PU_STATIC);

	for (i=0;i<10;i++)
	{
		 // numbers 0-9
		sprintf(name, "WINUM%d", i);
		num[i] = W_CachePatchName(name, PU_STATIC);
	}

	// percent sign
	percent = W_CachePatchName("WIPCNT", PU_STATIC);

	// "finished"
	finished = W_CachePatchName("WIF", PU_STATIC);

	// "entering"
	entering = W_CachePatchName("WIENTER", PU_STATIC);

	cscore = W_CachePatchName("CSCORE", PU_STATIC); // Tails 08-11-2001

	gotemall = W_CachePatchName("GOTEMALL", PU_STATIC); // Tails 08-12-2001
	cemerald = W_CachePatchName("CEMERALD", PU_STATIC); // Tails 08-12-2001
	nowsuper = W_CachePatchName("NOWSUPER", PU_STATIC); // Tails 08-12-2001
	chaos1 = W_CachePatchName("CHAOS1", PU_STATIC); // Tails 08-12-2001
	chaos2 = W_CachePatchName("CHAOS2", PU_STATIC); // Tails 08-12-2001
	chaos3 = W_CachePatchName("CHAOS3", PU_STATIC); // Tails 08-12-2001
	chaos4 = W_CachePatchName("CHAOS4", PU_STATIC); // Tails 08-12-2001
	chaos5 = W_CachePatchName("CHAOS5", PU_STATIC); // Tails 08-12-2001
	chaos6 = W_CachePatchName("CHAOS6", PU_STATIC); // Tails 08-12-2001
	chaos7 = W_CachePatchName("CHAOS7", PU_STATIC); // Tails 08-12-2001

	// "kills"
	kills = W_CachePatchName("WIOSTK", PU_STATIC);

	fscore = W_CachePatchName("SBOFRAGS", PU_STATIC); // Tails 03-14-2000

	// "scrt"
	secret = W_CachePatchName("WIOSTS", PU_STATIC);

	 // "secret"
	sp_secret = W_CachePatchName("WISCRT2", PU_STATIC);

	items = W_CachePatchName("WIOSTI", PU_STATIC);

	// "frgs"
	frags = W_CachePatchName("WIFRGS", PU_STATIC);

	// ":"
	colon = W_CachePatchName("WICOLON", PU_STATIC);

	// "time"
	time = W_CachePatchName("SBOARMOR", PU_STATIC); // Tails

	// "sucks"
	sucks = W_CachePatchName("WISUCKS", PU_STATIC);

	// "killers" (vertical)
	killers = W_CachePatchName("WIKILRS", PU_STATIC);

	// "victims" (horiz)
	victims = W_CachePatchName("WIVCTMS", PU_STATIC);

	// "total"
	total = W_CachePatchName("WIMSTT", PU_STATIC);

	// your face
	star = W_CachePatchName("STFST01", PU_STATIC);

	// dead face
	bstar = W_CachePatchName("STFDEAD0", PU_STATIC);

	haspassed = W_CachePatchName("HPASSED", PU_STATIC); // Tails
	soncpass = W_CachePatchName("SONCPASS", PU_STATIC); // Tails
	knuxpass = W_CachePatchName("KNUXPASS", PU_STATIC); // Tails
	tailpass = W_CachePatchName("TAILPASS", PU_STATIC); // Tails
	youpass = W_CachePatchName("YOUPASS", PU_STATIC); // Tails
	act1 = W_CachePatchName("TTLONE", PU_STATIC); // Tails
	act2 = W_CachePatchName("TTLTWO", PU_STATIC); // Tails
	act3 = W_CachePatchName("TTLTHREE", PU_STATIC); // Tails

	//added:08-02-98: now uses a single STPB0 which is remapped to the
	//                player translation table. Whatever new colors we add
	//                since we'll have to define a translation table for
	//                it, we'll have the right colors here automatically.
	stpb = W_CachePatchName("STPB0", PU_STATIC);
}

static void WI_unloadData(void)
{
	int         i;

	//faB: never Z_ChangeTag() a pointer returned by W_CachePatchxxx()
	//     it doesn't work and is unecessary
	if (rendermode==render_soft)
	{
		Z_ChangeTag(wiminus, PU_CACHE);

		for (i=0 ; i<10 ; i++)
			Z_ChangeTag(num[i], PU_CACHE);
	}

	if (rendermode==render_soft)
	{
		Z_ChangeTag(percent, PU_CACHE);
		Z_ChangeTag(colon, PU_CACHE);
		Z_ChangeTag(finished, PU_CACHE);
		Z_ChangeTag(entering, PU_CACHE);
		Z_ChangeTag(cscore, PU_CACHE); // Tails 08-11-2001
		Z_ChangeTag(cemerald, PU_CACHE); // Tails 08-12-2001
		Z_ChangeTag(gotemall, PU_CACHE); // Tails 08-12-2001
		Z_ChangeTag(nowsuper, PU_CACHE); // Tails 08-12-2001
		Z_ChangeTag(chaos1, PU_CACHE); // Tails 08-12-2001
		Z_ChangeTag(chaos2, PU_CACHE); // Tails 08-12-2001
		Z_ChangeTag(chaos3, PU_CACHE); // Tails 08-12-2001
		Z_ChangeTag(chaos4, PU_CACHE); // Tails 08-12-2001
		Z_ChangeTag(chaos5, PU_CACHE); // Tails 08-12-2001
		Z_ChangeTag(chaos6, PU_CACHE); // Tails 08-12-2001
		Z_ChangeTag(chaos7, PU_CACHE); // Tails 08-12-2001
		Z_ChangeTag(kills, PU_CACHE);
		Z_ChangeTag(fscore, PU_CACHE); // Oops! Forgot this! Tails 08-11-2001
		Z_ChangeTag(secret, PU_CACHE);
		Z_ChangeTag(sp_secret, PU_CACHE);
		Z_ChangeTag(items, PU_CACHE);
		Z_ChangeTag(frags, PU_CACHE);
		Z_ChangeTag(time, PU_CACHE);
		Z_ChangeTag(sucks, PU_CACHE);

		// Extra ChangeTags Tails 08-11-2001
		Z_ChangeTag(haspassed, PU_CACHE);
		Z_ChangeTag(soncpass, PU_CACHE);
		Z_ChangeTag(knuxpass, PU_CACHE);
		Z_ChangeTag(tailpass, PU_CACHE);
		Z_ChangeTag(youpass, PU_CACHE);
		Z_ChangeTag(act1, PU_CACHE);
		Z_ChangeTag(act2, PU_CACHE);
		Z_ChangeTag(act3, PU_CACHE);

		Z_ChangeTag(victims, PU_CACHE);
		Z_ChangeTag(killers, PU_CACHE);
		Z_ChangeTag(total, PU_CACHE);
	}
}

void WI_Drawer (void)
{
	switch (state)
	{
	  case StatCount:
		if (cv_deathmatch.value)
		{
			if(cv_teamplay.value)
				WI_drawTeamsStats();
			else
				WI_drawDeathmatchStats();
		}
		else if (multiplayer)
			WI_drawNetgameStats();
		else
			WI_drawStats();
		break;

	  case NoState:
		WI_drawNoState();
		break;
	}
}


void WI_initVariables(wbstartstruct_t* wbstartstruct)
{
	wbs = wbstartstruct;

#ifdef RANGECHECKING
	RNGCHECK(wbs->last, 0, 8);
	RNGCHECK(wbs->next, 0, 8);
	RNGCHECK(wbs->pnum, 0, MAXPLAYERS);
	RNGCHECK(wbs->pnum, 0, MAXPLAYERS);
#endif

	acceleratestage = 0;
	cnt = bcnt = 0;
	firstrefresh = 1;
	me = wbs->pnum;
	plrs = wbs->plyr;
}

void WI_Start(wbstartstruct_t* wbstartstruct)
{
	blinker = 0; // Tails 08-17-2001
	WI_initVariables(wbstartstruct);
	WI_loadData();

	if (cv_deathmatch.value)
		WI_initDeathmatchStats();
	else if (multiplayer)
		WI_initNetgameStats();
	else
		WI_initStats();
}
