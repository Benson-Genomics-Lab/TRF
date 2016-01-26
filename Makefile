VERSION       	= 409
C_SRCS        	= trf.c
C_FLAGS       	= -O2
C_FLAGS_DEBUG 	= -O2 -ggdb -DDEBUG
LD_FLAGS      	= -lm
UNIX_DEFINES  	= -DUNIXCONSOLE
WIN_DEFINES   	= -DWINDOWSGUI
DOS_DEFINES     = -DWINDOWSCONSOLE
C_FLAGS_LINUX64 = -m64
C_FLAGS_LINUX32 = -m32
C_FLAGS_WINGUI  = -mwin32 -mwindows
C_FLAGS_WINCMD  = -mwin32 -mconsole
C_FLAGS_APPLE   = -arch x86_64 -arch i386 -arch ppc64 -arch ppc
TRF_SRCS      	= tr30dat.c tr30dat.h trf.c trfclean.h trfrun.h
TRF_EXE       	= trf$(VERSION).$@.exe
WINGUI_CHECK    = win32-gui/trf.c \
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

linux64: $(TRF_SRCS)
	gcc $(C_FLAGS) $(C_FLAGS_LINUX64) $(UNIX_DEFINES) -o $(TRF_EXE) $(C_SRCS) $(LD_FLAGS)

linux32: $(TRF_SRCS)
	gcc $(C_FLAGS) $(C_FLAGS_LINUX32) $(UNIX_DEFINES) -o $(TRF_EXE) $(C_SRCS) $(LD_FLAGS)

win64: $(TRF_SRCS)
	x86_64-w64-mingw32-gcc $(C_FLAGS) $(C_FLAGS_WINGUI) $(WIN_DEFINES) -o $(TRF_EXE) $(C_SRCS) $(LD_FLAGS)

win32: $(TRF_SRCS)
	i686-pc-mingw32-gcc $(C_FLAGS) $(C_FLAGS_WINGUI) $(WIN_DEFINES) -o $(TRF_EXE) $(C_SRCS) $(LD_FLAGS)

dos64: $(TRF_SRCS)
	x86_64-w64-mingw32-gcc $(C_FLAGS) $(C_FLAGS_WINCMD) $(DOS_DEFINES) -o $(TRF_EXE) $(C_SRCS) $(LD_FLAGS)

dos32: $(TRF_SRCS)
	i686-pc-mingw32-gcc $(C_FLAGS) $(C_FLAGS_WINCMD) $(DOS_DEFINES) -o $(TRF_EXE) $(C_SRCS) $(LD_FLAGS)

mac: $(TRF_SRCS)
	gcc $(C_FLAGS) $(C_FLAGS_APPLE) $(UNIX_DEFINES) -o $(TRF_EXE) $(C_SRCS) $(LD_FLAGS)

win_gui: trf.c trf.h trfdlg.h trfcomm.h trffile.h trfrun.h trfini.h tr30dat.c tr30dat.h trfclean.h dirdlg.h
win_res: trf.h trf.ico toolbar.bmp trf.rc

clean: *.o trf*.exe
	rm *.o trf*.exe
