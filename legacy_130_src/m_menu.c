// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: m_menu.c,v 1.15 2000/04/24 15:10:56 hurdler Exp $
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
// $Log: m_menu.c,v $
// Revision 1.15  2000/04/24 15:10:56  hurdler
// Support colormap for text
//
// Revision 1.14  2000/04/23 00:29:28  hurdler
// fix a small bug in skin color
//
// Revision 1.13  2000/04/23 00:25:20  hurdler
// fix a small bug in skin color
//
// Revision 1.12  2000/04/22 21:12:15  hurdler
// I like it better like that
//
// Revision 1.11  2000/04/22 20:27:35  metzgermeister
// support for immediate fullscreen switching
//
// Revision 1.10  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.9  2000/04/13 16:26:41  hurdler
// looks better like that
//
// Revision 1.8  2000/04/12 19:31:37  metzgermeister
// added use_mouse to menu
//
// Revision 1.7  2000/04/08 17:29:24  stroggonmeth
// no message
//
// Revision 1.6  2000/04/07 23:11:17  metzgermeister
// added mouse move
//
// Revision 1.5  2000/04/04 10:44:00  hurdler
// Remove a warning message in Dos/Windows
//
// Revision 1.4  2000/04/04 00:32:46  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.3  2000/03/23 22:54:00  metzgermeister
// added support for HOME/.legacy under Linux
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      DOOM selection menu, options, episode etc.
//      Sliders and icons. Kinda widget stuff.
//
// NOTE:
//      All V_DrawPatchDirect () has been replaced by V_DrawScaledPatch ()
//      so that the menu is scaled to the screen size. The scaling is always
//      an integer multiple of the original size, so that the graphics look
//      good.
//
//-----------------------------------------------------------------------------

#ifndef __WIN32__
#include <unistd.h>
#endif
#include <fcntl.h>

#include "am_map.h"

#include "doomdef.h"
#include "dstrings.h"
#include "d_main.h"

#include "console.h"

#include "r_local.h"
#include "hu_stuff.h"
#include "g_game.h"
#include "g_input.h"

#include "m_argv.h"

// Data.
#include "sounds.h"
#include "s_sound.h"
#include "i_system.h"

#include "m_menu.h"
#include "m_misc.h"
#include "v_video.h"
#include "i_video.h"

#include "keys.h"
#include "z_zone.h"
#include "w_wad.h"
#include "p_local.h"
#include "p_fab.h"

#ifdef HWRENDER
#include "hardware/hw_main.h"
#endif

// -1 = no quicksave slot picked!
int                     quickSaveSlot;
boolean                 menuactive;

extern consvar_t cv_numsnow; // Tails 12-25-2001

#define SKULLXOFF       -32
#define LINEHEIGHT      16
#define SMALLLINEHEIGHT  8
#define SLIDER_RANGE    10
#define SLIDER_WIDTH    (8*SLIDER_RANGE+6)

// we are going to be entering a savegame string
int                     saveStringEnter;
int                     saveSlot;       // which slot to save in
int                     saveCharIndex;  // which char we're editing
// old save description before edit
char                    saveOldString[SAVESTRINGSIZE];

char                    savegamestrings[10][SAVESTRINGSIZE];

char    endstring[160];

// flags for items in the menu
#define  IT_TYPE             14     // (2+4+8)
#define  IT_CALL              0
#define  IT_ARROWS            2
#define  IT_KEYHANDLER        4
#define  IT_SUBMENU           6     // (2+4)
#define  IT_CVAR              8
#define  IT_SPACE            10
#define  IT_MSGHANDLER       12

#define  IT_DISPLAY      (48+64)     // 16+32+64
#define  IT_NOTHING           0
#define  IT_PATCH            16
#define  IT_STRING           32
#define  IT_WHITESTRING      48
#define  IT_DYBIGSPACE       64
#define  IT_DYLITLSPACE  (16+64)
#define  IT_STRING2      (32+64)
#define  IT_GRAYPATCH    (16+32+64)

//consvar specific
#define  IT_CVARTYPE   (128+256+512)
#define  IT_CV_NORMAL         0
#define  IT_CV_SLIDER       128
#define  IT_CV_STRING       256
#define  IT_CV_NOPRINT (128+256)
#define  IT_CV_NOMOD        512

// in short for some common use
#define  IT_BIGSPACE    (IT_SPACE  +IT_DYBIGSPACE)
#define  IT_LITLSPACE   (IT_SPACE  +IT_DYLITLSPACE)
#define  IT_CONTROL     (IT_STRING2+IT_CALL)
#define  IT_CVARMAX     (IT_CVAR   +IT_CV_NOMOD)
#define  IT_DISABLED    (IT_SPACE  +IT_GRAYPATCH)

typedef union
{
	struct menu_s     *submenu;               // IT_SUBMENU
	consvar_t         *cvar;                  // IT_CVAR
	void             (*routine)(int choice);  // IT_CALL, IT_KEYHANDLER, IT_ARROWS
} itemaction_t;

//
// MENU TYPEDEFS
//
typedef struct menuitem_s
{
	// show IT_xxx
	short       status;

	char        *name;

	// FIXME: should be itemaction_t !!!
	void *itemaction;

	// hotkey in menu
	unsigned char   alphaKey;
} menuitem_t;

typedef struct menu_s
{
	char            *menutitlepic;
	short           numitems;               // # of menu items
	struct menu_s*  prevMenu;               // previous menu
	menuitem_t*     menuitems;              // menu items
	void            (*drawroutine)(void);   // draw routine
	short           x;
	short           y;                      // x,y of menu
	short           lastOn;                 // last item user was on in menu
	boolean         (*quitroutine)(void);   // called before quit a menu return true if we can
} menu_t;

// current menudef
menu_t*   currentMenu;
short     itemOn;                       // menu item skull is on
short     skullAnimCounter;             // skull animation counter
short     whichSkull;                   // which skull to draw

// graphic name of skulls
char      skullName[2][9] = {"M_SKULL1","M_SKULL2"};

//
// PROTOTYPES
//
void M_DrawSaveLoadBorder(int x,int y);
void M_SetupNextMenu(menu_t *menudef);

void M_DrawTextBox (int x, int y, int width, int lines);     //added:06-02-98:
void M_DrawThermo(int x,int y,int thermWidth,int thermDot);
void M_DrawEmptyCell(menu_t *menu,int item);
void M_DrawSelCell(menu_t *menu,int item);
void M_DrawSlider (int x, int y, int range);
void M_CentreText(int y, char* string);        //added:30-01-98:writetext centered

void M_StartControlPanel(void);
void M_StartMessage(char *string,void *routine,int itemtype);
void M_StopMessage(int choice);
void M_ClearMenus (void);
int  M_StringHeight(char* string);
void M_GameOption(int choice);
void M_NetOption(int choice);
//28/08/99: added by Hurdler
void M_OpenGLOption(int choice);
//void M_SetupMultiOption(int choice); // Tails 03-26-2001

menu_t MainDef,SinglePlayerDef,MultiPlayerDef,SetupMultiPlayerDef,
	   EpiDef,NewDef,OptionsDef,VidModeDef,ControlDef,SoundDef,
	   ReadDef2,ReadDef1,SaveDef,LoadDef,ControlDef2,GameOptionDef,
	   NetOptionDef,VideoOptionsDef,MouseOptionsDef;

//===========================================================================
//Generic Stuffs (more easy to create menus :))
//===========================================================================

void M_DrawGenericMenu(void)
{
	int x,y,i,cursory=0;

	// DRAW MENU
	x = currentMenu->x;
	y = currentMenu->y;

	// draw title (or big pic)
	if( currentMenu->menutitlepic )
	{
		patch_t* p = W_CachePatchName(currentMenu->menutitlepic,PU_CACHE);
		// did some fanangling around here Tails 01-11-2001
		int xtitle = (BASEVIDWIDTH-p->width)/2;
		int ytitle = (y-p->height)/2;

		if(xtitle<0) xtitle=0;
		if(ytitle<0) ytitle=0;
		V_DrawScaledPatch (xtitle,ytitle,0,p);
	}

	for (i=0;i<currentMenu->numitems;i++)
	{
		if (i==itemOn)
			cursory=y;
		switch (currentMenu->menuitems[i].status & IT_DISPLAY) {
		   case IT_PATCH  :
			   if( currentMenu->menuitems[i].name &&
				   currentMenu->menuitems[i].name[0] )
				   V_DrawScaledPatch (x,y,0,
									  W_CachePatchName(currentMenu->menuitems[i].name ,PU_CACHE));
				/* FALLTHRU */
		   case IT_DYBIGSPACE:
			   y += LINEHEIGHT;
			   break;
		   case IT_STRING :
			   V_DrawString (x,y+currentMenu->menuitems[i].alphaKey,
								 currentMenu->menuitems[i].name);

			   // Cvar specific handling
			   switch (currentMenu->menuitems[i].status & IT_TYPE)
				   case IT_CVAR:
				   {
					consvar_t *cv=(consvar_t *)currentMenu->menuitems[i].itemaction;
					switch (currentMenu->menuitems[i].status & IT_CVARTYPE) {
					   case IT_CV_SLIDER :
									M_DrawSlider (BASEVIDWIDTH-x-SLIDER_WIDTH,
															  y+currentMenu->menuitems[i].alphaKey,
															  ( (cv->value - cv->PossibleValue[0].value) * 100 /
															  (cv->PossibleValue[1].value - cv->PossibleValue[0].value)));
					   case IT_CV_NOPRINT:
							break;
					   default:
							//added:08-02-98: BIG HACK : use alphaKey member as coord y for option
							V_DrawStringWhite(BASEVIDWIDTH-x-V_StringWidth (cv->string),
											  y+currentMenu->menuitems[i].alphaKey,
											  cv->string);
							break;
				   }
				   break;
			   }
			   break;
		   case IT_STRING2:
			   V_DrawString (x,y,currentMenu->menuitems[i].name);
			   /* FALLTHRU */
		   case IT_DYLITLSPACE:
			   y+=SMALLLINEHEIGHT;
			   break;
		   case IT_WHITESTRING :
			   V_DrawStringWhite (x,currentMenu->y+currentMenu->menuitems[i].alphaKey,
									currentMenu->menuitems[i].name);
			   break;
		   case IT_GRAYPATCH:
			   if( currentMenu->menuitems[i].name &&
				   currentMenu->menuitems[i].name[0] )
				   V_DrawMappedPatch (x,y,0,
									  W_CachePatchName(currentMenu->menuitems[i].name ,PU_CACHE),
									  graymap);
			   y += LINEHEIGHT;
			   break;

		}
	}

	// DRAW THE SKULL CURSOR // New stuff added Tails 11-30-2000
	if (((currentMenu->menuitems[itemOn].status & IT_DISPLAY)==IT_PATCH)
	  ||((currentMenu->menuitems[itemOn].status & IT_DISPLAY)==IT_NOTHING) )
	{
		V_DrawScaledPatch(currentMenu->x - 32,
						  currentMenu->y + itemOn*LINEHEIGHT,
						  0,
						  W_CachePatchName("M_CURSOR", PU_CACHE) );
	}
	else
	if (skullAnimCounter!=10)  //blink cursor // Tails
	{
		if ((currentMenu->menuitems[itemOn].status & IT_DISPLAY)==IT_STRING2)
			y=cursory;
		else
			y=currentMenu->y+currentMenu->menuitems[itemOn].alphaKey;
/*        V_DrawMappedPatch(currentMenu->x - 10, // Tails
						  y,
						  0,
						  W_CachePatchName( "STCFN042" ,PU_CACHE),
						  whitemap);*/
		  V_DrawScaledPatch(currentMenu->x - 24, y, 0, W_CachePatchName("M_CURSOR", PU_CACHE));
			   V_DrawStringWhite (currentMenu->x,y,
									currentMenu->menuitems[itemOn].name);
	}

}

//===========================================================================
//MAIN MENU
//===========================================================================

void M_QuitDOOM(int choice);
// big overhaul Tails 11-30-2000
enum
{
	singleplr = 0,
	options,
	quitdoom,
	main_end
} main_e;

menuitem_t MainMenu[]=
{
	{IT_SUBMENU | IT_STRING,"1  player" ,&SinglePlayerDef,  96},
	{IT_SUBMENU | IT_STRING," options"  ,&OptionsDef     , 104},
	{IT_CALL    | IT_STRING,"quit game",M_QuitDOOM      , 112}
};

