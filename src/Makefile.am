# TODO Figure out best way to compile the different units together
AM_CFLAGS = -fno-align-functions -fno-align-loops
# noinst_LIBRARIES = libtrfrun.a
# libtrfrun_a_SOURCES = tr30dat.c trfrun.h trfclean.h tr30dat.h

bin_PROGRAMS = trf
trf_SOURCES = trf.c tr30dat.c trfrun.c trfrun.h trfclean.c trfclean.h tr30dat.h
# trf_LDADD = libtrfrun.a

install-exec-hook:
	$(LN_S) $(DESTDIR)$(bindir)/trf$(EXEEXT) \
	$(DESTDIR)$(bindir)/trf@PACKAGE_VERSION@.@TARGET_NAME@.exe

uninstall-hook:
	rm $(DESTDIR)$(bindir)/trf@PACKAGE_VERSION@.@TARGET_NAME@.exe