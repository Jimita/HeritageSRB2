// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: w_wad.c,v 1.15 2000/07/01 09:23:49 bpereira Exp $
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
// $Log: w_wad.c,v $
// Revision 1.15  2000/07/01 09:23:49  bpereira
// no message
//
// Revision 1.14  2000/05/09 20:57:58  hurdler
// use my own code for colormap (next time, join with Boris own code)
// (necessary due to a small bug in Boris' code (not found) which shows strange effects under linux)
//
// Revision 1.13  2000/04/30 10:30:10  bpereira
// no message
//
// Revision 1.12  2000/04/27 17:43:19  hurdler
// colormap code in hardware mode is now the default
//
// Revision 1.11  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.10  2000/04/13 23:47:47  stroggonmeth
// See logs
//
// Revision 1.9  2000/04/11 19:07:25  stroggonmeth
// Finished my logs, fixed a crashing bug.
//
// Revision 1.8  2000/04/09 02:30:57  stroggonmeth
// Fixed missing sprite def
//
// Revision 1.7  2000/04/08 11:27:29  hurdler
// fix some boom stuffs
//
// Revision 1.6  2000/04/07 01:39:53  stroggonmeth
// Fixed crashing bug in Linux.
// Made W_ColormapNumForName search in the other direction to find newer colormaps.
//
// Revision 1.5  2000/04/06 20:40:22  hurdler
// Mostly remove warnings under windows
//
// Revision 1.4  2000/04/05 15:47:47  stroggonmeth
// Added hack for Dehacked lumps. Transparent sprites are now affected by colormaps.
//
// Revision 1.3  2000/04/04 00:32:48  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Handles WAD file header, directory, lump I/O.
//
//-----------------------------------------------------------------------------


// added for linux 19990220 by Kin
#ifdef LINUX
#define O_BINARY 0
#endif

#include <malloc.h>
#include <fcntl.h>
#ifndef __WIN32__
#include <unistd.h>
#endif

#include "doomdef.h"
#include "doomtype.h"
#include "w_wad.h"
#include "z_zone.h"

#include "i_video.h"        //rendermode
#include "d_netfil.h"
#include "dehacked.h"

#ifdef HWRENDER
#include "hardware/hw_main.h"
#endif


//===========================================================================
//                                                                    GLOBALS
//===========================================================================
int          numwadfiles;             // number of active wadfiles
wadfile_t*   wadfiles[MAX_WADFILES];  // 0 to numwadfiles-1 are valid

//===========================================================================
//                                                        LUMP BASED ROUTINES
//===========================================================================

// W_AddFile
// All files are optional, but at least one file must be
//  found (PWAD, if all required lumps are present).
// Files with a .wad extension are wadlink files
//  with multiple lumps.
// Other files are single lumps with the base filename
//  for the lump name.
//
// If filename starts with a tilde, the file is handled
//  specially to allow map reloads.
// But: the reload feature is a fragile hack...

static char filenamebuf[MAX_WADPATH];

// W_OpenWadFile
// Helper function for opening the WAD file.
// Returns the FILE * handle for the file, or NULL if not found or could not be opened
// If "useerrors" is true then print errors in the console, else just don't bother
// "filename" may be modified to have the correct path the actual file is located in, if necessary
static FILE *W_OpenWadFile(const char **filename)
{
	FILE *handle;

	// Officially, strncpy should not have overlapping buffers, since W_VerifyNMUSlumps is called after this, and it
	// changes filename to point at filenamebuf, it would technically be doing that. I doubt any issue will occur since
	// they point to the same location, but it's better to be safe and this is a simple change.
	if (filenamebuf != *filename)
	{
		strncpy(filenamebuf, *filename, MAX_WADPATH);
		filenamebuf[MAX_WADPATH - 1] = '\0';
		*filename = filenamebuf;
	}

	// open wad file
	if ((handle = fopen(*filename, "rb")) == NULL)
	{
		// If we failed to load the file with the path as specified by
		// the user, strip the directories and search for the file.
		nameonly(filenamebuf);

		// If findfile finds the file, the full path will be returned
		// in filenamebuf == *filename.
		if (findfile(filenamebuf, NULL, true))
		{
			if ((handle = fopen(*filename, "rb")) == NULL)
			{
				CONS_Printf("Can't open %s\n", *filename);
				return NULL;
			}
		}
		else
		{
			CONS_Printf("File %s not found.\n", *filename);
			return NULL;
		}
	}

	return handle;
}