menu_t  MainDef =
{
	NULL, // Tails 12-01-2000
	4, // Tails 03-26-2001
	NULL,
	MainMenu,
	M_DrawGenericMenu,
	124,64, // Tails
	0
};

//===========================================================================
//SINGLE PLAYER MENU
//===========================================================================
// Menu Revamp! Tails 11-30-2000
void M_NewGame(int choice);
void M_LoadGame(int choice);
void M_SaveGame(int choice);
void M_EndGame(int choice);
void M_SetupMultiPlayer (int choice);

enum
{
	newgame = 0,
	loadgame,
	savegame,
	endgame,
	setupplayer1,
	single_end
} single_e;

menuitem_t SinglePlayerMenu[] =
{
	{IT_CALL | IT_STRING,"New Game",M_NewGame, 80},
	{IT_CALL | IT_STRING,"Load Game",M_LoadGame, 90},
	{IT_CALL | IT_STRING,"Save Game",M_SaveGame, 100},
	{IT_CALL | IT_STRING,"End Game",M_EndGame, 110},
//    {IT_CALL | IT_PATCH,"M_NGAME" ,M_NewGame ,'n'},
//    {IT_CALL | IT_PATCH,"M_LOADG" ,M_LoadGame,'l'},
//    {IT_CALL | IT_PATCH,"M_SAVEG" ,M_SaveGame,'s'},
//    {IT_CALL | IT_PATCH,"M_ENDGAM",M_EndGame ,'e'},
//    {IT_CALL | IT_PATCH,"M_SETUPA",M_SetupMultiPlayer ,'s'},
//    {IT_CALL | IT_STRING,"Player Setup",M_SetupMultiOption, 120}
	{IT_CALL | IT_STRING,"Player Setup",M_SetupMultiPlayer, 120}
};

menu_t  SinglePlayerDef =
{
	0, // was "M_SINGLE" Tails 01-11-2001
	single_end,
	NULL,
	SinglePlayerMenu,
	M_DrawGenericMenu,
	130,64, // Tails 11-30-2000
	0
};


//===========================================================================
//                            MULTI PLAYER MENU
//===========================================================================
//void M_SetupMultiPlayer (int choice);
void M_SetupMultiPlayerBis (int choice);
void M_Splitscreen(int choise);

// Menu overhaul Tails 11-30-2000
enum {
	startsplitscreengame=0,
//    setupplayer1,
	setupplayer2,
	multiplayeroptions,
	multiplayer_end
} multiplayer_e;

menuitem_t MultiPlayerMenu[] =
{
	{IT_CALL | IT_PATCH,"M_2PLAYR",M_Splitscreen ,'n'},
//    {IT_CALL | IT_PATCH,"M_SETUPA",M_SetupMultiPlayer ,'s'},
	{IT_CALL | IT_PATCH,"M_SETUPB",M_SetupMultiPlayerBis ,'t'},
	{IT_CALL | IT_PATCH,"M_OPTTTL",M_NetOption ,'o'}
};

menu_t  MultiPlayerDef =
{
	"M_MULTI",
	multiplayer_end, //sizeof(MultiPlayerMenu)/sizeof(menuitem_t),
	NULL,
	MultiPlayerMenu,
	M_DrawGenericMenu,
	65,64,
	0
};

static boolean StartSplitScreenGame;

void M_Splitscreen(int choise)
{
	if (netgame)
	{
		M_StartMessage(NEWGAME,NULL,false);
		return;
	}

	if ( gamemode == commercial )
		M_SetupNextMenu(&NewDef);
	else
		M_SetupNextMenu(&EpiDef);

	StartSplitScreenGame=true;
}

//===========================================================================
// Seconde mouse config for the splitscreen player
//===========================================================================

menuitem_t  SecondMouseCfgMenu[] =
{
	{IT_STRING | IT_CVAR,"Seconde Mouse Serial Port",&cv_mouse2port,0},
	{IT_STRING | IT_CVAR,"Use Mouse 2"     ,&cv_usemouse2        ,10},
	{IT_STRING | IT_CVAR
	 | IT_CV_SLIDER     ,"Mouse2 Speed"    ,&cv_mousesens2       ,20},
	{IT_STRING | IT_CVAR,"Always MouseLook",&cv_alwaysfreelook2  ,30},
	{IT_STRING | IT_CVAR,"Mouse Move",      &cv_mousemove2       ,40},
	{IT_STRING | IT_CVAR,"Invert Mouse2"   ,&cv_invertmouse2     ,50},
	{IT_STRING | IT_CVAR
	 | IT_CV_SLIDER     ,"Mlook Speed"     ,&cv_mlooksens2       ,60},
};

menu_t  SecondMouseCfgdef =
{
	"M_OPTTTL",
	sizeof(SecondMouseCfgMenu)/sizeof(menuitem_t),
	&SetupMultiPlayerDef,
	SecondMouseCfgMenu,
	M_DrawGenericMenu,
	27,40,
	0,
};

//===========================================================================
//MULTI PLAYER SETUP MENU
//===========================================================================
void M_DrawSetupMultiPlayerMenu(void);
void M_HandleSetupMultiPlayer(int choice);
void M_SetupControlsMenu(int choice);
boolean M_QuitMutliPlayerMenu(void);

menuitem_t SetupMultiPlayerMenu[] =
{
	{IT_KEYHANDLER | IT_STRING          ,"Your name" ,M_HandleSetupMultiPlayer,0},
	{IT_CVAR | IT_STRING | IT_CV_NOPRINT,"Your color",&cv_playercolor         ,16},
	{IT_KEYHANDLER | IT_STRING          ,"Your player" ,M_HandleSetupMultiPlayer,96}, // changed to player Tails 11-09-99
	/* this line calls the setup controls for secondary player, only if numitems is > 3 */
	{IT_CALL | IT_WHITESTRING, "Setup Controls...", M_SetupControlsMenu, 120},
	{IT_SUBMENU | IT_WHITESTRING, "Second Mouse config...", &SecondMouseCfgdef, 130}
};

enum {
	setupmultiplayer_name = 0,
	setupmultiplayer_color,
	setupmultiplayer_skin,
	setupmultiplayer_controls,
	setupmultiplayer_mouse2,
	setupmulti_end
};

menu_t  SetupMultiPlayerDef =
{
	"M_MULTI",
	sizeof(SetupMultiPlayerMenu)/sizeof(menuitem_t),
	&SinglePlayerDef,
	SetupMultiPlayerMenu,
	M_DrawSetupMultiPlayerMenu,
	27,40,
	0,
	M_QuitMutliPlayerMenu
};


#define PLBOXW    8
#define PLBOXH    9

static  int       multi_tics;
static  state_t*  multi_state;

// this is set before entering the MultiPlayer setup menu,
// for either player 1 or 2
static  char       setupm_name[MAXPLAYERNAME+1];
static  player_t*  setupm_player;
static  consvar_t* setupm_cvskin;
static  consvar_t* setupm_cvcolor;
static  consvar_t* setupm_cvname;

void M_SetupMultiPlayer (int choice)
{
//	if(!cv_gametype.value) // Don't change if in single player Tails 03-25-2001
//		return;
	multi_state = &states[mobjinfo[MT_PLAYER].seestate];
	multi_tics = multi_state->tics;
	strcpy(setupm_name, cv_playername.string);

	SetupMultiPlayerDef.numitems = setupmulti_end - 2;      //remove player2 setup controls and mouse2

	// set for player 1
	SetupMultiPlayerMenu[setupmultiplayer_color].itemaction = &cv_playercolor;
	setupm_player = &players[consoleplayer];
	setupm_cvskin = &cv_skin;
	setupm_cvcolor = &cv_playercolor;
	setupm_cvname = &cv_playername;
	M_SetupNextMenu (&SetupMultiPlayerDef);
}

// start the multiplayer setup menu, for secondary player (splitscreen mode)
void M_SetupMultiPlayerBis (int choice)
{
	multi_state = &states[mobjinfo[MT_PLAYER].seestate];
	multi_tics = multi_state->tics;
	strcpy (setupm_name, cv_playername2.string);
	SetupMultiPlayerDef.numitems = setupmulti_end;          //activate the setup controls for player 2

	// set for splitscreen secondary player
	SetupMultiPlayerMenu[setupmultiplayer_color].itemaction = &cv_playercolor2;
	setupm_player = &players[secondarydisplayplayer];
	setupm_cvskin = &cv_skin2;
	setupm_cvcolor = &cv_playercolor2;
	setupm_cvname = &cv_playername2;
	M_SetupNextMenu (&SetupMultiPlayerDef);
}
// called at splitscreen changes
void M_SwitchSplitscreen(void)
{
// activate setup for player 2
	if (cv_splitscreen.value)
		MultiPlayerMenu[setupplayer2].status = IT_CALL | IT_PATCH;
	else
		MultiPlayerMenu[setupplayer2].status = IT_DISABLED;

	if( MultiPlayerDef.lastOn==setupplayer2)
		MultiPlayerDef.lastOn=setupplayer1;
}



//
//  Draw the multi player setup menu, had some fun with player anim
//
void M_DrawSetupMultiPlayerMenu(void)
{
	int             mx,my;
	spritedef_t*    sprdef;
	spriteframe_t*  sprframe;
	int             lump;
	patch_t*        patch;
	int             st;
	UINT8*           colormap;

	mx = SetupMultiPlayerDef.x;
	my = SetupMultiPlayerDef.y;

	// use generic drawer for cursor, items and title
	M_DrawGenericMenu();

	// draw name string
	M_DrawTextBox(mx+90,my-8,MAXPLAYERNAME,1);
	V_DrawString (mx+98,my,setupm_name);

	// draw skin string
	V_DrawString (mx+90, my+96, setupm_cvskin->string);

	// draw text cursor for name
	if (itemOn==0 &&
		skullAnimCounter<4)   //blink cursor
		V_DrawScaledPatch (mx+98+V_StringWidth(setupm_name),my,0,
						   W_CachePatchName("STCFN095",PU_CACHE)); // Tails 11-30-2000

	// anim the player in the box
	if (--multi_tics<=0)
	{
		st = multi_state->nextstate;
		if (st!=S_NULL)
			multi_state = &states[st];
		multi_tics = multi_state->tics;
		if (multi_tics==-1)
			multi_tics=15;
	}

	// skin 0 is default player sprite
	sprdef    = &skins[R_SkinAvailable(setupm_cvskin->zstring)].spritedef;
	sprframe  = &sprdef->spriteframes[ multi_state->frame & FF_FRAMEMASK];
	lump  = sprframe->lumppat[0];
	patch = W_CachePatchNum (lump, PU_CACHE);

	// draw box around guy
	M_DrawTextBox(mx+90,my+8, PLBOXW, PLBOXH);

	if (setupm_cvcolor->value==0)
		colormap = colormaps;
	else
		colormap = (UINT8 *) translationtables - 256 + (setupm_cvcolor->value<<8);
	// draw player sprite
	V_DrawMappedPatch (mx+98+(PLBOXW*8/2),my+16+(PLBOXH*8)-8,0,patch,colormap);
}


//
// Handle Setup MultiPlayer Menu
//
void M_HandleSetupMultiPlayer (int choice)
{
	int      l;
	boolean  exitmenu = false;  // exit to previous menu and send name change
	int      myskin;

	myskin  = setupm_cvskin->value;

	switch( choice )
	{
	  case KEY_DOWNARROW:
		S_StartSound(NULL,sfx_menu1); // Tails
		if (itemOn+1 >= SetupMultiPlayerDef.numitems)
			itemOn = 0;
		else itemOn++;
		break;

	  case KEY_UPARROW:
		S_StartSound(NULL,sfx_menu1); // Tails
		if (!itemOn)
			itemOn = SetupMultiPlayerDef.numitems-1;
		else itemOn--;
		break;

	  case KEY_LEFTARROW:
		if (itemOn==2)       //player skin
		{
			S_StartSound(NULL,sfx_menu1); // Tails
			myskin--;
		}
		break;

	  case KEY_RIGHTARROW:
		if (itemOn==2)       //player skin
		{
			S_StartSound(NULL,sfx_menu1); // Tails
			myskin++;
		}
		break;

	  case KEY_ENTER:
		S_StartSound(NULL,sfx_menu1); // Tails
		exitmenu = true;
		break;

	  case KEY_ESCAPE:
// Tails        S_StartSound(NULL,sfx_swtchx);
		exitmenu = true;
		break;

	  case KEY_BACKSPACE:
		if ( (l=strlen(setupm_name))!=0 && itemOn==0)
		{
			S_StartSound(NULL,sfx_stnmov);
			setupm_name[l-1]=0;
		}
		break;

	  default:
		if (choice < 32 || choice > 127 || itemOn!=0)
			break;
		l = strlen(setupm_name);
		if (l<MAXPLAYERNAME-1)
		{
			S_StartSound(NULL,sfx_stnmov);
			setupm_name[l]=choice;
			setupm_name[l+1]=0;
		}
		break;
	}

	// check skin
	if (myskin <0)
		myskin = numskins-1;
	if (myskin >numskins-1)
		myskin = 0;

	// check skin change
	if (myskin != setupm_player->skin)
		COM_BufAddText ( va("%s \"%s\"",setupm_cvskin->name ,skins[myskin].name));

	if (exitmenu)
	{
		if (currentMenu->prevMenu)
			M_SetupNextMenu (currentMenu->prevMenu);
		else
			M_ClearMenus ();
	}
}

