all: linux win

linux: tr30dat.c tr30dat.h trf.c trfclean.h trfrun.h
	gcc -O2 -m64 -D UNIXCONSOLE -o trf409-ngs.linux.exe trf.c -lm

win: tr30dat.c tr30dat.h trf.c trfclean.h trfrun.h
	x86_64-w64-mingw32-gcc -O2 -mwin32 -mconsole -D WINDOWSCONSOLE -o trf409-ngs.win.exe trf.c -lm
	#x86_64-w64-mingw32-gcc -O2 -mwin32 -mconsole -o trf409-ngs.win.mw64.exe trf.c -lm

clean: *.o trf*.exe
	rm *.o trf*.exe