//  Allocate a wadfile, setup the lumpinfo (directory) and
//  lumpcache, add the wadfile to the current active wadfiles
//
//  now returns index into wadfiles[], you can get wadfile_t*
//  with:
//       wadfiles[<return value>]
//
//  return -1 in case of problem
//
int W_LoadWadFile (const char *filename)
{
	FILE*            handle;
	wadinfo_t        header;
	int              numlumps;
	filelump_t*      fileinfo;
	filelump_t*      fileinfo_p;
	lumpinfo_t*      lump_p;
	lumpinfo_t*      lumpinfo;
	lumpcache_t*     lumpcache;
	wadfile_t*       wadfile;
	int              length;
	int              i;
#ifdef HWRENDER
	GlidePatch_t*    grPatch;
#endif

	//
	// check if limit of active wadfiles
	//
	if (numwadfiles>=MAX_WADFILES)
	{
		CONS_Printf ("Maximum wad files reached\n");
		return -1;
	}

	// open wad file
	if ((handle = W_OpenWadFile(&filename)) == NULL)
	{
		CONS_Printf ("Couldn't open %s\n", filename);
		return -1;
	}

	// read the header
	fread (&header, 1, sizeof(header), handle);

	if (strncmp(header.identification,"IWAD",4))
	{
		// Homebrew levels?
		if (strncmp(header.identification,"PWAD",4))
		{
			if (strncmp(header.identification,"SDLL",4)) // Support "SDLL" files Tails 12-17-2001
			{
				CONS_Printf ("%s doesn't have IWAD or PWAD id\n", filename);
				return -1;
			}
		}
	}

	header.numlumps = LONG(header.numlumps);
	header.infotableofs = LONG(header.infotableofs);

	// read wad file directory
	length = header.numlumps*sizeof(filelump_t);
	fileinfo = fileinfo_p = malloc (length); // Lactozilla: Replaced alloca
	fseek (handle, header.infotableofs, SEEK_SET);
	fread (fileinfo, 1, length, handle);

	numlumps = header.numlumps;

	// fill in lumpinfo for this wad
	lump_p = lumpinfo = Z_Malloc (numlumps*sizeof(lumpinfo_t),PU_STATIC,NULL);

	for (i=0 ; i<numlumps ; i++,lump_p++, fileinfo_p++)
	{
		lump_p->position = LONG(fileinfo_p->filepos);
		lump_p->size     = LONG(fileinfo_p->size);
		strncpy (lump_p->name, fileinfo_p->name, 8);
	}

	free(fileinfo);

	//
	//  link wad file to search files
	//
	wadfile = Z_Malloc (sizeof (wadfile_t),PU_STATIC,NULL);
	strncpy (wadfile->filename, filename, MAX_WADPATH);
	wadfile->handle = handle;
	wadfile->numlumps = numlumps;
	wadfile->lumpinfo = lumpinfo;
	fseek (handle, 0, SEEK_END);
	wadfile->filesize = (unsigned)ftell(handle);

	//
	//  set up caching
	//
	length = numlumps * sizeof(lumpcache_t);
	lumpcache = Z_Malloc (length,PU_STATIC,NULL);

	memset (lumpcache, 0, length);
	wadfile->lumpcache = lumpcache;

#ifdef HWRENDER
	//faB: now allocates GlidePatch info structures STATIC from the start,
	//     because these were causing a lot of fragmentation of the heap,
	//     considering they are never freed.
	length = numlumps * sizeof(GlidePatch_t);
	grPatch = Z_Malloc (length, PU_3DFXPATCHINFO, 0);    //never freed
	// set mipmap.downloaded to false
	memset (grPatch, 0, length);
	for (i=0; i<numlumps; i++)
	{
		//store the software patch lump number for each GlidePatch
		grPatch[i].patchlump = (numwadfiles<<16) + i;
	}
	wadfile->cache3Dfx = grPatch;
#endif

	//
	//  add the wadfile
	//
	wadfiles[numwadfiles++] = wadfile;

	CONS_Printf ("Added wadfile %s (%i lumps)\n", filename, numlumps);
	return numwadfiles-1;
}