boolean M_QuitMutliPlayerMenu(void)
{
	int      l;
		// send name if changed
		if (strcmp(setupm_name, setupm_cvname->string))
		{
			// remove trailing whitespaces
			for (l= strlen(setupm_name)-1;
				 l>=0 && setupm_name[l]==' '; l--)
				   setupm_name[l]=0;
			COM_BufAddText ( va("%s \"%s\"",setupm_cvname->name ,setupm_name));

		}
	return true;
}


//===========================================================================
//                              EPISODE SELECT
//===========================================================================

void M_Episode(int choice);

enum
{
	ep1,
	ep2,
	ep3,
	ep4,
	ep_end
} episodes_e;

menuitem_t EpisodeMenu[]=
{
	{IT_CALL | IT_PATCH,"M_EPI1", M_Episode,'k'},
	{IT_CALL | IT_PATCH,"M_EPI2", M_Episode,'t'},
	{IT_CALL | IT_PATCH,"M_EPI3", M_Episode,'i'},
	{IT_CALL | IT_PATCH,"M_EPI4", M_Episode,'t'}
};

menu_t  EpiDef =
{
	"M_EPISOD",
	ep_end,             // # of menu items
	&MainDef,           // previous menu
	EpisodeMenu,        // menuitem_t ->
	M_DrawGenericMenu,  // drawing routine ->
	48,63,              // x,y
	ep1                 // lastOn, flags
};

//
//      M_Episode
//
int     epi;

void M_Episode(int choice)
{
	if ( (gamemode == shareware)
		 && choice)
	{
		M_SetupNextMenu(&ReadDef1);
		M_StartMessage(SWSTRING,NULL,false);
		return;
	}

	// Yet another hack...
	if ( (gamemode == registered)
		 && (choice > 2))
	{
		I_Error("M_Episode: 4th episode requires UltimateDOOM\n");
		choice = 0;
	}

	epi = choice;
	M_SetupNextMenu(&NewDef);
}


//===========================================================================
//                           NEW GAME FOR SINGLE PLAYER
//===========================================================================
void M_DrawNewGame(void);
// overhaul! Tails 11-30-2000
void M_ChooseSkill(int choice);

enum
{
	killthings,
	toorough,
	hurtme,
	violence,
	nightmare,
	newg_end
} newgame_e;

menuitem_t NewGameMenu[]=
{
	{IT_CALL | IT_STRING,"Cakewalk",M_ChooseSkill, 80},
	{IT_CALL | IT_STRING,"Easy",M_ChooseSkill, 90},
	{IT_CALL | IT_STRING,"Normal",M_ChooseSkill, 100},
	{IT_CALL | IT_STRING,"Hard",M_ChooseSkill, 110},
	{IT_CALL | IT_STRING,"Very Hard",M_ChooseSkill, 120}
/*    {IT_CALL | IT_PATCH,"M_JKILL",M_ChooseSkill, 'i'},
	{IT_CALL | IT_PATCH,"M_ROUGH",M_ChooseSkill, 'h'},
	{IT_CALL | IT_PATCH,"M_HURT" ,M_ChooseSkill, 'h'},
	{IT_CALL | IT_PATCH,"M_ULTRA",M_ChooseSkill, 'u'},
	{IT_CALL | IT_PATCH,"M_NMARE",M_ChooseSkill, 'n'}*/
};

menu_t  NewDef =
{
	"M_NEWG",
	newg_end,           // # of menu items
	&EpiDef,            // previous menu
	NewGameMenu,        // menuitem_t ->
	M_DrawNewGame,      // drawing routine ->
	48,63,              // x,y
	violence            // lastOn
};

void M_DrawNewGame(void)
{
//    patch_t* p;

	//faB: testing with glide
//    p = W_CachePatchName("M_SKILL",PU_CACHE);
//    V_DrawScaledPatch ((BASEVIDWIDTH-p->width)/2,38,0,p);
	V_DrawString(96,128, "Choose Skill Level");

	//    V_DrawScaledPatch (54,38,0,W_CachePatchName("M_SKILL",PU_CACHE));
	M_DrawGenericMenu();
}

void M_NewGame(int choice)
{
	if (netgame)
	{
		M_StartMessage(NEWGAME,NULL,false);
		return;
	}

	if ( gamemode == commercial )
		M_SetupNextMenu(&NewDef);
	else
		M_SetupNextMenu(&EpiDef);

	StartSplitScreenGame=false;
}

void M_VerifyNightmare(int ch);

void M_ChooseSkill(int choice)
{
	if (choice == nightmare)
	{
		M_StartMessage(NIGHTMARE,M_VerifyNightmare,true);
		return;
	}

	G_DeferedInitNew(choice, G_BuildMapName(epi+1,1),StartSplitScreenGame);
	M_ClearMenus ();
}

void M_VerifyNightmare(int ch)
{
	if (ch != 'y')
		return;

	G_DeferedInitNew (nightmare, G_BuildMapName(epi+1,1),StartSplitScreenGame);
	M_ClearMenus ();
}

//===========================================================================
//                             OPTIONS MENU
//===========================================================================
//
// M_Options
//

//added:10-02-98: note: alphaKey member is the y offset
menuitem_t OptionsMenu[]=
{
//    {IT_STRING | IT_CVAR,"Messages:"       ,&cv_showmessages    ,0},
//    {IT_STRING | IT_CVAR,"Always Run"      ,&cv_autorun         ,10},
//    {IT_STRING | IT_CVAR,"Crosshair"       ,&cv_crosshair       ,20},
//    {IT_STRING | IT_CVAR,"Autoaim"         ,&cv_autoaim         ,30},
	{IT_STRING | IT_CVAR,"Control per key" ,&cv_controlperkey   ,40}, // Changed all to normal string Tails 11-30-2000
	{IT_STRING | IT_CVAR,"Analog Control (BETA)" ,&cv_analog   ,60}, // Changed all to normal string Tails 11-30-2000
	{IT_STRING | IT_CVAR,"Amount of Snow" ,&cv_numsnow   ,70}, // Changed all to normal string Tails 11-30-2000
	{IT_CALL   | IT_STRING,"Game Options..."  ,M_GameOption,80},
	{IT_SUBMENU | IT_STRING,"Sound Volume..."  ,&SoundDef  ,90},
	{IT_SUBMENU | IT_STRING,"Video Options..." ,&VideoOptionsDef,100},
	{IT_SUBMENU | IT_STRING,"Mouse Options..." ,&MouseOptionsDef,110},
	{IT_CALL    | IT_STRING,"Setup Controls...",M_SetupControlsMenu,120}
};

menu_t  OptionsDef =
{
	"M_OPTTTL",
	sizeof(OptionsMenu)/sizeof(menuitem_t),
	&MainDef,
	OptionsMenu,
	M_DrawGenericMenu,
	60,40,
	0
};

//
//  A smaller 'Thermo', with range given as percents (0-100)
//
void M_DrawSlider (int x, int y, int range)
{
	int i;

	if (range < 0)
		range = 0;
	if (range > 100)
		range = 100;

	V_DrawScaledPatch (x-8, y, 0, W_CachePatchName( "M_SLIDEL" ,PU_CACHE) );

	for (i=0 ; i<SLIDER_RANGE ; i++)
		V_DrawScaledPatch (x+i*8, y, 0,
						   W_CachePatchName( "M_SLIDEM" ,PU_CACHE) );

	V_DrawScaledPatch (x+SLIDER_RANGE*8, y, 0,
					   W_CachePatchName( "M_SLIDER" ,PU_CACHE) );

	// draw the slider cursor
	V_DrawMappedPatch (x + ((SLIDER_RANGE-1)*8*range)/100, y, 0,
					   W_CachePatchName( "M_SLIDEC" ,PU_CACHE),
					   whitemap);
}

//===========================================================================
//                        Video OPTIONS MENU
//===========================================================================

//added:10-02-98: note: alphaKey member is the y offset
menuitem_t VideoOptionsMenu[]=
{
	{IT_STRING | IT_SUBMENU, "Video Modes..."   , &VidModeDef       , 0},
	{IT_STRING | IT_CVAR,    "Fullscreen"       , &cv_fullscreen    , 10},

	{IT_STRING | IT_CVAR
		| IT_CV_SLIDER,      "Brightness"       , &cv_usegamma      ,20},
#ifdef HERITAGE_VIEWSIZE_FIX
	{IT_STRING | IT_CVAR
		| IT_CV_SLIDER,      "Screen Size"      , &cv_viewsize      ,30},
#endif
#if 0
	{IT_STRING | IT_CVAR,    "Translucency"     , &cv_translucency  ,40},
	{IT_STRING | IT_CVAR,    "Splats"           , &cv_splats        ,50}, // Tails
	{IT_STRING | IT_CVAR,    "Screens Link"     , &cv_screenslink   ,60},
#endif
/*#ifdef HWRENDER
	//17/10/99: added by Hurdler
	{IT_CALL|IT_WHITESTRING, "OpenGL Options...", M_OpenGLOption    ,90},
#endif*/
};

menu_t  VideoOptionsDef =
{
	"M_OPTTTL",
	sizeof(VideoOptionsMenu)/sizeof(menuitem_t),
	&OptionsDef,
	VideoOptionsMenu,
	M_DrawGenericMenu,
	60,40,
	0
};

//===========================================================================
//                        Mouse OPTIONS MENU
//===========================================================================

//added:24-03-00: note: alphaKey member is the y offset
menuitem_t MouseOptionsMenu[]=
{
	{IT_STRING | IT_CVAR,"Use Mouse",        &cv_usemouse        ,0},
	{IT_STRING | IT_CVAR,"Always MouseLook", &cv_alwaysfreelook  ,10},
	{IT_STRING | IT_CVAR,"Mouse Move"      , &cv_mousemove       ,20},
	{IT_STRING | IT_CVAR,"Invert Mouse"    , &cv_invertmouse     ,30},
	{IT_STRING | IT_CVAR
	 | IT_CV_SLIDER     ,"Mouse Speed"     , &cv_mousesens       ,40},
	{IT_STRING | IT_CVAR
	 | IT_CV_SLIDER     ,"Mlook Speed"     , &cv_mlooksens       ,50}

};

menu_t  MouseOptionsDef =
{
	"M_OPTTTL",
	sizeof(MouseOptionsMenu)/sizeof(menuitem_t),
	&OptionsDef,
	MouseOptionsMenu,
	M_DrawGenericMenu,
	60,40,
	0
};

//===========================================================================
//                        Game OPTIONS MENU
//===========================================================================

menuitem_t GameOptionsMenu[]=
{
	{IT_STRING | IT_CVAR,"Enemy Respawn"     ,&cv_respawnmonsters    ,20}, // Tails
	{IT_STRING | IT_CVAR,"Enemy Respawn time",&cv_respawnmonsterstime,30}, // Tails
	{IT_STRING | IT_CVAR,"Fast Enemies"       ,&cv_fastmonsters       ,40}, // Tails,
	{IT_CALL   | IT_STRING,"Network Options..."  ,M_NetOption     ,110} // Tails
};

menu_t  GameOptionDef =
{
	"M_OPTTTL",
	sizeof(GameOptionsMenu)/sizeof(menuitem_t),
	&OptionsDef,
	GameOptionsMenu,
	M_DrawGenericMenu,
	60,40,
	0
};

void M_GameOption(int choice)
{
	if(!server)
	{
		M_StartMessage("You are not the server\nYou can't change the options\n",NULL,false);
		return;
	}
	M_SetupNextMenu(&GameOptionDef);
}

