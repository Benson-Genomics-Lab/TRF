# TODO Handle debug/release build directories to not pollute source directory
# FIXME clean doesn't work

# use semantic versioning, please: https://semver.org/
VERSION       	= 4.10.0-rc.1
CFLAGS       	= -O2 -DVERSION=\"$(VERSION)\"
CFLAGS_DEBUG 	= -ggdb -DDEBUG
LDFLAGS      	= -lm
UNIX_DEFINES  	= -DUNIXCONSOLE
DOS_DEFINES		= -DWINDOWSCONSOLE
CFLAGS_LINUX64	= -m64
CFLAGS_LINUX32	= -m32
CFLAGS_WINCMD	= -mwin32 -mconsole
# CFLAGS_APPLE64	= -arch x86_64
# CFLAGS_APPLE32	= -arch i386
CFLAGS_APPLE    = -arch x86_64
C_SRCS        	= src/trf.c
TRF_SRCS		= src/tr30dat.c src/tr30dat.h src/trf.c src/trfclean.h src/trfrun.h
BUILD_DIR       = build/
BUILD_DIR_DEBUG = debug/
TRF_EXE			= $(BUILD_DIR)trf$(VERSION).$@.exe

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