//
// W_InitMultipleFiles
// Pass a null terminated list of files to use.
// All files are optional, but at least one file
//  must be found.
// Files with a .wad extension are idlink files
//  with multiple lumps.
// Other files are single lumps with the base filename
//  for the lump name.
// Lump names can appear multiple times.
// The name searcher looks backwards, so a later file
//  does override all earlier ones.
//
int W_InitMultipleFiles (char** filenames)
{
	int         rc=1;

	// open all the files, load headers, and count lumps
	numwadfiles = 0;

	// will be realloced as lumps are added
	for ( ; *filenames ; filenames++)
		rc &= (W_LoadWadFile (*filenames) != -1) ? 1 : 0;

	if (!numwadfiles)
		I_Error ("W_InitMultipleFiles: no files found");

	return rc;
}


// !!!NOT CHECKED WITH NEW WAD SYSTEM
//
// W_InitFile
// Just initialize from a single file.
//
/*
void W_InitFile (char* filename)
{
	char*       names[2];

	names[0] = filename;
	names[1] = NULL;
	W_InitMultipleFiles (names);
}*/


// !!!NOT CHECKED WITH NEW WAD SYSTEM
//
// W_NumLumps
//
/*
int W_NumLumps (void)
{
	return numlumps;
}*/



//
//  W_CheckNumForName
//  Returns -1 if name not found.
//

// this is normally always false, so external pwads take precedence,
//  this is set true temporary as W_GetNumForNameFirst() is called
static boolean scanforward = false;

int W_CheckNumForName (char* name)
{
	union {
				char    s[9];
				int             x[2];
	} name8;

	int         i,j;
	int         v1;
	int         v2;
	lumpinfo_t* lump_p;

	// make the name into two integers for easy compares
	strncpy (name8.s,name,8);

	// in case the name was a fill 8 chars
	name8.s[8] = 0;

	// case insensitive
	strupr (name8.s);

	v1 = name8.x[0];
	v2 = name8.x[1];

	if (!scanforward)
	{
		//
		// scan wad files backwards so patch lump files take precedence
		//
		for (i = numwadfiles-1 ; i>=0; i--)
		{
			lump_p = wadfiles[i]->lumpinfo;

			for (j = 0; j<wadfiles[i]->numlumps; j++,lump_p++)
			{
				if (    *(int *)lump_p->name == v1
					 && *(int *)&lump_p->name[4] == v2)
				{
					// high word is the wad file number
					return ((i<<16) + j);
				}
			}
		}
		// not found.
		return -1;
	}

	//
	// scan wad files forward, when original wad resources
	//  must take precedence
	//
	for (i = 0; i<numwadfiles; i++)
	{
		lump_p = wadfiles[i]->lumpinfo;
		for (j = 0; j<wadfiles[i]->numlumps; j++,lump_p++)
		{
			if (    *(int *)lump_p->name == v1
				 && *(int *)&lump_p->name[4] == v2)
			{
				return ((i<<16) + j);
			}
		}
	}
	// not found.
	return -1;
}


//
//  Same as the original, but checks in one pwad only
//  wadid is a wad number
//  (Used for sprites loading)
//
//  'startlump' is the lump number to start the search
//
int W_CheckNumForNamePwad (char* name, int wadid, int startlump)
{
	union {
		char  s[9];
		int   x[2];
	} name8;

	int         i;
	int         v1;
	int         v2;
	lumpinfo_t* lump_p;

	strncpy (name8.s,name,8);
	name8.s[8] = 0;
	strupr (name8.s);

	v1 = name8.x[0];
	v2 = name8.x[1];

	//
	// scan forward
	// start at 'startlump', useful parameter when there are multiple
	//                       resources with the same name
	//
	if (startlump < wadfiles[wadid]->numlumps)
	{
		lump_p = wadfiles[wadid]->lumpinfo + startlump;
		for (i = startlump; i<wadfiles[wadid]->numlumps; i++,lump_p++)
		{
			if ( *(int *)lump_p->name == v1
			  && *(int *)&lump_p->name[4] == v2)
			{
				return ((wadid<<16)+i);
			}
		}
	}

	// not found.
	return -1;
}