//===========================================================================
//                        Network OPTIONS MENU
//===========================================================================

menuitem_t NetOptionsMenu[]=
{
	{IT_STRING | IT_CVAR,   "Allow exitlevel"  , &cv_allowexitlevel,40},
	{IT_STRING | IT_CVAR,   "Allow join player", &cv_allownewplayer,50},

	{IT_STRING | IT_CVAR,   "Timelimit"        , &cv_timelimit     ,90},
	{IT_STRING | IT_CVAR,   "Game Type"        , &cv_gametype      ,100}, // Tails
	{IT_STRING | IT_CVAR,   "Maxplayers"       , &cv_maxplayers    ,110},
	{IT_CALL   | IT_STRING, "Game Options..."  , M_GameOption      ,130}, // Tails
};

menu_t  NetOptionDef =
{
	"M_OPTTTL",
	sizeof(NetOptionsMenu)/sizeof(menuitem_t),
	&MultiPlayerDef,
	NetOptionsMenu,
	M_DrawGenericMenu,
	60,40,
	0
};

void M_NetOption(int choice)
{
	if(!server)
	{
		M_StartMessage("You are not the server\nYou can't change the options\n",NULL,false);
		return;
	}
	M_SetupNextMenu(&NetOptionDef);
}

//===========================================================================
//                          Read This! MENU 1
//===========================================================================

void M_DrawReadThis1(void);
void M_DrawReadThis2(void);

enum
{
	rdthsempty1,
	read1_end
} read_e;

menuitem_t ReadMenu1[] =
{
	{IT_SUBMENU | IT_NOTHING,"",&ReadDef2,0}
};

menu_t  ReadDef1 =
{
	NULL,
	read1_end,
	&MainDef,
	ReadMenu1,
	M_DrawReadThis1,
	280,185,
	0
};

//
// Read This Menus
// Had a "quick hack to fix romero bug"
//
void M_DrawReadThis1(void)
{
	V_DrawScaledPatch (0,0,0,W_CachePatchName("HELP",PU_CACHE));
}

//===========================================================================
//                          Read This! MENU 2
//===========================================================================

enum
{
	rdthsempty2,
	read2_end
} read_e2;

menuitem_t ReadMenu2[]=
{
	{IT_SUBMENU | IT_NOTHING,"",&MainDef,0}
};

menu_t  ReadDef2 =
{
	NULL,
	read2_end,
	&ReadDef1,
	ReadMenu2,
	M_DrawReadThis2,
	330,175,
	0
};


//
// Read This Menus - optional second page.
//
void M_DrawReadThis2(void)
{
	// This hack keeps us from having to change menus.
	V_DrawScaledPatch (0,0,0,W_CachePatchName("CREDIT",PU_CACHE));
}

//===========================================================================
//                        SOUND VOLUME MENU
//===========================================================================
void M_DrawSound(void);

void M_SfxVol(int choice);
void M_MusicVol(int choice);
void M_CDAudioVol (int choice);

enum
{
	sfx_vol,
	sfx_empty1,
	music_vol,
	sfx_empty2,
	sound_end
} sound_e;

menuitem_t SoundMenu[]=
{
	{IT_CVARMAX | IT_PATCH,"M_SFXVOL",&cv_soundvolume  ,'s'},
	{IT_BIGSPACE          ,NULL      ,NULL             ,0},
	{IT_CVARMAX | IT_PATCH,"M_MUSVOL",&cv_musicvolume  ,'m'},
	{IT_BIGSPACE          ,NULL      ,NULL             ,0}
};

menu_t  SoundDef =
{
	"M_SVOL",
	sound_end,
	&OptionsDef,
	SoundMenu,
	M_DrawSound,
	80,50,
	0
};


//
// Change Sfx & Music volumes
//
void M_DrawSound(void)
{
	M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(sfx_vol+1),
				 16,cv_soundvolume.value);

	M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(music_vol+1),
				 16,cv_musicvolume.value);

	M_DrawGenericMenu();
}

//===========================================================================
//                          CONTROLS MENU
//===========================================================================
void M_DrawControl(void);               // added 3-1-98
void M_ChangeControl(int choice);

//
// this is the same for all control pages
//
menuitem_t ControlMenu[]=
{
	{IT_CALL | IT_STRING2,"Ring Throw"  ,M_ChangeControl,gc_fire       }, // Tails 11-09-99
	{IT_CALL | IT_STRING2,"Rolling"     ,M_ChangeControl,gc_use        }, // Tails 12-04-99
	{IT_CALL | IT_STRING2,"Jump"        ,M_ChangeControl,gc_jump       },
	{IT_CALL | IT_STRING2,"Forward"     ,M_ChangeControl,gc_forward    },
	{IT_CALL | IT_STRING2,"Backpedal"   ,M_ChangeControl,gc_backward   },
	{IT_CALL | IT_STRING2,"Turn Left"   ,M_ChangeControl,gc_turnleft   },
	{IT_CALL | IT_STRING2,"Turn Right"  ,M_ChangeControl,gc_turnright  },
	{IT_CALL | IT_STRING2,"Run"         ,M_ChangeControl,gc_speed      },
	{IT_CALL | IT_STRING2,"Strafe On"   ,M_ChangeControl,gc_strafe     },
	{IT_CALL | IT_STRING2,"Strafe Left" ,M_ChangeControl,gc_strafeleft },
	{IT_CALL | IT_STRING2,"Strafe Right",M_ChangeControl,gc_straferight},
	{IT_CALL | IT_STRING2,"Look Up"     ,M_ChangeControl,gc_lookup     },
	{IT_CALL | IT_STRING2,"Look Down"   ,M_ChangeControl,gc_lookdown   },
	{IT_CALL | IT_STRING2,"Center View" ,M_ChangeControl,gc_centerview },
	{IT_CALL | IT_STRING2,"Mouselook"   ,M_ChangeControl,gc_mouseaiming},

	{IT_SUBMENU | IT_WHITESTRING,"next" ,&ControlDef2,128} // Tails
};

menu_t  ControlDef =
{
	"M_CONTRO",
	sizeof(ControlMenu)/sizeof(menuitem_t),
	&OptionsDef,
	ControlMenu,
	M_DrawControl,
	50,40,
	0
};

//
//  Controls page 1
//
menuitem_t ControlMenu2[]=
{
	{IT_CALL | IT_STRING2,"Talk key"       ,M_ChangeControl,gc_talkkey},
	{IT_CALL | IT_STRING2,"Rankings/Scores",M_ChangeControl,gc_scores},
	{IT_CALL | IT_STRING2,"Console"        ,M_ChangeControl,gc_console},

	{IT_SUBMENU | IT_WHITESTRING,"next"    ,&ControlDef,128}
};

menu_t  ControlDef2 =
{
	"M_CONTRO",
	sizeof(ControlMenu2)/sizeof(menuitem_t),
	&OptionsDef,
	ControlMenu2,
	M_DrawControl,
	50,40,
	0
};


//
// Start the controls menu, setting it up for either the console player,
// or the secondary splitscreen player
//
static  boolean setupcontrols_secondaryplayer;
static  int   (*setupcontrols)[2];  // pointer to the gamecontrols of the player being edited
void M_SetupControlsMenu (int choice)
{
	// set the gamecontrols to be edited
	if (choice == setupmultiplayer_controls) {
		setupcontrols_secondaryplayer = true;
		setupcontrols = gamecontrolbis;     // was called from secondary player's multiplayer setup menu
	}
	else {
		setupcontrols_secondaryplayer = false;
		setupcontrols = gamecontrol;        // was called from main Options (for console player, then)
	}
	currentMenu->lastOn = itemOn;
	M_SetupNextMenu(&ControlDef);
}


//
//  Draws the Customise Controls menu
//
void M_DrawControl(void)
{
	char     tmp[50];
	int      i;
	int      keys[2];

	// draw title, strings and submenu
	M_DrawGenericMenu();

	M_CentreText (ControlDef.y-12,
		(setupcontrols_secondaryplayer ? "SET CONTROLS FOR SECONDARY PLAYER" :
										 "PRESS ENTER TO CHANGE, BACKSPACE TO CLEAR") );

	for(i=0;i<currentMenu->numitems;i++)
	{
		if (currentMenu->menuitems[i].status!=IT_CONTROL)
			continue;

		keys[0] = setupcontrols[currentMenu->menuitems[i].alphaKey][0];
		keys[1] = setupcontrols[currentMenu->menuitems[i].alphaKey][1];

		tmp[0]='\0';
		if (keys[0] == KEY_NULL && keys[1] == KEY_NULL)
		{
			strcpy(tmp, "---");
		}
		else
		{
			if( keys[0] != KEY_NULL )
				strcat (tmp, G_KeynumToString (keys[0]));

			if( keys[0] != KEY_NULL && keys[1] != KEY_NULL )
				strcat(tmp," or ");

			if( keys[1] != KEY_NULL )
				strcat (tmp, G_KeynumToString (keys[1]));


		}
		V_DrawStringWhite(ControlDef.x+220-V_StringWidth(tmp), ControlDef.y + i*8, tmp);
	}

}

static int controltochange;

void M_ChangecontrolResponse(event_t* ev)
{
	int        control;
	int        found;
	int        ch=ev->data1;

	// ESCAPE cancels
	if (ch!=KEY_ESCAPE && ch!=KEY_PAUSE)
	{

		switch (ev->type)
		{
		  // ignore mouse/joy movements, just get buttons
		  case ev_mouse:
			   ch = KEY_NULL;      // no key
			break;
		  case ev_joystick:
			   ch = KEY_NULL;      // no key
			break;

		  // keypad arrows are converted for the menu in cursor arrows
		  // so use the event instead of ch
		  case ev_keydown:
			ch = ev->data1;
			break;

		  default:
			break;
		}

		control = controltochange;

		// check if we already entered this key
		found = -1;
		if (setupcontrols[control][0]==ch)
			found = 0;
		else
		if (setupcontrols[control][1]==ch)
			found = 1;
		if (found>=0)
		{
			// replace mouse and joy clicks by double clicks
			if (ch>=KEY_MOUSE1 && ch<=KEY_MOUSE1+MOUSEBUTTONS)
				setupcontrols[control][found] = ch-KEY_MOUSE1+KEY_DBLMOUSE1;
			else
			  if (ch>=KEY_JOY1 && ch<=KEY_JOY1+JOYBUTTONS)
				setupcontrols[control][found] = ch-KEY_JOY1+KEY_DBLJOY1;
		}
		else
		{
			// check if change key1 or key2, or replace the two by the new
			found = 0;
			if (setupcontrols[control][0] == KEY_NULL)
				found++;
			if (setupcontrols[control][1] == KEY_NULL)
				found++;
			if (found==2)
			{
				found = 0;
				setupcontrols[control][1] = KEY_NULL;  //replace key 1 ,clear key2
			}
			G_CheckDoubleUsage(ch);
			setupcontrols[control][found] = ch;
		}

	}

	M_StopMessage(0);
}

void M_ChangeControl(int choice)
{
	static char tmp[55];

	controltochange = currentMenu->menuitems[choice].alphaKey;
	sprintf (tmp,"Hit the new key for\n%s\nESC for Cancel",currentMenu->menuitems[choice].name);

	M_StartMessage (tmp,M_ChangecontrolResponse,2);
}

//===========================================================================
//                        VIDEO MODE MENU
//===========================================================================
void M_DrawVideoMode(void);             //added:30-01-98:

void M_HandleVideoMode (int ch);

menuitem_t VideoModeMenu[]=
{
	{IT_KEYHANDLER | IT_NOTHING, "", M_HandleVideoMode, '\0'},     // dummy menuitem for the control func
};


menu_t  VidModeDef =
{
	NULL,
	1,                  // # of menu items
	//sizeof(VideoModeMenu)/sizeof(menuitem_t),
	&OptionsDef,        // previous menu
	VideoModeMenu,      // menuitem_t ->
	M_DrawVideoMode,    // drawing routine ->
	48,60,              // x,y
	0                   // lastOn
};

//added:30-01-98:
#define MAXCOLUMNMODES   10     //max modes displayed in one column
#define MAXMODEDESCS     (MAXCOLUMNMODES*3)

// shhh... what am I doing... nooooo!
static int vidm_testingmode=0;
static int vidm_previousmode;
static int vidm_current=0;
static int vidm_nummodes;
static int vidm_column_size;

