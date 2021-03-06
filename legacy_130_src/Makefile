
#     GNU Make makefile for SRB2
#############################################################################
# Copyright (C) 1998-2000 by DooM Legacy Team.
# Copyright (C) 2003-2020 by Sonic Team Junior.
#
# This program is free software distributed under the
# terms of the GNU General Public License, version 2.
# See the 'LICENSE' file for more details.
#
#     -DLINUX     -> use for the GNU/Linux specific
#     -D_WINDOWS  -> use for the Win32/DirectX specific
#     -DHAVE_SDL  -> use for the SDL interface
#
# Sets:
#     Compile the SDL/Mingw version with 'make MINGW=1'
#     Compile the SDL/Linux version with 'make LINUX=1'
#     Compile the SDL/Solaris version with 'make SOLARIS=1'
#     Compile the SDL/FreeBSD version with 'gmake FREEBSD=1'
#     Compile the SDL/Cygwin version with 'make CYGWIN32=1'
#     Compile the SDL/other version try with 'make SDL=1'
#
# 'Targets':
#     clean
#       Remove all object files
#     cleandep
#       Remove depend.dep
#
#
# Addon:
#     To Cross-Compile, CC=gcc-version make * PREFIX=<dir>
#     Compile with GCC 2.97 version, add 'GCC29=1'
#     Compile with GCC 4.0x version, add 'GCC40=1'
#     Compile with GCC 4.1x version, add 'GCC41=1'
#     Compile with GCC 4.2x version, add 'GCC42=1'
#     Compile with GCC 4.3x version, add 'GCC43=1'
#     Compile with GCC 4.4x version, add 'GCC44=1'
#     Compile with GCC 4.5x version, add 'GCC45=1'
#     Compile with GCC 4.6x version, add 'GCC46=1'
#     Compile a profile version, add 'PROFILEMODE=1'
#     Compile a debug version, add 'DEBUGMODE=1'
#     Compile with less warnings, add 'RELAXWARNINGS=1'
#     Generate compiler errors for most compiler warnings, add 'ERRORMODE=1'
#     Compile without NASM's tmap.nas, add 'NOASM=1'
#     Compile without 3D hardware support, add 'NOHW=1'
#     Compile with GDBstubs, add 'RDB=1'
#     Compile without PNG, add 'NOPNG=1'
#     Compile without zlib, add 'NOZLIB=1'
#
# Addon for SDL:
#     To Cross-Compile, add 'SDL_CONFIG=/usr/*/bin/sdl-config'
#     Compile without SDL_Mixer, add 'NOMIXER=1'
#     Compile without SDL_Mixer_X, add 'NOMIXERX=1' (Win32 only)
#     Compile without GME, add 'NOGME=1'
#     Compile without libopenmpt, add 'NOOPENMPT=1'
#     To link with the whole SDL_Image lib to load Icons, add 'SDL_IMAGE=1' but it isn't not realy needed
#     To link with SDLMain to hide console or make on a console-less binary, add 'SDLMAIN=1'
#
#############################################################################

ALL_SYSTEMS=\
	PANDORA\
	LINUX64\
	MINGW64\
	HAIKU\
	DUMMY\
	DJGPPDOS\
	MINGW\
	UNIX\
	LINUX\
	SOLARIS\
	FREEBSD\
	MACOSX\
	SDL\

# check for user specified system
ifeq (,$(filter $(ALL_SYSTEMS),$(.VARIABLES)))
ifeq ($(OS),Windows_NT) # all windows are Windows_NT...

 $(info Detected a Windows system, compiling for 32-bit MinGW SDL2...)

 # go for a 32-bit sdl mingw exe by default
 MINGW=1
 WINDOWSHELL=1

else # if you on the *nix

 system:=$(shell uname -s)

 ifeq ($(system),Linux)
 new_system=LINUX
 else

 $(error \
	 Could not automatically detect your system,\
	 try specifying a system manually)

 endif

 ifeq ($(shell getconf LONG_BIT),64)
 system+=64-bit
 new_system:=$(new_system)64
 endif

 $(info Detected $(system) ($(new_system))...)
 $(new_system)=1