//
// W_GetNumForName
//   Calls W_CheckNumForName, but bombs out if not found.
//
int W_GetNumForName (char* name)
{
	int i;

	i = W_CheckNumForName (name);

	if (i == -1)
		I_Error ("W_GetNumForName: %s not found!\n", name);

	return i;
}

int W_CheckNumForNameFirst (char* name)
{
	int i;

	// 3am coding.. force a scan of resource name forward, for one call
	scanforward = true;
	i = W_CheckNumForName (name);
	scanforward = false;

	return i;
}

//
//  W_GetNumForNameFirst : like W_GetNumForName, but scans FORWARD
//                         so it gets resources from the original wad first
//  (this is used only to get S_START for now, in r_data.c)
int W_GetNumForNameFirst (char* name)
{
	int i;

	i = W_CheckNumForNameFirst (name);
	if (i == -1)
		I_Error ("W_GetNumForNameFirst: %s not found!", name);

	return i;
}


//
//  W_LumpLength
//   Returns the buffer size needed to load the given lump.
//
int W_LumpLength (int lump)
{
#ifdef PARANOIA
	if (lump<0) I_Error("W_LumpLenght: lump not exist\n");

	if ((lump&0xFFFF) >= wadfiles[lump>>16]->numlumps)
		I_Error ("W_LumpLength: %i >= numlumps",lump);
#endif
	return wadfiles[lump>>16]->lumpinfo[lump&0xFFFF].size;
}



//
// W_ReadLumpHeader : read 'size' bytes of lump
//                    sometimes just the header is needed
//
//Fab:02-08-98: now returns the number of bytes read (should == size)
int  W_ReadLumpHeader ( int           lump,
						void*         dest,
						int           size )
{
	int         bytesread;
	lumpinfo_t* l;
	FILE*       handle;
#ifdef PARANOIA
	if (lump<0) I_Error("W_ReadLumpHeader: lump not exist\n");

	if ((lump&0xFFFF) >= wadfiles[lump>>16]->numlumps)
		I_Error ("W_ReadLumpHeader: %i >= numlumps",lump);
#endif
	l = wadfiles[lump>>16]->lumpinfo + (lump&0xFFFF);

	// the good ole 'loading' disc icon TODO: restore it :)
	// ??? I_BeginRead ();

	// empty resource (usually markers like S_START, F_END ..)
	if (l->size==0)
		return 0;

	handle = wadfiles[lump>>16]->handle;

	// 0 size means read all the lump
	if (!size || size>l->size)
		size = l->size;

	fseek (handle, l->position, SEEK_SET);
	bytesread = fread (dest, 1, size, handle);

	// ??? I_EndRead ();
	return bytesread;
}


//
//  W_ReadLump
//  Loads the lump into the given buffer,
//   which must be >= W_LumpLength().
//
//added:06-02-98: now calls W_ReadLumpHeader() with full lump size.
//                0 size means the size of the lump, see W_ReadLumpHeader
void W_ReadLump ( int           lump,
				  void*         dest )
{
	W_ReadLumpHeader (lump, dest, 0);
}