typedef struct
{
	int         modenum;    //video mode number in the vidmodes list
	const char  *desc;      //XXXxYYY
	int         iscur;      //1 if it is the current active mode
} modedesc_t;

static modedesc_t   modedescs[MAXMODEDESCS];


//
// Draw the video modes list, ...-la-Quake
//
void M_DrawVideoMode(void)
{
	int           i,j,dup,row,col,nummodes;
	const char    *desc;
	char          temp[80];
	patch_t*      p;

	p = (patch_t*) W_CachePatchName("M_VIDEO",PU_CACHE);
	V_DrawScaledPatch ((BASEVIDWIDTH-p->width)/2,2,0,p);

	VID_PrepareModeList(); // FIXME: hack
	vidm_nummodes = 0;
	nummodes = VID_NumModes ();

	for (i=0 ; i<nummodes && vidm_nummodes<MAXMODEDESCS ; i++)
	{
		desc = VID_GetModeName (i);
		if (desc)
		{
			dup = 0;

			//when a resolution exists both under VGA and VESA, keep the
			// VESA mode, which is always a higher modenum
			for (j=0 ; j<vidm_nummodes ; j++)
			{
				if (!strcmp (modedescs[j].desc, desc))
				{
					//mode(0): 320x200 is always standard VGA, not vesa
					if (modedescs[j].modenum != 0)
					{
						modedescs[j].modenum = i;
						dup = 1;

						if (i == vid.modenum)
							modedescs[j].iscur = 1;
					}
					else
					{
						dup = 1;
					}

					break;
				}
			}

			if (!dup)
			{
				modedescs[vidm_nummodes].modenum = i;
				modedescs[vidm_nummodes].desc = desc;
				modedescs[vidm_nummodes].iscur = 0;

				if (i == vid.modenum)
					modedescs[vidm_nummodes].iscur = 1;

				vidm_nummodes++;
			}
		}
	}

	vidm_column_size = (vidm_nummodes+2) / 3;


	row = 41; // was 16 Tails 11-30-2000
	col = 36;
	for(i=0; i<vidm_nummodes; i++)
	{
		if (modedescs[i].iscur)
			V_DrawStringWhite (row, col, modedescs[i].desc);
		else
			V_DrawString (row, col, modedescs[i].desc);

		col += 8;
		if((i % vidm_column_size) == (vidm_column_size-1))
		{
			row += 7*13; // was 8 Tails 11-30-2000
			col = 36;
		}
	}

	if (vidm_testingmode>0)
	{
		sprintf(temp, "TESTING MODE %s", modedescs[vidm_current].desc );
		M_CentreText(VidModeDef.y+80, temp );
		M_CentreText(VidModeDef.y+90, "Please wait 5 seconds..." );
	}
	else
	{
		M_CentreText(VidModeDef.y+60,"Press ENTER to set mode");

		M_CentreText(VidModeDef.y+70,"T to test mode for 5 seconds");

		sprintf(temp, "D to make %s the default", VID_GetModeName(vid.modenum));
		M_CentreText(VidModeDef.y+80,temp);

		sprintf(temp, "Current default is %dx%d (%d bits)", cv_scr_width.value, cv_scr_height.value, cv_scr_depth.value);
		M_CentreText(VidModeDef.y+90,temp);

		M_CentreText(VidModeDef.y+100,"Press ESC to exit");
	}

// Draw the cursor for the VidMode menu
	if (skullAnimCounter!=10)    //use the Skull anim counter to blink the cursor // Tails
	{
		i = 41 - 10 + ((vidm_current / vidm_column_size)*7*13); // first integer was 16, second to last was 8 Tails 11-30-2000
		j = 36 + ((vidm_current % vidm_column_size)*8);
		V_DrawScaledPatch(i-14, j, 0, W_CachePatchName( "M_CURSOR" ,PU_CACHE)); // Tails
	}
}


//added:30-01-98: special menuitem key handler for video mode list
void M_HandleVideoMode (int ch)
{
	if (vidm_testingmode>0)
	{
	   // change back to the previous mode quickly
	   if (ch==KEY_ESCAPE)
	   {
		   setmodeneeded = vidm_previousmode+1;
		   vidm_testingmode = 0;
	   }
	   return;
	}

	switch( ch )
	{
	  case KEY_DOWNARROW:
		S_StartSound(NULL,sfx_menu1); // Tails
		vidm_current++;
		if (vidm_current>=vidm_nummodes)
			vidm_current = 0;
		break;

	  case KEY_UPARROW:
		S_StartSound(NULL,sfx_menu1); // Tails
		vidm_current--;
		if (vidm_current<0)
			vidm_current = vidm_nummodes-1;
		break;

	  case KEY_LEFTARROW:
		S_StartSound(NULL,sfx_menu1); // Tails
		vidm_current -= vidm_column_size;
		if (vidm_current<0)
			vidm_current = (vidm_column_size*3) + vidm_current;
		if (vidm_current>=vidm_nummodes)
			vidm_current = vidm_nummodes-1;
		break;

	  case KEY_RIGHTARROW:
		S_StartSound(NULL,sfx_menu1); // Tails
		vidm_current += vidm_column_size;
		if (vidm_current>=(vidm_column_size*3))
			vidm_current %= vidm_column_size;
		if (vidm_current>=vidm_nummodes)
			vidm_current = vidm_nummodes-1;
		break;

	  case KEY_ENTER:
		S_StartSound(NULL,sfx_menu1); // Tails
		if (!setmodeneeded) //in case the previous setmode was not finished
			setmodeneeded = modedescs[vidm_current].modenum+1;
		break;

	  case KEY_ESCAPE:      //this one same as M_Responder
// Tails        S_StartSound(NULL,sfx_swtchx);

		if (currentMenu->prevMenu)
			M_SetupNextMenu (currentMenu->prevMenu);
		else
			M_ClearMenus ();
		return;

	  case 'T':
	  case 't':
// Tails        S_StartSound(NULL,sfx_swtchx);
		vidm_testingmode = TICRATE*5;
		vidm_previousmode = vid.modenum;
		if (!setmodeneeded) //in case the previous setmode was not finished
			setmodeneeded = modedescs[vidm_current].modenum+1;
		return;

	  case 'D':
	  case 'd':
		// current active mode becomes the default mode.
// Tails        S_StartSound(NULL,sfx_swtchx);
		SCR_SetDefaultMode ();
		return;

	  default:
		break;
	}

}


//===========================================================================
//LOAD GAME MENU
//===========================================================================
void M_DrawLoad(void);

void M_LoadSelect(int choice);

enum
{
	load1,
	load2,
	load3,
	load4,
	load5,
	load6,
	load_end
} load_e;

menuitem_t LoadGameMenu[]=
{
	{IT_CALL | IT_NOTHING,"", M_LoadSelect,'1'},
	{IT_CALL | IT_NOTHING,"", M_LoadSelect,'2'},
	{IT_CALL | IT_NOTHING,"", M_LoadSelect,'3'},
	{IT_CALL | IT_NOTHING,"", M_LoadSelect,'4'},
	{IT_CALL | IT_NOTHING,"", M_LoadSelect,'5'},
	{IT_CALL | IT_NOTHING,"", M_LoadSelect,'6'}
};

menu_t  LoadDef =
{
	"M_LOADG",
	load_end,
	&MainDef,
	LoadGameMenu,
	M_DrawLoad,
	80,54,
	0
};

//
// M_LoadGame & Cie.
//
void M_DrawLoad(void)
{
	int             i;

	M_DrawGenericMenu();

	for (i = 0;i < load_end; i++)
	{
		M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
		V_DrawString(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);
	}
}

//
// User wants to load this game
//
void M_LoadSelect(int choice)
{
	G_LoadGame (choice);
	M_ClearMenus ();
}

//
// M_ReadSaveStrings
//  read the strings from the savegame files
//  and put it in savegamestrings global variable
//
void M_ReadSaveStrings(void)
{
	int     handle;
	int     count;
	int     i;
	char    name[256];

	for (i = 0;i < load_end;i++)
	{
		sprintf(name,savegamename,i);

		handle = open (name, O_RDONLY | 0, 0666);
		if (handle == -1)
		{
			strcpy(&savegamestrings[i][0],EMPTYSTRING);
			LoadGameMenu[i].status = 0;
			continue;
		}
		count = read (handle, &savegamestrings[i], SAVESTRINGSIZE);
		close (handle);
		LoadGameMenu[i].status = 1;
		(void)count;
	}
}

//
// Selected from DOOM menu
//
void M_LoadGame (int choice)
{
// change can't load message to can't load in server mode
	if (netgame && !server)
	{
		M_StartMessage(LOADNET,NULL,false);
		return;
	}

	M_SetupNextMenu(&LoadDef);
	M_ReadSaveStrings();
}


//===========================================================================
//                                SAVE GAME MENU
//===========================================================================
void M_DrawSave(void);

void M_SaveSelect(int choice);

menuitem_t SaveMenu[]=
{
	{IT_CALL | IT_NOTHING,"", M_SaveSelect,'1'},
	{IT_CALL | IT_NOTHING,"", M_SaveSelect,'2'},
	{IT_CALL | IT_NOTHING,"", M_SaveSelect,'3'},
	{IT_CALL | IT_NOTHING,"", M_SaveSelect,'4'},
	{IT_CALL | IT_NOTHING,"", M_SaveSelect,'5'},
	{IT_CALL | IT_NOTHING,"", M_SaveSelect,'6'}
};

menu_t  SaveDef =
{
	"M_SAVEG",
	load_end,
	&MainDef,
	SaveMenu,
	M_DrawSave,
	80,54,
	0
};



//
// Draw border for the savegame description
//
void M_DrawSaveLoadBorder(int x,int y)
{
	int             i;

	V_DrawScaledPatch (x-8,y+7,0,W_CachePatchName("M_LSLEFT",PU_CACHE));

	for (i = 0;i < 24;i++)
	{
		V_DrawScaledPatch (x,y+7,0,W_CachePatchName("M_LSCNTR",PU_CACHE));
		x += 8;
	}

	V_DrawScaledPatch (x,y+7,0,W_CachePatchName("M_LSRGHT",PU_CACHE));
}


//
//  M_SaveGame & Cie.
//
void M_DrawSave(void)
{
	int             i;

	M_DrawGenericMenu();

	for (i = 0;i < load_end; i++)
	{
		M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
		V_DrawString(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);
	}

	if (saveStringEnter && skullAnimCounter<4) // Tails 11-30-2000
	{
		i = V_StringWidth(savegamestrings[saveSlot]);
		V_DrawString(LoadDef.x + i,LoadDef.y+LINEHEIGHT*saveSlot,"_");
	}
}

//
// M_Responder calls this when user is finished
//
void M_DoSave(int slot)
{
	G_SaveGame (slot,savegamestrings[slot]);
	M_ClearMenus ();

	// PICK QUICKSAVE SLOT YET?
	if (quickSaveSlot == -2)
		quickSaveSlot = slot;
}

//
// User wants to save. Start string input for M_Responder
//
void M_SaveSelect(int choice)
{
	// we are going to be intercepting all chars
	saveStringEnter = 1;

	saveSlot = choice;
	strcpy(saveOldString,savegamestrings[choice]);
	if (!strcmp(savegamestrings[choice],EMPTYSTRING))
		savegamestrings[choice][0] = 0;
	saveCharIndex = strlen(savegamestrings[choice]);
}

//
// Selected from DOOM menu
//
void M_SaveGame (int choice)
{
	if(demorecording)
	{
		M_StartMessage("You can't save while recording demos\n\nPress a key\n",NULL,false);
		return;
	}

	if(cv_gametype.value) // Tails 04-25-2001
	{ // Tails 04-25-2001
		M_StartMessage("You can't save non-co-op games!\nPress a key\n",NULL,false); // Tails 04-25-2001
		return; // Tails 04-25-2001
	} // Tails 04-25-2001

	if (!usergame)
	{
		M_StartMessage(SAVEDEAD,NULL,false);
		return;
	}

	if (gamestate != GS_LEVEL)
		return;

	if (netgame && !server)
	{
		M_StartMessage("You are not the server",NULL,false);
		return;
	}

	M_SetupNextMenu(&SaveDef);
	M_ReadSaveStrings();
}

//===========================================================================
//                            QuickSAVE & QuickLOAD
//===========================================================================

//
//      M_QuickSave
//
char    tempstring[80];

