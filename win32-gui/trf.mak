#-------------------------------------
#   TRF.MAK MAKE FILE FOR TRF.EXE
#-------------------------------------

trf.exe : trf.mak trf.obj trf.res
        link -OUT:trf.exe trf.obj trf.res user32.lib gdi32.lib comctl32.lib shell32.lib comdlg32.lib

trf.obj : trf.mak trf.c trf.h trfdlg.h trfcomm.h trffile.h trfrun.h trfini.h tr30dat.c tr30dat.h trfclean.h dirdlg.h
        cl -c /W3 /O2 /MT trf.c

trf.res : trf.h trf.ico toolbar.bmp trf.rc
        rc trf.rc