endif
endif


# SRB2 data files
D_DIR?=../bin/Resources
D_FILES=$(D_DIR)/srb2.pk3 \
	$(D_DIR)/player.dta \
	$(D_DIR)/zones.pk3 \
	$(D_DIR)/music.dta \

PKG_CONFIG?=pkg-config

ifdef PANDORA
LINUX=1
endif

ifdef LINUX64
LINUX=1
NONX86=1
# LINUX64 does not imply X86_64=1; could mean ARM64 or Itanium
endif

ifdef MINGW64
MINGW=1
NONX86=1
NOASM=1
# MINGW64 should not necessarily imply X86_64=1, but we make that assumption elsewhere
# Once that changes, remove this
X86_64=1
endif #ifdef MINGW64

ifdef HAIKU
SDL=1
endif

include Makefile.cfg

ifdef DUMMY
NOPNG=1
NOZLIB=1
NOHW=1
NOASM=1
EXENAME?=srb2dummy
OBJS=$(OBJDIR)/i_video.o
LIBS=-lm
endif

ifdef HAIKU
NOASM=1
LIBS=-lnetwork
CFLAGS+=-DUNIXCOMMON
PNG_CFLAGS?=
PNG_LDFLAGS?=-lpng
endif

ifdef PANDORA
NONX86=1
NOHW=1
endif

ifndef NOOPENMPT
HAVE_OPENMPT=1
endif

ifdef MINGW
include win32/Makefile.cfg
endif #ifdef MINGW

ifdef UNIX
UNIXCOMMON=1
endif

ifdef LINUX
UNIXCOMMON=1
ifndef NOGME
HAVE_LIBGME=1
endif
endif

ifdef SOLARIS
UNIXCOMMON=1
endif

ifdef FREEBSD
UNIXCOMMON=1
endif

ifdef MACOSX
UNIXCOMMON=1
endif

ifdef SDL
	include sdl/Makefile.cfg
endif #ifdef SDL

ifdef DISTCC
        CC:=distcc $(CC)
endif

ifdef CCACHE
        CC:=ccache $(CC)
endif

MSGFMT?=msgfmt

ifndef ECHO
	NASM:=@$(NASM)
	REMOVE:=@$(REMOVE)
	CC:=@$(CC)
	CXX:=@$(CXX)
	OBJCOPY:=@$(OBJCOPY)
	OBJDUMP:=@$(OBJDUMP)
	STRIP:=@$(STRIP)
	WINDRES:=@$(WINDRES)
	MKDIR:=@$(MKDIR)
	GZIP:=@$(GZIP)
	MSGFMT:=@$(MSGFMT)
	UPX:=@$(UPX)
	UPX_OPTS+=-q
endif

ifdef NOHERITAGE
	OPTS+=-DNOHERITAGE
endif

ifdef NOHW
	OPTS+=-DNOHW
else
	OPTS+=-DHWRENDER
	OBJS+=$(OBJDIR)/hw_bsp.o $(OBJDIR)/hw_draw.o $(OBJDIR)/hw_light.o \
		 $(OBJDIR)/hw_main.o $(OBJDIR)/hw_md2.o $(OBJDIR)/hw_cache.o
endif

OPTS += -DCOMPVERSION

ifndef NONX86
ifndef GCC29
	ARCHOPTS?=-msse3 -mfpmath=sse
else
	ARCHOPTS?=-mpentium
endif
else
ifdef X86_64
	ARCHOPTS?=-march=nocona
endif
endif

ifndef NOASM
ifndef NONX86
	OBJS+=$(OBJDIR)/tmap.o $(OBJDIR)/tmap_nasm.o
	OPTS+=-DUSEASM
endif
endif

ifndef NOPNG
OPTS+=-DHAVE_PNG

ifdef PNG_PKGCONFIG
PNG_CFLAGS?=$(shell $(PKG_CONFIG) $(PNG_PKGCONFIG) --cflags)
PNG_LDFLAGS?=$(shell $(PKG_CONFIG) $(PNG_PKGCONFIG) --libs)
else
ifdef PREFIX
PNG_CONFIG?=$(PREFIX)-libpng-config
else
PNG_CONFIG?=libpng-config
endif

