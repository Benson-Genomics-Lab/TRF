all: linux win mac

C_SRCS = tr30dat.c tr30dat.h trf.c trfclean.h trfrun.h

linux: linux64 linux32

win: win64 win32

dos: dos64 dos32

linux64: $(C_SRCS)
	gcc -O2 -m64 -D UNIXCONSOLE -o trf409-ngs.linux64.exe trf.c -lm

linux32: $(C_SRCS)
	gcc -O2 -m32 -D UNIXCONSOLE -o trf409-ngs.linux.exe trf.c -lm

win64: $(C_SRCS)
	x86_64-w64-mingw32-gcc -O2 -mwin32 -mwindows -D WINDOWSGUI -o trf409-ngs.win64.exe trf.c -lm

win32: $(C_SRCS)
	i686-pc-mingw32-gcc -O2 -mwin32 -mwindows -D WINDOWSGUI -o trf409-ngs.win.exe trf.c -lm

dos64: $(C_SRCS)
	x86_64-w64-mingw32-gcc -O2 -mwin32 -mconsole -D WINDOWSCONSOLE -o trf409-ngs.dos64.exe trf.c -lm

dos32: $(C_SRCS)
	i686-pc-mingw32-gcc -O2 -mwin32 -mconsole -D WINDOWSCONSOLE -o trf409-ngs.dos.exe trf.c -lm

mac: $(C_SRCS)
	gcc -O2 -arch x86_64 -arch i386 -arch ppc64 -arch ppc -D UNIXCONSOLE -o trf409-ngs.mac.exe trf.c -lm

clean: *.o trf*.exe
	rm *.o trf*.exe
