# TODO Handle debug/release build directories to not pollute source directory
# TODO Windows GUI version (maybe move to something like Qt to be cross-platform?)
# FIXME clean doesn't work

VERSION       	= 409
C_SRCS        	= trf.c
CFLAGS       	= -O2
CFLAGS_DEBUG 	= -ggdb -DDEBUG
LDFLAGS      	= -lm
UNIX_DEFINES  	= -DUNIXCONSOLE
DOS_DEFINES		= -DWINDOWSCONSOLE
CFLAGS_LINUX64	= -m64
CFLAGS_LINUX32	= -m32
CFLAGS_WINGUI	= -mwin32 -mwindows
CFLAGS_WINCMD	= -mwin32 -mconsole
# CFLAGS_APPLE64	= -arch x86_64
# CFLAGS_APPLE32	= -arch i386
CFLAGS_APPLE    = -arch x86_64
TRF_SRCS		= tr30dat.c tr30dat.h trf.c trfclean.h trfrun.h
TRF_EXE			= trf$(VERSION).$@.exe
BUILD_DIR       = build/
BUILD_DIR_DEBUG = debug/

all: linux dos mac

linux: linux64 linux32

dos: dos64 dos32

# mac: mac64 mac32
# mac: macuni

linux64: $(TRF_SRCS)
	gcc $(CFLAGS) $(CFLAGS_LINUX64) $(UNIX_DEFINES) -o $(TRF_EXE) $(C_SRCS) $(LDFLAGS)

linux32: $(TRF_SRCS)
	gcc $(CFLAGS) $(CFLAGS_LINUX32) $(UNIX_DEFINES) -o $(TRF_EXE) $(C_SRCS) $(LDFLAGS)

dos64: $(TRF_SRCS)
	x86_64-w64-mingw32-gcc $(CFLAGS) $(CFLAGS_WINCMD) $(DOS_DEFINES) -o $(TRF_EXE) $(C_SRCS) $(LDFLAGS)

dos32: $(TRF_SRCS)
	i686-w64-mingw32-gcc $(CFLAGS) $(CFLAGS_WINCMD) $(DOS_DEFINES) -o $(TRF_EXE) $(C_SRCS) $(LDFLAGS)

# mac64: $(TRF_SRCS)
# 	gcc $(CFLAGS) $(CFLAGS_APPLE64) $(UNIX_DEFINES) -o $(TRF_EXE) $(C_SRCS) $(LDFLAGS)

# mac32: $(TRF_SRCS)
# 	gcc $(CFLAGS) $(CFLAGS_APPLE32) $(UNIX_DEFINES) -o $(TRF_EXE) $(C_SRCS) $(LDFLAGS)

mac: $(TRF_SRCS)
	gcc $(CFLAGS) $(CFLAGS_APPLE) $(UNIX_DEFINES) -o $(TRF_EXE) $(C_SRCS) $(LDFLAGS)

clean: *.o trf*.exe debug build
	rm -r *.o trf*.exe debug build