ifdef PNG_STATIC
PNG_CFLAGS?=$(shell $(PNG_CONFIG) --static --cflags)
PNG_LDFLAGS?=$(shell $(PNG_CONFIG) --static --ldflags)
else
PNG_CFLAGS?=$(shell $(PNG_CONFIG) --cflags)
PNG_LDFLAGS?=$(shell $(PNG_CONFIG) --ldflags)
endif
endif

ifdef LINUX
PNG_CFLAGS+=-D_LARGEFILE64_SOURCE
endif

LIBS+=$(PNG_LDFLAGS)
CFLAGS+=$(PNG_CFLAGS)

OBJS+=$(OBJDIR)/apng.o
endif

ifdef HAVE_LIBGME
OPTS+=-DHAVE_LIBGME

LIBGME_PKGCONFIG?=libgme
LIBGME_CFLAGS?=$(shell $(PKG_CONFIG) $(LIBGME_PKGCONFIG) --cflags)
LIBGME_LDFLAGS?=$(shell $(PKG_CONFIG) $(LIBGME_PKGCONFIG) --libs)

LIBS+=$(LIBGME_LDFLAGS)
CFLAGS+=$(LIBGME_CFLAGS)
endif

ifdef HAVE_OPENMPT
OPTS+=-DHAVE_OPENMPT

LIBOPENMPT_PKGCONFIG?=libopenmpt
LIBOPENMPT_CFLAGS?=$(shell $(PKG_CONFIG) $(LIBOPENMPT_PKGCONFIG) --cflags)
LIBOPENMPT_LDFLAGS?=$(shell $(PKG_CONFIG) $(LIBOPENMPT_PKGCONFIG) --libs)

LIBS+=$(LIBOPENMPT_LDFLAGS)
CFLAGS+=$(LIBOPENMPT_CFLAGS)
endif

ifndef NOZLIB
OPTS+=-DHAVE_ZLIB
ZLIB_PKGCONFIG?=zlib
ZLIB_CFLAGS?=$(shell $(PKG_CONFIG) $(ZLIB_PKGCONFIG) --cflags)
ZLIB_LDFLAGS?=$(shell $(PKG_CONFIG) $(ZLIB_PKGCONFIG) --libs)

LIBS+=$(ZLIB_LDFLAGS)
CFLAGS+=$(ZLIB_CFLAGS)
else
NOPNG=1
endif

ifdef STATIC
LIBS:=-static $(LIBS)
endif

ifdef NOMD5
	OPTS+=-DNOMD5
else
	OBJS:=$(OBJDIR)/md5.o $(OBJS)
endif

ifdef NOPOSTPROCESSING
	OPTS+=-DNOPOSTPROCESSING
endif

	OPTS:=-fno-exceptions $(OPTS)

ifdef MOBJCONSISTANCY
	OPTS+=-DMOBJCONSISTANCY
endif

ifdef PACKETDROP
	OPTS+=-DPACKETDROP
endif

ifdef DEBUGMODE

	# build with debugging information
	WINDRESFLAGS = -D_DEBUG
ifdef GCC48
	CFLAGS+=-Og
else
	CFLAGS+=-O0
endif
	CFLAGS+= -Wall -DPARANOIA -DRANGECHECK -DPACKETDROP -DMOBJCONSISTANCY
else


	# build a normal optimised version
	WINDRESFLAGS = -DNDEBUG
	CFLAGS+=-O3
endif
	CFLAGS+=-g $(OPTS) $(ARCHOPTS) $(WINDRESFLAGS)

ifdef YASM
ifdef STABS
	NASMOPTS?= -g stabs
else
	NASMOPTS?= -g dwarf2
endif
else
	NASMOPTS?= -g
endif

ifdef PROFILEMODE
	# build with profiling information
	CFLAGS+=-pg
	LDFLAGS+=-pg
endif

ifdef ZDEBUG
	CPPFLAGS+=-DZDEBUG
endif

OPTS+=$(CPPFLAGS)

# default EXENAME if all else fails
EXENAME?=srb2
DBGNAME?=$(EXENAME).debug