void M_QuickSaveResponse(int ch)
{
	if (ch == 'y')
	{
		M_DoSave(quickSaveSlot);
// Tails        S_StartSound(NULL,sfx_swtchx);
	}
}

void M_QuickSave(void)
{
	if (!usergame)
	{
		S_StartSound(NULL,sfx_spring);
		return;
	}

	if (gamestate != GS_LEVEL)
		return;

	if (quickSaveSlot < 0)
	{
		M_StartControlPanel();
		M_ReadSaveStrings();
		M_SetupNextMenu(&SaveDef);
		quickSaveSlot = -2;     // means to pick a slot now
		return;
	}
	sprintf(tempstring,QSPROMPT,savegamestrings[quickSaveSlot]);
	M_StartMessage(tempstring,M_QuickSaveResponse,true);
}



//
// M_QuickLoad
//
void M_QuickLoadResponse(int ch)
{
	if (ch == 'y')
	{
		M_LoadSelect(quickSaveSlot);
// Tails        S_StartSound(NULL,sfx_swtchx);
	}
}


void M_QuickLoad(void)
{
	if (netgame)
	{
		M_StartMessage(QLOADNET,NULL,false);
		return;
	}

	if (quickSaveSlot < 0)
	{
		M_StartMessage(QSAVESPOT,NULL,false);
		return;
	}
	sprintf(tempstring,QLPROMPT,savegamestrings[quickSaveSlot]);
	M_StartMessage(tempstring,M_QuickLoadResponse,true);
}


//===========================================================================
//                                 END GAME
//===========================================================================

//
// M_EndGame
//
void M_EndGameResponse(int ch)
{
	if (ch != 'y')
		return;

	currentMenu->lastOn = itemOn;
	M_ClearMenus ();
	D_StartTitle ();
}

void M_EndGame(int choice)
{
	choice = 0;
	if (!usergame)
	{
		S_StartSound(NULL,sfx_spring);
		return;
	}

	if (netgame)
	{
		M_StartMessage(NETEND,NULL,false);
		return;
	}

	M_StartMessage(ENDGAME,M_EndGameResponse,true);
}

//===========================================================================
//                                 Quit Game
//===========================================================================

//
// M_QuitDOOM
//
int     quitsounds[8] =
{
	sfx_pldeth,
	sfx_dmpain,
	sfx_popain,
	sfx_pop,
	sfx_telept,
	sfx_posit1,
	sfx_posit3,
	sfx_sgtatk
};

int     quitsounds2[8] =
{
	sfx_spring, // Tails 11-09-99
	sfx_itemup, // Tails 11-09-99
	sfx_jump, // Tails 11-09-99
	sfx_pop,
	sfx_gloop, // Tails 11-09-99
	sfx_splash, // Tails 11-09-99
	sfx_floush, // Tails 11-09-99
	sfx_chchng // Tails 11-09-99
};



void M_QuitResponse(int ch)
{
	UINT32   time;

	if (ch != 'y')
		return;

	if (!netgame)
	{
		//added:12-02-98: quitsounds are much more fun than quitsounds2 (NOT!! -Tails)
		S_StartSound(NULL,quitsounds2[(gametic>>2)&7]); // Use quitsounds2, not quitsounds Tails 11-09-99

		//added:12-02-98: do that instead of I_WaitVbl which does not work
		time = I_GetTime() + TICRATE*4; // Shortened the quit time, used to be 2 seconds Tails 03-26-2001
		while (time > I_GetTime())
		{
			V_DrawScaledPatch (0,0,0,W_CachePatchName("DEMOQUIT",PU_CACHE)); // Demo 3 Quit Screen Tails 06-16-2001
			I_FinishUpdate (); // Update the screen with the image Tails 06-19-2001
		}
	}

	I_Quit ();
}




void M_QuitDOOM(int choice)
{
	// We pick index 0 which is language sensitive,
	//  or one at random, between 1 and maximum number.
	static char s[200];
	sprintf(s,text[DOSY_NUM],text[ QUITMSG_NUM+(gametic%NUM_QUITMESSAGES)]);
	M_StartMessage( s,M_QuitResponse,true);
}


//===========================================================================
//                              Some Draw routine
//===========================================================================

//
//      Menu Functions
//
void M_DrawThermo ( int   x,
					int   y,
					int   thermWidth,
					int   thermDot )
{
	int         xx;
	int         i;

	xx = x;
	V_DrawScaledPatch (xx,y,0,W_CachePatchName("M_THERML",PU_CACHE));
	xx += 8;
	for (i=0;i<thermWidth;i++)
	{
		V_DrawScaledPatch (xx,y,0,W_CachePatchName("M_THERMM",PU_CACHE));
		xx += 8;
	}
	V_DrawScaledPatch (xx,y,0,W_CachePatchName("M_THERMR",PU_CACHE));

	V_DrawScaledPatch ((x+8) + thermDot*4,y,
					   0,W_CachePatchName("M_THERMO",PU_CACHE));
}



void M_DrawEmptyCell( menu_t*       menu,
					  int           item )
{
	V_DrawScaledPatch (menu->x - 10,        menu->y+item*LINEHEIGHT - 1, 0,
					   W_CachePatchName("M_CELL1",PU_CACHE));
}

void M_DrawSelCell ( menu_t*       menu,
					 int           item )
{
	V_DrawScaledPatch (menu->x - 10,        menu->y+item*LINEHEIGHT - 1, 0,
					   W_CachePatchName("M_CELL2",PU_CACHE));
}


//
//  Draw a textbox, like Quake does, because sometimes it's difficult
//  to read the text with all the stuff in the background...
//
//added:06-02-98:
extern int st_borderpatchnum;   //st_stuff.c (for Glide)
void M_DrawTextBox (int x, int y, int width, int lines)
{
	patch_t  *p;
	int      cx, cy;
	int      n;

	// draw left side
	cx = x;
	cy = y;
	p = W_CachePatchName("brdr_tl",PU_CACHE);
	V_DrawScaledPatch (cx, cy, 0, p);
	p = W_CachePatchName ("brdr_l",PU_CACHE);
	for (n = 0; n < lines; n++)
	{
		cy += 8;
		V_DrawScaledPatch (cx, cy, 0, p);
	}
	p = W_CachePatchName ("brdr_bl",PU_CACHE);
	V_DrawScaledPatch (cx, cy+8, 0, p);

	// draw middle
	V_DrawFlatFill (x+8,y+8,width*8,lines*8,st_borderpatchnum);

	cx += 8;
	while (width > 0)
	{
		cy = y;
		p = W_CachePatchName ("brdr_t",PU_CACHE);
		V_DrawScaledPatch (cx, cy, 0, p);

		//p = W_CachePatchName ("brdr_mt",PU_CACHE);
		//cy += 8;
		//V_DrawScaledPatch (cx, cy, 0, p);

		//p = W_CachePatchName ("brdr_mm",PU_CACHE); //middle ombr� dessus
		//for (n = 1; n < lines; n++)
		//{
		//    cy += 8;
		//    V_DrawScaledPatch (cx, cy, 0, p);
		//}

		p = W_CachePatchName ("brdr_b",PU_CACHE);
		V_DrawScaledPatch (cx, y+8+lines*8, 0, p);
		width --;
		cx += 8;
	}

	// draw right side
	cy = y;
	p = W_CachePatchName ("brdr_tr",PU_CACHE);
	V_DrawScaledPatch (cx, cy, 0, p);
	p = W_CachePatchName ("brdr_r",PU_CACHE);
	for (n = 0; n < lines; n++)
	{
		cy += 8;
		V_DrawScaledPatch (cx, cy, 0, p);
	}
	p = W_CachePatchName ("brdr_br",PU_CACHE);
	V_DrawScaledPatch (cx, cy+8, 0, p);
}

//==========================================================================
//                        Message is now a (hackeble) Menu
//==========================================================================
void M_DrawMessageMenu(void);

menuitem_t MessageMenu[]=
{
	// TO HACK
	{0 ,"" , NULL ,0}
};

menu_t MessageDef =
{
	NULL,               // title
	1,                  // # of menu items
	NULL,               // previous menu       (TO HACK)
	MessageMenu,        // menuitem_t ->
	M_DrawMessageMenu,  // drawing routine ->
	0,0,                // x,y                 (TO HACK)
	0                   // lastOn, flags       (TO HACK)
};


void M_StartMessage ( char*         string,
					  void*         routine,
					  int           itemtype )
{
	int    max,start,i,lines;

	M_StartControlPanel(); // can't put menuactiv to true
	MessageDef.prevMenu = currentMenu;
	MessageDef.menuitems[0].name     = string;
	MessageDef.menuitems[0].alphaKey = itemtype;
	switch(itemtype) {
		case false:
			 MessageDef.menuitems[0].status     = IT_MSGHANDLER;
			 MessageDef.menuitems[0].itemaction = M_StopMessage;
			 break;
		case true:
			 MessageDef.menuitems[0].status     = IT_MSGHANDLER;
			 MessageDef.menuitems[0].itemaction = routine;
			 break;
		case 2:
			 MessageDef.menuitems[0].status     = IT_MSGHANDLER;
			 MessageDef.menuitems[0].itemaction = routine;
			 break;
	}
	//added:06-02-98: now draw a textbox around the message
	// compute lenght max and the numbers of lines
	max = 0;
	start = 0;
	for (lines=0; *(string+start); lines++)
	{
		for (i = 0;i < (int)strlen(string+start);i++)
		{
			if (*(string+start+i) == '\n')
			{
				if (i > max)
					max = i;
				start += i+1;
				i = -1; //added:07-02-98:damned!
				break;
			}
		}

		if (i == (int)strlen(string+start))
			start += i;
	}

	MessageDef.x=(BASEVIDWIDTH-8*max-16)/2;
	MessageDef.y=(BASEVIDHEIGHT - M_StringHeight(string))/2;

	MessageDef.lastOn = (lines<<8)+max;

//    M_SetupNextMenu();
	currentMenu = &MessageDef;
	itemOn=0;
}

void M_DrawMessageMenu(void)
{
	int    y;
	short  i,max;
	char   string[40];
	int    start,lines;
	char   *msg=currentMenu->menuitems[0].name;

	y=currentMenu->y;
	start = 0;
	lines = currentMenu->lastOn>>8;
	max = (currentMenu->lastOn & 0xFF)*8;
	M_DrawTextBox (currentMenu->x,y-8,(max+7)>>3,lines);

	while(*(msg+start))
	{
		for (i = 0;i < (int)strlen(msg+start);i++)
		{
			if (*(msg+start+i) == '\n')
			{
				memset(string,0,40);
				strncpy(string,msg+start,i);
				start += i+1;
				i = -1; //added:07-02-98:damned!
				break;
			}
		}

		if (i == (int)strlen(msg+start))
		{
			strcpy(string,msg+start);
			start += i;
		}

		V_DrawString((BASEVIDWIDTH - V_StringWidth(string))/2,y,string);
		y += 8; //SHORT(hu_font[0]->height);
	}
}

// default message handler
void M_StopMessage(int choice)
{
	M_SetupNextMenu(MessageDef.prevMenu);
// Tails    S_StartSound(NULL,sfx_swtchx);
}


//added:30-01-98:
//
//  Write a string centered using the hu_font
//
void M_CentreText (int y, char* string)
{
	int x;
	//added:02-02-98:centre on 320, because V_DrawString centers on vid.width...
	x = (BASEVIDWIDTH - V_StringWidth(string))>>1;
	V_DrawString(x,y,string);
}


//
// CONTROL PANEL
//

void M_ChangeCvar(int choise)
{
	consvar_t *cv=(consvar_t *)currentMenu->menuitems[itemOn].itemaction;

	if(((currentMenu->menuitems[itemOn].status & IT_CVARTYPE) == IT_CV_SLIDER )
	 ||((currentMenu->menuitems[itemOn].status & IT_CVARTYPE) == IT_CV_NOMOD  ))
	{
		CV_SetValue(cv,cv->value+choise*2-1);
	}
	else
		if(cv->flags & CV_FLOAT)
		{
			char s[20];
			sprintf(s,"%f",(float)cv->value/FRACUNIT+(choise*2-1)*(1.0/16.0));
			CV_Set(cv,s);
		}
		else
			CV_AddValue(cv,choise*2-1);
}