// ==========================================================================
// W_CacheLumpNum
// ==========================================================================
void* W_CacheLumpNum ( int lump, int tag )
{
	lumpcache_t*  lumpcache;

	//SoM: 4/8/2000: Don't keep doing oporations to the lump variable!
	int           llump = lump & 0xffff;
	int           lfile = lump >> 16;

#ifdef PARANOIA
	// check return value of a previous W_CheckNumForName()
	//SoM: 4/8/2000: Do better checking. No more SIGSEGV's!
	if (lfile >= numwadfiles)
	  I_Error("W_CacheLumpNum: %i >= numwadfiles(%i)\n", lfile, numwadfiles);
	if (llump >= wadfiles[lfile]->numlumps)
	  I_Error ("W_CacheLumpNum: %i >= numlumps", llump);
	if(lump == -1)
	  I_Error ("W_CacheLumpNum: -1 passed!\n");
	if(llump < 0)
	  I_Error ("W_CacheLumpNum: %i < 0!\n", llump);
#endif

	lumpcache = wadfiles[lfile]->lumpcache;
	if (!lumpcache[llump]) {
		// read the lump in
		//CONS_Printf ("cache miss on lump %i\n",lump);
		Z_Malloc (W_LumpLength (lump), tag, &lumpcache[llump]);
		W_ReadLumpHeader (lump, lumpcache[llump], 0);   // read full
	}
	else {
		//CONS_Printf ("cache hit on lump %i\n",lump);
		Z_ChangeTag (lumpcache[llump],tag);
	}

	return lumpcache[llump];
}


// ==========================================================================
// W_CacheLumpName
// ==========================================================================
void* W_CacheLumpName ( char* name, int tag )
{
	return W_CacheLumpNum (W_GetNumForName(name), tag);
}



// ==========================================================================
//                                         CACHING OF GRAPHIC PATCH RESOURCES
// ==========================================================================

// Graphic 'patches' are loaded, and if necessary, converted into the format
// the most useful for the current rendermode. For software renderer, the
// graphic patches are kept as is. For the 3Dfx renderer, graphic patches
// are 'unpacked', and are kept into the cache in that unpacked format, the
// heap memory cache then act as a 'level 2' cache just after the graphics
// card memory.

//
// Cache a patch into heap memory, convert the patch format as necessary
//

// Software-only compile cache the data without conversion
#ifdef HWRENDER // not win32 only 19990829 by Kin

void* W_CachePatchNum ( int lump,int tag )
{
	GlidePatch_t*   grPatch;

	if( rendermode == render_soft )
		return W_CacheLumpNum(lump,tag);

// ------------------------------------------------------ accelereted RENDER

#ifdef PARANOIA
	// check the return value of a previous W_CheckNumForName()
	if ( ( lump==-1 ) ||
		 ((lump&0xFFFF) >= wadfiles[lump>>16]->numlumps) )
		I_Error ("W_CachePatchNum: %i >= numlumps", lump&0xffff);
#endif

	grPatch = &(wadfiles[lump>>16]->cache3Dfx[lump & 0xffff]);

	if( grPatch->mipmap.grInfo.data )
	{
		Z_ChangeTag (grPatch->mipmap.grInfo.data, tag);
	}
	else
	{   // first time init grPatch fields
		// we need patch w,h,offset,...
		// well this code will be executed latter in GetPatch, anyway
		// do it now ...
		patch_t *ptr = W_CacheLumpNum(grPatch->patchlump, PU_STATIC);
		HWR_Make3DfxPatch ( ptr, grPatch, &grPatch->mipmap );
		Z_Free (ptr);
		//Hurdler: why not do a Z_ChangeTag (grPatch->mipmap.grInfo.data, tag) here?
		//BP: mmm, yes we can...
	}

	// return GlidePatch_t, which can be casted to (patch_t) with valid patch header info
	return (void*)grPatch;
}


//
// W_CachePatchName
//
void* W_CachePatchName ( char*   name,
						 int     tag )
{
	return W_CachePatchNum (W_GetNumForName(name), tag);
}

#endif // HWRENDER Glide version


void W_LoadDehackedLumps( void )
{
	int cfile = 0;
	int clump = 0;

	while(cfile < numwadfiles)
	{
		clump = 0;
		DEHLUMP:
		clump = W_CheckNumForNamePwad("DEHACKED", cfile, clump);
		if(clump != -1)
		{
			DEH_LoadDehackedLump(clump);
			clump++;
			goto DEHLUMP;
		}
		cfile ++;
	}
}