# not too sophisticated dependency
OBJS:=$(i_main_o) \
		$(OBJDIR)/comptime.o \
		$(OBJDIR)/string.o   \
		$(OBJDIR)/filesrch.o \
		$(OBJDIR)/dstrings.o \
		$(OBJDIR)/tables.o	 \
		$(OBJDIR)/f_finale.o \
		$(OBJDIR)/f_wipe.o	 \
		$(OBJDIR)/d_main.o	 \
		$(OBJDIR)/d_net.o	 \
		$(OBJDIR)/d_items.o	 \
		$(OBJDIR)/g_game.o	 \
		$(OBJDIR)/m_menu.o	 \
		$(OBJDIR)/m_misc.o	 \
		$(OBJDIR)/m_argv.o	 \
		$(OBJDIR)/m_bbox.o	 \
		$(OBJDIR)/m_fixed.o	 \
		$(OBJDIR)/m_swap.o	 \
		$(OBJDIR)/m_cheat.o	 \
		$(OBJDIR)/m_random.o \
		$(OBJDIR)/am_map.o	 \
		$(OBJDIR)/p_ceilng.o \
		$(OBJDIR)/p_doors.o	 \
		$(OBJDIR)/p_enemy.o	 \
		$(OBJDIR)/p_floor.o	 \
		$(OBJDIR)/p_genlin.o \
		$(OBJDIR)/p_inter.o	 \
		$(OBJDIR)/p_lights.o \
		$(OBJDIR)/p_map.o	 \
		$(OBJDIR)/p_maputl.o \
		$(OBJDIR)/p_plats.o	 \
		$(OBJDIR)/p_pspr.o	 \
		$(OBJDIR)/p_setup.o	 \
		$(OBJDIR)/p_sight.o	 \
		$(OBJDIR)/p_spec.o	 \
		$(OBJDIR)/p_switch.o \
		$(OBJDIR)/p_mobj.o	 \
		$(OBJDIR)/p_telept.o \
		$(OBJDIR)/p_tick.o	 \
		$(OBJDIR)/p_saveg.o	 \
		$(OBJDIR)/p_user.o	 \
		$(OBJDIR)/r_bsp.o	 \
		$(OBJDIR)/r_data.o	 \
		$(OBJDIR)/r_draw.o	 \
		$(OBJDIR)/r_main.o	 \
		$(OBJDIR)/r_plane.o	 \
		$(OBJDIR)/r_segs.o	 \
		$(OBJDIR)/r_sky.o	 \
		$(OBJDIR)/r_things.o \
		$(OBJDIR)/w_wad.o	 \
		$(OBJDIR)/wi_stuff.o \
		$(OBJDIR)/v_video.o	 \
		$(OBJDIR)/st_lib.o	 \
		$(OBJDIR)/st_stuff.o \
		$(OBJDIR)/hu_stuff.o \
		$(OBJDIR)/s_sound.o	 \
		$(OBJDIR)/z_zone.o	 \
		$(OBJDIR)/info.o	 \
		$(OBJDIR)/sounds.o	 \
		$(OBJDIR)/p_fab.o	 \
		$(OBJDIR)/dehacked.o \
		$(qmus2mid_obj)      \
		$(OBJDIR)/g_input.o	 \
		$(OBJDIR)/screen.o	 \
		$(OBJDIR)/console.o	 \
		$(OBJDIR)/command.o	 \
		$(OBJDIR)/d_netcmd.o \
		$(OBJDIR)/d_clisrv.o \
		$(OBJDIR)/r_splats.o \
		$(OBJDIR)/g_state.o	 \
		$(OBJDIR)/d_netfil.o \
		$(OBJDIR)/i_tcp.o \
		$(OBJDIR)/m_anigif.o \
		$(i_net_o)      \
		$(i_system_o)   \
		$(i_sound_o)    \
		$(OBJS)


ifndef ECHO
ifndef NOECHOFILENAMES
define echoName =
	@echo -- $< ...
endef
endif
endif

