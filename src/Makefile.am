bin_PROGRAMS = trf
trf_SOURCES = trf.c
trf_DEPENDENCIES = tr30dat.c tr30dat.h trf.c trfclean.h trfrun.h
trf_CFLAGS = -fno-align-functions -fno-align-loops

install-exec-hook:
	$(LN_S) $(DESTDIR)$(bindir)/trf$(EXEEXT) \
	$(DESTDIR)$(bindir)/trf@PACKAGE_VERSION@.@TARGET_NAME@.exe

uninstall-hook:
	rm $(DESTDIR)$(bindir)/trf@PACKAGE_VERSION@.@TARGET_NAME@.exe