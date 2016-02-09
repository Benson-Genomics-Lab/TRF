# TODO Handle debug/release build directories to not pollute source directory
# TODO Windows GUI version (maybe move to something like Qt to be cross-platform?)
# NOTE Redefine CFLAGS in the make command to set MAXWRAPLENGTHCONST
#    ex: make CFLAGS="-02 -DMAXWRAPLENGTHCONST=20000000"

VERSION       	= 409
C_SRCS        	= trf.c
CFLAGS       	= -O2
CFLAGS_DEBUG 	= -ggdb -DDEBUG
LDFLAGS      	= -lm
UNIX_DEFINES  	= -DUNIXCONSOLE
WIN_DEFINES   	= -DWINDOWSGUI
DOS_DEFINES		= -DWINDOWSCONSOLE
CFLAGS_LINUX64	= -m64
CFLAGS_LINUX32	= -m32
CFLAGS_WINGUI	= -mwin32 -mwindows
CFLAGS_WINCMD	= -mwin32 -mconsole
# CFLAGS_APPLE64	= -arch x86_64
# CFLAGS_APPLE32	= -arch i386
CFLAGS_APPLE    = -arch x86_64 -arch i386 -arch ppc64 -arch ppc
TRF_SRCS		= tr30dat.c tr30dat.h trf.c trfclean.h trfrun.h
TRF_EXE			= trf$(VERSION).$@.exe
WINGUI_CHECK	= win32-gui/trf.c \
				win32-gui/trf.h \
				win32-gui/trfdlg.h \
				win32-gui/trfcomm.h \
				win32-gui/trffile.h \
				win32-gui/trfrun.h \
				win32-gui/trfini.h \
				win32-gui/tr30dat.c \
				win32-gui/tr30dat.h \
				win32-gui/trfclean.h \
				win32-gui/dirdlg.h
WINGUI_SRCS     = win32-gui/trf.c
WINGUI_INCLUDE  = -Iwin32-gui -I.
BUILD_DIR       = build/
BUILD_DIR_DEBUG = debug/

all: linux win mac

linux: linux64 linux32

win: win64 win32

dos: dos64 dos32

# mac: mac64 mac32
mac: macuni

linux64: $(TRF_SRCS)
	gcc $(CFLAGS) $(CFLAGS_LINUX64) $(UNIX_DEFINES) -o $(TRF_EXE) $(C_SRCS) $(LDFLAGS)

linux32: $(TRF_SRCS)
	gcc $(CFLAGS) $(CFLAGS_LINUX32) $(UNIX_DEFINES) -o $(TRF_EXE) $(C_SRCS) $(LDFLAGS)

win64: $(TRF_SRCS)
	x86_64-w64-mingw32-gcc $(CFLAGS) $(CFLAGS_WINGUI) $(WIN_DEFINES) -o $(TRF_EXE) $(C_SRCS) $(LDFLAGS)

win32: $(TRF_SRCS)
	i686-w64-mingw32-gcc $(CFLAGS) $(CFLAGS_WINGUI) $(WIN_DEFINES) -o $(TRF_EXE) $(C_SRCS) $(LDFLAGS)

dos64: $(TRF_SRCS)
	x86_64-w64-mingw32-gcc $(CFLAGS) $(CFLAGS_WINCMD) $(DOS_DEFINES) -o $(TRF_EXE) $(C_SRCS) $(LDFLAGS)

dos32: $(TRF_SRCS)
	i686-w64-mingw32-gcc $(CFLAGS) $(CFLAGS_WINCMD) $(DOS_DEFINES) -o $(TRF_EXE) $(C_SRCS) $(LDFLAGS)

# mac64: $(TRF_SRCS)
# 	gcc $(CFLAGS) $(CFLAGS_APPLE64) $(UNIX_DEFINES) -o $(TRF_EXE) $(C_SRCS) $(LDFLAGS)

# mac32: $(TRF_SRCS)
# 	gcc $(CFLAGS) $(CFLAGS_APPLE32) $(UNIX_DEFINES) -o $(TRF_EXE) $(C_SRCS) $(LDFLAGS)

macuni: $(TRF_SRCS)
	gcc $(CFLAGS) $(CFLAGS_APPLE) $(UNIX_DEFINES) -o $(TRF_EXE) $(C_SRCS) $(LDFLAGS)

win_gui: trf.c trf.h trfdlg.h trfcomm.h trffile.h trfrun.h trfini.h tr30dat.c tr30dat.h trfclean.h dirdlg.h
win_res: trf.h trf.ico toolbar.bmp trf.rc

clean: *.o trf*.exe debug build
	rm -r *.o trf*.exe debug build