# List of languages to compile.
# For reference, this is the command I use to build a srb2.pot file from the source code.
# (The listed source files are the ones containing translated strings).
# FILES=""; for file in `find ./ | grep "\.c" | grep -v svn`; do [ "`grep "M_GetText(" $file`" ] && FILES="$FILES $file"; done; xgettext -d srb2 -o locale/srb2.pot -kM_GetText -F --no-wrap $FILES
ifdef GETTEXT
POS:=$(BIN)/en.mo

OPTS+=-DGETTEXT
endif

ifdef PANDORA
all:	pre-build $(BIN)/$(PNDNAME)
endif


ifdef SDL
all:	 pre-build $(BIN)/$(EXENAME)
endif

ifdef DUMMY
all:	$(BIN)/$(EXENAME)
endif

cleandep:
	$(REMOVE) $(OBJDIR)/depend.dep
	$(REMOVE) comptime.h

pre-build:
ifdef WINDOWSHELL
	-..\comptime.bat .
else
	-@../comptime.sh .
endif

clean:
	$(REMOVE) *~ *.flc
	$(REMOVE) $(OBJDIR)/*.o

ifdef MINGW
	$(REMOVE) $(OBJDIR)/*.res
endif

ifdef CYGWIN32
	$(REMOVE) $(OBJDIR)/*.res
endif

#make a big srb2.s that is the disasm of the exe (dos only ?)
asm:
	$(CC) $(LDFLAGS) $(OBJS) -o $(OBJDIR)/tmp.exe $(LIBS)
	$(OBJDUMP) -d $(OBJDIR)/tmp.exe --no-show-raw-insn > srb2.s
	$(REMOVE) $(OBJDIR)/tmp.exe

# executable
# NOTE: DJGPP's objcopy do not have --add-gnu-debuglink

$(BIN)/$(EXENAME): $(POS) $(OBJS)
	-$(MKDIR) $(BIN)
	@echo Linking $(EXENAME)...
	$(LD) $(LDFLAGS) $(OBJS) -o $(BIN)/$(EXENAME) $(LIBS)
ifndef VALGRIND
ifndef NOOBJDUMP
	@echo Dumping debugging info
	$(OBJDUMP) $(OBJDUMP_OPTS) $(BIN)/$(EXENAME) > $(BIN)/$(DBGNAME).txt
ifdef WINDOWSHELL
	-$(GZIP) $(GZIP_OPTS) $(BIN)/$(DBGNAME).txt
else
	-$(GZIP) $(GZIP_OPT2) $(BIN)/$(DBGNAME).txt
endif
endif

# mac os x lsdlsrb2 does not like objcopy
ifndef MACOSX
	$(OBJCOPY) $(BIN)/$(EXENAME) $(BIN)/$(DBGNAME)
	$(OBJCOPY) --strip-debug $(BIN)/$(EXENAME)
	-$(OBJCOPY) --add-gnu-debuglink=$(BIN)/$(DBGNAME) $(BIN)/$(EXENAME)
endif
ifndef NOUPX
	-$(UPX) $(UPX_OPTS) $(BIN)/$(EXENAME)
endif
endif
	@echo Build is done, please look for $(EXENAME) in $(BIN), \(checking for post steps\)

reobjdump:
	@echo Redumping debugging info
	$(OBJDUMP) $(OBJDUMP_OPTS) $(BIN)/$(DBGNAME) > $(BIN)/$(DBGNAME).txt
ifdef WINDOWSHELL
	-$(GZIP) $(GZIP_OPTS) $(BIN)/$(DBGNAME).txt
else
	-$(GZIP) $(GZIP_OPT2) $(BIN)/$(DBGNAME).txt
endif

$(OBJDIR):
	-$(MKDIR) $(OBJDIR)

ifdef SDL
ifdef MINGW
$(OBJDIR)/r_opengl.o: hardware/r_opengl/r_opengl.c hardware/r_opengl/r_opengl.h \
 doomdef.h doomtype.h g_state.h m_swap.h hardware/hw_drv.h screen.h \
 command.h hardware/hw_data.h hardware/hw_defs.h \
 hardware/hw_md2.h hardware/hw_glob.h hardware/hw_main.h hardware/hw_light.h \
 am_map.h d_event.h d_player.h p_pspr.h m_fixed.h tables.h info.h d_think.h \
 p_mobj.h doomdata.h d_ticcmd.h r_defs.h
	$(echoName)
	$(CC) $(CFLAGS) $(WFLAGS) -c $< -o $@
else
$(OBJDIR)/r_opengl.o: hardware/r_opengl/r_opengl.c hardware/r_opengl/r_opengl.h \
 doomdef.h doomtype.h g_state.h m_swap.h hardware/hw_drv.h screen.h \
 command.h hardware/hw_data.h hardware/hw_defs.h \
 hardware/hw_md2.h hardware/hw_glob.h hardware/hw_main.h hardware/hw_light.h \
 am_map.h d_event.h d_player.h p_pspr.h m_fixed.h tables.h info.h d_think.h \
 p_mobj.h doomdata.h d_ticcmd.h r_defs.h
	$(echoName)
	$(CC) $(CFLAGS) $(WFLAGS) -I/usr/X11R6/include -c $< -o $@
endif
endif

#dependecy made by gcc itself !
$(OBJS):
ifndef DUMMY
-include $(OBJDIR)/depend.dep
endif

$(OBJDIR)/depend.dep:
	@echo "Creating dependency file, depend.dep"
	@echo > comptime.h
	-$(MKDIR) $(OBJDIR)
	$(CC) $(CFLAGS) -MM *.c > $(OBJDIR)/depend.ped
	$(CC) $(CFLAGS) -MM $(INTERFACE)/*.c >> $(OBJDIR)/depend.ped
ifndef NOHW
	$(CC) $(CFLAGS) -MM hardware/*.c >> $(OBJDIR)/depend.ped
endif
	@sed -e 's,\(.*\)\.o: ,$(subst /,\/,$(OBJDIR))\/&,g' < $(OBJDIR)/depend.ped > $(OBJDIR)/depend.dep
	$(REMOVE) $(OBJDIR)/depend.ped
	@echo "Created dependency file, depend.dep"

ifdef VALGRIND
$(OBJDIR)/z_zone.o: z_zone.c
	$(echoName)
	$(CC) $(CFLAGS) $(WFLAGS) -DHAVE_VALGRIND $(VALGRIND_CFLAGS) -c $< -o $@
endif

$(OBJDIR)/comptime.o: comptime.c pre-build
	$(echoName)
	$(CC) $(CFLAGS) $(WFLAGS) -c $< -o $@

$(BIN)/%.mo: locale/%.po
	-$(MKDIR) $(BIN)
	$(echoName)
	$(MSGFMT) -f -o $@ $<

$(OBJDIR)/%.o: %.c
	$(echoName)
	$(CC) $(CFLAGS) $(WFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(INTERFACE)/%.c
	$(echoName)
	$(CC) $(CFLAGS) $(WFLAGS) -c $< -o $@

ifdef MACOSX
$(OBJDIR)/%.o: sdl/macosx/%.c
	$(echoName)
	$(CC) $(CFLAGS) $(WFLAGS) -c $< -o $@
endif

$(OBJDIR)/%.o: hardware/%.c
	$(echoName)
	$(CC) $(CFLAGS) $(WFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.nas
	$(echoName)
	$(NASM) $(NASMOPTS) -o $@ -f $(NASMFORMAT) $<

$(OBJDIR)/vid_copy.o: vid_copy.s asm_defs.inc
	$(echoName)
	$(CC) $(OPTS) $(ASFLAGS) -x assembler-with-cpp -c $< -o $@

$(OBJDIR)/%.o: %.s
	$(echoName)
	$(CC) $(OPTS) -x assembler-with-cpp -c $< -o $@

$(OBJDIR)/SRB2.res: win32/Srb2win.rc win32/afxres.h win32/resource.h
	$(echoName)
	$(WINDRES) -i $< -O rc $(WINDRESFLAGS) --include-dir=win32 -o $@ -O coff


ifdef SDL
ifdef MINGW
$(OBJDIR)/win_dbg.o: win32/win_dbg.c
	$(echoName)
	$(CC) $(CFLAGS) $(WFLAGS) -c $< -o $@
endif
endif

#############################################################
#
#############################################################