//
// M_Responder
//
boolean M_Responder (event_t* ev)
{
	int             ch;
	int             i;
	static  UINT32   joywait = 0;
	static  UINT32   mousewait = 0;
	static  int     mousey = 0;
	static  int     lasty = 0;
	static  int     mousex = 0;
	static  int     lastx = 0;
	void  (*routine)(int choice);  // for some casting problem

	ch = -1;

	if (ev->type == ev_joystick && joywait < I_GetTime())
	{
		if (ev->data3 == -1)
		{
			ch = KEY_UPARROW;
			joywait = I_GetTime() + 5;
		}
		else if (ev->data3 == 1)
		{
			ch = KEY_DOWNARROW;
			joywait = I_GetTime() + 5;
		}

		if (ev->data2 == -1)
		{
			ch = KEY_LEFTARROW;
			joywait = I_GetTime() + 2;
		}
		else if (ev->data2 == 1)
		{
			ch = KEY_RIGHTARROW;
			joywait = I_GetTime() + 2;
		}
	}
	else
	{
		if (ev->type == ev_mouse && mousewait < I_GetTime())
		{
			mousey += ev->data3;
			if (mousey < lasty-30)
			{
				ch = KEY_DOWNARROW;
				mousewait = I_GetTime() + 5;
				mousey = lasty -= 30;
			}
			else if (mousey > lasty+30)
			{
				ch = KEY_UPARROW;
				mousewait = I_GetTime() + 5;
				mousey = lasty += 30;
			}

			mousex += ev->data2;
			if (mousex < lastx-30)
			{
				ch = KEY_LEFTARROW;
				mousewait = I_GetTime() + 5;
				mousex = lastx -= 30;
			}
			else if (mousex > lastx+30)
			{
				ch = KEY_RIGHTARROW;
				mousewait = I_GetTime() + 5;
				mousex = lastx += 30;
			}
		}
		else
			if (ev->type == ev_keydown)
			{
				ch = ev->data1;

				// metzgermeister: Let users be free to choose their keypad mappings :)
#if 0
				//added:28-02-98: some people like to use the keypad...
				if (ch>KEY_KEYPAD7 && ch<KEY_KEYPAD0)
				{
				  switch(ch)
				  {
					case KEY_KEYPAD8:
					  ch = KEY_UPARROW;   break;
					case KEY_KEYPAD4:
					  ch = KEY_LEFTARROW; break;
					case KEY_KEYPAD6:
					  ch = KEY_RIGHTARROW;break;
					case KEY_KEYPAD2:
					  ch = KEY_DOWNARROW; break;
				  }
				}
#endif
				// added 5-2-98 remap virtual keys (mouse & joystick buttons)
				switch(ch) {
					case KEY_MOUSE1   : ch=KEY_ENTER;break;
					case KEY_MOUSE1+1 : ch=KEY_BACKSPACE;break;
					case KEY_JOY1     :
					case KEY_JOY1+2   :
					case KEY_JOY1+3   : ch=KEY_ENTER;break;
					case KEY_JOY1+1   : ch=KEY_BACKSPACE;break;
				}
			}
	}

	if (ch == -1)
		return false;


	// Save Game string input
	if (saveStringEnter)
	{
		switch(ch)
		{
		  case KEY_BACKSPACE:
			if (saveCharIndex > 0)
			{
				saveCharIndex--;
				savegamestrings[saveSlot][saveCharIndex] = 0;
			}
			break;

		  case KEY_ESCAPE:
			saveStringEnter = 0;
			strcpy(&savegamestrings[saveSlot][0],saveOldString);
			break;

		  case KEY_ENTER:
			saveStringEnter = 0;
			if (savegamestrings[saveSlot][0])
				M_DoSave(saveSlot);
			break;

		  default:
			ch = toupper(ch);
			if (ch != 32)
				if (ch-HU_FONTSTART < 0 || ch-HU_FONTSTART >= HU_FONTSIZE)
					break;
			if (ch >= 32 && ch <= 127 &&
				saveCharIndex < SAVESTRINGSIZE-1 &&
				V_StringWidth(savegamestrings[saveSlot]) <
				(SAVESTRINGSIZE-2)*8)
			{
				savegamestrings[saveSlot][saveCharIndex++] = ch;
				savegamestrings[saveSlot][saveCharIndex] = 0;
			}
			break;
		}
		return true;
	}

	if (devparm && ch == KEY_F1)
	{
		COM_BufAddText("screenshot\n");
		return true;
	}

#ifdef HERITAGE_SCREENSHOT_SHORTCUTS
	switch(ch)
	{
		case KEY_F8:            // Screenshots
			M_ScreenShot();
			return true;
		case KEY_F9:            // Movie mode
			((moviemode) ? M_StopMovie : M_StartMovie)();
			return true;
	}
#endif

	// F-Keys
	if (!menuactive)
		switch(ch)
		{
#ifdef HERITAGE_VIEWSIZE_FIX
			case KEY_MINUS:         // Screen size down Tails 11-09-99
				if (automapactive || chat_on || con_destlines)     // DIRTY !!! Tails 11-09-99
					return false;
				CV_SetValue (&cv_viewsize, cv_viewsize.value-1);
				S_StartSound(NULL,sfx_stnmov);
				return true;

			case KEY_EQUALS:        // Screen size up Tails 11-09-99
				if (automapactive || chat_on || con_destlines)     // DIRTY !!! Tails 11-09-99
					return false;
				CV_SetValue (&cv_viewsize, cv_viewsize.value+1);
				S_StartSound(NULL,sfx_stnmov);
				return true;
#endif
			case KEY_F1:            // Help key
				M_StartControlPanel ();
				currentMenu = &ReadDef1;
				itemOn = 0;
				return true;

			case KEY_F2:            // Save
				M_StartControlPanel();
				M_SaveGame(0);
				return true;

			case KEY_F3:            // Load
				M_StartControlPanel();
				M_LoadGame(0);
				return true;

			case KEY_F4:            // Sound Volume
				M_StartControlPanel ();
				currentMenu = &SoundDef;
				itemOn = sfx_vol;
				return true;

			//added:26-02-98: now F5 calls the Video Menu
			case KEY_F5:
				M_StartControlPanel();
				M_SetupNextMenu (&VidModeDef);
				return true;

#ifndef HERITAGE_SCREENSHOT_SHORTCUTS
			case KEY_F6:            // Quicksave
				M_QuickSave();
				return true;
#endif

			//added:26-02-98: F7 changed to Options menu
			case KEY_F7:            // End game
				M_StartControlPanel();
				M_SetupNextMenu (&OptionsDef);
				return true;

#ifndef HERITAGE_SCREENSHOT_SHORTCUTS
			case KEY_F8:            // Toggle messages
				CV_AddValue(&cv_showmessages,+1);
				return true;

			case KEY_F9:            // Quickload
				M_QuickLoad();
				return true;
#endif

			case KEY_F10:           // Quit DOOM
				M_QuitDOOM(0);
				return true;

			//added:10-02-98: the gamma toggle is now also in the Options menu
			case KEY_F11:
				CV_AddValue (&cv_usegamma,+1);
				return true;
		}

	// Pop-up menu?
	if (!menuactive)
	{
		if (ch == KEY_ESCAPE)
		{
			M_StartControlPanel ();
			return true;
		}
		return false;
	}

	// disable calling console while in menu
	if (ch == gamecontrol[gc_console][0] ||
		ch == gamecontrol[gc_console][1])
	{
			return true; //eat key
	}

	routine = currentMenu->menuitems[itemOn].itemaction;

	//added:30-01-98:
	// Handle menuitems which need a specific key handling
	if(routine && (currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_KEYHANDLER )
	{
		routine(ch);
		return true;
	}

	if(currentMenu->menuitems[itemOn].status==IT_MSGHANDLER)
	{
		if(currentMenu->menuitems[itemOn].alphaKey==true)
		{
			if(ch == ' ' || ch == 'n' || ch == 'y' || ch == KEY_ESCAPE)
			{
				if(routine) routine(ch);
				M_StopMessage(0);
				return true;
			}
			return false;
		}
		else
		{
			//added:07-02-98:dirty hak:for the customise controls, I want only
			//      buttons/keys, not moves
			if (ev->type==ev_mouse || ev->type==ev_joystick )
				return false;
			if(routine) routine((int)ev);
			return true;
		}
	}

	// BP: one of the more big hack i have never made
	if(routine && (currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_CVAR )
		routine=M_ChangeCvar;

	// Keys usable within menu
	switch (ch)
	{
	  case KEY_DOWNARROW:
		do
		{
			if (itemOn+1 > currentMenu->numitems-1)
				itemOn = 0;
			else itemOn++;
		} while((currentMenu->menuitems[itemOn].status & IT_TYPE)==IT_SPACE);
		S_StartSound(NULL,sfx_menu1); // was pstop Tails 11-30-2000
		return true;

	  case KEY_UPARROW:
		do
		{
			if (!itemOn)
				itemOn = currentMenu->numitems-1;
			else itemOn--;
		} while((currentMenu->menuitems[itemOn].status & IT_TYPE)==IT_SPACE);
		S_StartSound(NULL,sfx_menu1); // was pstop Tails 11-30-2000
		return true;

	  case KEY_LEFTARROW:
		if (  routine &&
			( (currentMenu->menuitems[itemOn].status & IT_TYPE) ==  IT_ARROWS
			||(currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_CVAR   ))
		{
			S_StartSound(NULL,sfx_menu1); // Tails 11-30-2000
			routine(0);
		}
		return true;

	  case KEY_RIGHTARROW:
		if ( routine &&
			( (currentMenu->menuitems[itemOn].status & IT_TYPE) ==  IT_ARROWS
			||(currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_CVAR   ))
		{
			S_StartSound(NULL,sfx_menu1); // Tails 11-30-2000
			routine(1);
		}
		return true;

	  case KEY_ENTER:
		currentMenu->lastOn = itemOn;
		if ( routine )
		{
			switch (currentMenu->menuitems[itemOn].status & IT_TYPE)  {
				case IT_CVAR:
				case IT_ARROWS:
					routine(1);            // right arrow
					S_StartSound(NULL,sfx_menu1); // Tails
					break;
				case IT_CALL:
					routine(itemOn);
					S_StartSound(NULL,sfx_menu1);
					break;
				case IT_SUBMENU:
					currentMenu->lastOn = itemOn;
					M_SetupNextMenu((menu_t *)currentMenu->menuitems[itemOn].itemaction);
					S_StartSound(NULL,sfx_menu1);
					break;
			}
		}
		return true;

	  case KEY_ESCAPE:
		currentMenu->lastOn = itemOn;
		M_ClearMenus ();
		return true;

	  case KEY_BACKSPACE:
		if((currentMenu->menuitems[itemOn].status)==IT_CONTROL)
		{
			S_StartSound(NULL,sfx_stnmov);
			// detach any keys associated to the game control
			G_ClearControlKeys (setupcontrols, currentMenu->menuitems[itemOn].alphaKey);
			return true;
		}
		currentMenu->lastOn = itemOn;
		if (currentMenu->prevMenu)
		{
			currentMenu = currentMenu->prevMenu;
			itemOn = currentMenu->lastOn;
		}
		return true;

	  default:
		for (i = itemOn+1;i < currentMenu->numitems;i++)
			if (currentMenu->menuitems[i].alphaKey == ch)
			{
				itemOn = i;
				S_StartSound(NULL,sfx_menu1); // Tails 11-30-2000
				return true;
			}
		for (i = 0;i <= itemOn;i++)
			if (currentMenu->menuitems[i].alphaKey == ch)
			{
				itemOn = i;
				S_StartSound(NULL,sfx_menu1); // Tails 11-30-2000
				return true;
			}
		break;

	}

	return false;
}



//
//      Find string height from hu_font chars
//
int M_StringHeight(char* string)
{
	int      i;
	int      h;
	int      height = 8; //SHORT(hu_font[0]->height);

	h = height;
	for (i = 0;i < (int)strlen(string);i++)
		if (string[i] == '\n')
			h += height;

	return h;
}


//
// M_Drawer
// Called after the view has been rendered,
// but before it has been blitted.
//
void M_Drawer (void)
{
	if (!menuactive)
		return;

	//added:18-02-98:
	// center the scaled graphics for the menu,
	//  set it 0 again before return!!!
	scaledofs = vid.centerofs;

#ifdef HERITAGE_FADE_MENU_BACKGROUND
	// now that's more readable with a faded background (yeah like Quake...)
	V_DrawFadeScreen (); // Tails 11-30-2000 (Re-added by Lactozilla, really hard to read otherwise)
#endif

	if (currentMenu->drawroutine)
		currentMenu->drawroutine();      // call current menu Draw routine

	//added:18-02-98: it should always be 0 for non-menu scaled graphics.
	scaledofs = 0;

}

//
// M_StartControlPanel
//
void M_StartControlPanel (void)
{
	// intro might call this repeatedly
	if (menuactive)
		return;

	menuactive = 1;
	currentMenu = &MainDef;         // JDC
	itemOn = currentMenu->lastOn;   // JDC

#ifndef HERITAGE_PAUSE_BEHAVIOR
	if(!netgame && !demoplayback && !paused)
		COM_BufAddText("pause\n");
#endif

#ifdef HERITAGE_CONNECTION_SCREEN
	if (!CL_ConnectionScreen())
#endif
		CON_ToggleOff ();   // dirty hack : move away console
}

//
// M_ClearMenus
//
void M_ClearMenus (void)
{
	if(!menuactive)
		return;

	if( currentMenu->quitroutine )
	{
		if( !currentMenu->quitroutine())
			return; // we can't quit this menu (also used to set parameter from the menu)
	}

#ifndef HERITAGE_PAUSE_BEHAVIOR
	if(!netgame && !demoplayback && paused)
		COM_BufInsertText("pause\n"); // insert in head (first unpause after load game)
#endif

	menuactive = 0;
}


//
// M_SetupNextMenu
//
void M_SetupNextMenu(menu_t *menudef)
{
	if( currentMenu->quitroutine )
	{
		if( !currentMenu->quitroutine())
			return; // we can't quit this menu (also used to set parameter from the menu)
	}
	currentMenu = menudef;
	itemOn = currentMenu->lastOn;

	// in case of...
	if (itemOn >= currentMenu->numitems)
		itemOn = currentMenu->numitems - 1;

	// the curent item can be desabled,
	// this code go up until a enabled item found
	while(currentMenu->menuitems[itemOn].status==IT_DISABLED && itemOn)
		itemOn--;
}


//
// M_Ticker
//
void M_Ticker (void)
{
	if (--skullAnimCounter <= 0)
	{
		whichSkull ^= 1;
		skullAnimCounter = 8;
	}

	//added:30-01-98:test mode for five seconds
	if( vidm_testingmode>0 )
	{
		// restore the previous video mode
		if (--vidm_testingmode==0)
			setmodeneeded = vidm_previousmode+1;
	}
}


//
// M_Init
//
void M_Init (void)
{
	currentMenu = &MainDef;
	menuactive = 0;
	itemOn = currentMenu->lastOn;

	whichSkull = 0;
	skullAnimCounter = 10;

	quickSaveSlot = -1;

	// Here we could catch other version dependencies,
	//  like HELP1/2, and four episodes.


	switch ( gamemode )
	{
	  case commercial:
		// This is used because DOOM 2 had only one HELP
		//  page. I use CREDIT as second page now, but
		//  kept this hack for educational purposes.
//        MainMenu[readthis] = MainMenu[quitdoom]; // Tails
		MainDef.numitems--;
		MainDef.y += 8;
		NewDef.prevMenu = &MainDef;
		ReadDef1.drawroutine = M_DrawReadThis1;
		ReadDef1.x = 330;
		ReadDef1.y = 165;
		ReadMenu1[0].itemaction = &MainDef;
		break;
	  case shareware:
		// Episode 2 and 3 are handled,
		//  branching to an ad screen.
	  case registered:
		// We need to remove the fourth episode.
		EpiDef.numitems--;
		break;
	  case retail:
		// We are fine.
	  default:
		break;
	}
}


//======================================================================
// OpenGL specifics options
//======================================================================

#ifdef HWRENDER

void M_DrawOpenGLMenu(void);
void M_OGL_DrawFogMenu(void);
void M_OGL_DrawColorMenu(void);
void M_HandleFogColor (int choice);
void M_HandleScreenDepth(int choice);
menu_t OGL_LightingDef, OGL_FogDef, OGL_ColorDef, OGL_DevDef;

menuitem_t OpenGLOptionsMenu[]=
{
	{IT_STRING | IT_CVAR, "Mouse look"          , &cv_grcrappymlook     ,  0},
	{IT_STRING | IT_CVAR, "Field of view"       , &cv_grfov             , 10},
	{IT_STRING | IT_CVAR, "Quality"             , &cv_scr_depth         , 20},

	{IT_SUBMENU|IT_WHITESTRING, "Lighting..."       , &OGL_LightingDef   , 60},
	{IT_SUBMENU|IT_WHITESTRING, "Fog..."            , &OGL_FogDef        , 70},
	{IT_SUBMENU|IT_WHITESTRING, "Color..."          , &OGL_ColorDef      , 80},
//    {IT_SUBMENU|IT_WHITESTRING, "Development..."    , &OGL_DevDef        , 90},
};

menuitem_t OGL_LightingMenu[]=
{
	{IT_STRING | IT_CVAR, "Coronas"                 , &cv_grcoronas         ,  0},
	{IT_STRING | IT_CVAR, "Dynamic lighting"        , &cv_grdynamiclighting , 10},
	{IT_STRING | IT_CVAR, "Static lighting"         , &cv_grstaticlighting  , 20},
	{IT_STRING | IT_CVAR, "Monsters' balls lighting", &cv_grmblighting      , 30},
};

menuitem_t OGL_FogMenu[]=
{
	{IT_STRING | IT_CVAR, "Fog"             , &cv_grfog              ,  0},
	{IT_STRING | IT_KEYHANDLER, "Fog color" , M_HandleFogColor       , 10},
	{IT_STRING | IT_CVAR, "Fog density"     , &cv_grfogdensity       , 20},
};

menuitem_t OGL_ColorMenu[]=
{
	//{IT_STRING | NOTHING, "Gamma correction", NULL                   ,  0},
	{IT_STRING | IT_CVAR | IT_CV_SLIDER, "red"  , &cv_grgammared     , 10},
	{IT_STRING | IT_CVAR | IT_CV_SLIDER, "green", &cv_grgammagreen   , 20},
	{IT_STRING | IT_CVAR | IT_CV_SLIDER, "blue" , &cv_grgammablue    , 30},
	//{IT_STRING | IT_CVAR | IT_CV_SLIDER, "Constrast", &cv_grcontrast , 50},
};

menuitem_t OGL_DevMenu[]=
{
//    {IT_STRING | IT_CVAR, "Polygon smooth"  , &cv_grpolygonsmooth    ,  0},
	{IT_STRING | IT_CVAR, "MD2 models"      , &cv_grmd2              , 10},
};

menu_t  OpenGLOptionDef =
{
	"M_OPTTTL",
	sizeof(OpenGLOptionsMenu)/sizeof(menuitem_t),
	&VideoOptionsDef,
	OpenGLOptionsMenu,
	M_DrawOpenGLMenu,
	60,40,
	0
};

menu_t  OGL_LightingDef =
{
	"M_OPTTTL",
	sizeof(OGL_LightingMenu)/sizeof(menuitem_t),
	&OpenGLOptionDef,
	OGL_LightingMenu,
	M_DrawGenericMenu,
	60,40,
	0,
};

menu_t  OGL_FogDef =
{
	"M_OPTTTL",
	sizeof(OGL_FogMenu)/sizeof(menuitem_t),
	&OpenGLOptionDef,
	OGL_FogMenu,
	M_OGL_DrawFogMenu,
	60,40,
	0,
};

menu_t  OGL_ColorDef =
{
	"M_OPTTTL",
	sizeof(OGL_ColorMenu)/sizeof(menuitem_t),
	&OpenGLOptionDef,
	OGL_ColorMenu,
	M_OGL_DrawColorMenu,
	60,40,
	0,
};

menu_t  OGL_DevDef =
{
	"M_OPTTTL",
	sizeof(OGL_DevMenu)/sizeof(menuitem_t),
	&OpenGLOptionDef,
	OGL_DevMenu,
	M_DrawGenericMenu,
	60,40,
	0,
};


//======================================================================
// M_DrawOpenGLMenu()
//======================================================================
void M_DrawOpenGLMenu(void)
{
	int             mx,my;

	mx = OpenGLOptionDef.x;
	my = OpenGLOptionDef.y;
	M_DrawGenericMenu(); // use generic drawer for cursor, items and title
	V_DrawStringWhite(BASEVIDWIDTH-mx-V_StringWidth(cv_scr_depth.string),
					  my+currentMenu->menuitems[2].alphaKey,
					  cv_scr_depth.string);
}


#define FOG_COLOR_ITEM  1
//======================================================================
// M_OGL_DrawFogMenu()
//======================================================================
void M_OGL_DrawFogMenu(void)
{
	int             mx,my;

	mx = OGL_FogDef.x;
	my = OGL_FogDef.y;
	M_DrawGenericMenu(); // use generic drawer for cursor, items and title
	V_DrawStringWhite(BASEVIDWIDTH-mx-V_StringWidth (cv_grfogcolor.string),
					  my+currentMenu->menuitems[FOG_COLOR_ITEM].alphaKey,
					  cv_grfogcolor.string);
	if (itemOn==FOG_COLOR_ITEM && skullAnimCounter<4) //blink cursor on 4th item if selected
		V_DrawMappedPatch (BASEVIDWIDTH-mx, my+currentMenu->menuitems[FOG_COLOR_ITEM].alphaKey,
						   0, W_CachePatchName("STCFN095",PU_CACHE), whitemap);
}


//======================================================================
// M_OGL_DrawColorMenu()
//======================================================================
void M_OGL_DrawColorMenu(void)
{
	int             mx,my;

	mx = OGL_ColorDef.x;
	my = OGL_ColorDef.y;
	M_DrawGenericMenu(); // use generic drawer for cursor, items and title
	V_DrawStringWhite(mx, my+currentMenu->menuitems[0].alphaKey-10,
					  "Gamma correction");
}


//======================================================================
// M_OpenGLOption()
//======================================================================
void M_OpenGLOption(int choice)
{
	// TODO: The same for Glide
	if (rendermode == render_opengl )
		M_SetupNextMenu(&OpenGLOptionDef);
	else
		M_StartMessage("You are not in OpenGL mode\nYou can't change the options\n",NULL,false);
}

// New function for player setup menu Tails
//======================================================================
// M_SetupMultiOption()
//======================================================================
/*void M_SetupMultiOption(int choice)
{
//    if (netgame || multiplayer)
		M_SetupMultiPlayer(setupplayer1);
//    else
//        M_StartMessage("You are in single player\nYou can't use this option\n",NULL,false);
}*/

//======================================================================
// M_HandleFogColor()
//======================================================================
void M_HandleFogColor (int choice)
{
	int      i, l;
	char     temp[8];
	char     fogcolor[7]; // Lactozilla
	boolean  exitmenu = false;  // exit to previous menu and send name change

	switch( choice )
	{
	  case KEY_DOWNARROW:
		S_StartSound(NULL,sfx_menu1); // Tails 11-30-2000
		itemOn++;
		break;

	  case KEY_UPARROW:
		S_StartSound(NULL,sfx_menu1); // Tails 11-30-2000
		itemOn--;
		break;

	  case KEY_ESCAPE:
		S_StartSound(NULL,sfx_menu1); // Tails 11-30-2000
		exitmenu = true;
		break;

	  case KEY_BACKSPACE:
		S_StartSound(NULL,sfx_menu1); // Tails 11-30-2000
		strcpy(temp, cv_grfogcolor.string);
		strcpy(fogcolor, "000000");
		l = strlen(temp)-1;
		for (i=0; i<l; i++)
			fogcolor[i+6-l] = temp[i];
		fogcolor[6] = '\0';
		CV_Set(&cv_grfogcolor, fogcolor);
		break;

	  default:
		if ((choice >= '0' && choice <= '9') ||
			(choice >= 'a' && choice <= 'f') ||
			(choice >= 'A' && choice <= 'F')) {
			S_StartSound(NULL,sfx_menu1); // Tails 11-30-2000
			strcpy(temp, cv_grfogcolor.string);
			strcpy(fogcolor, "000000");
			l = strlen(temp);
			for (i=0; i<l; i++)
				fogcolor[5-i] = temp[l-i];
			fogcolor[5] = choice;
			fogcolor[6] = '\0';
			CV_Set(&cv_grfogcolor, fogcolor);
		}
		break;
	}
	if (exitmenu)
	{
		if (currentMenu->prevMenu)
			M_SetupNextMenu (currentMenu->prevMenu);
		else
			M_ClearMenus ();
	}
}

#endif
