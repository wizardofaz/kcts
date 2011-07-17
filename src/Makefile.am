# Copyright (c) 2008 Dave Freese, W1HKJ (w1hkj AT w1hkj DOT com)

bin_PROGRAMS = kcts

kcts_cxxFLAGS = @kcts_BUILD_cxxFLAGS@
kcts_CXXFLAGS = @kcts_BUILD_CXXFLAGS@
kcts_CFLAGS = $(kcts_CXXFLAGS)
kcts_LDFLAGS = @kcts_BUILD_LDFLAGS@
kcts_LDADD = @kcts_BUILD_LDADD@

MINGW32_SRC = \
	kcts.rc \
	include/kctsrc.h \
	mingw.c \
	include/compat.h \
	include/compat-mingw.h


# We distribute these but do not always compile them
EXTRA_kcts_SOURCES = $(kcts_WIN32_RES_SRC) $(MINGW32_SRC)

kcts_SOURCES =

if MINGW32
if HAVE_WINDRES
  kcts_SOURCES += $(MINGW32_SRC)
endif
endif

########################################################################


kcts_VERSION_MAJOR = @kcts_VERSION_MAJOR@
kcts_VERSION_MINOR = @kcts_VERSION_MINOR@
kcts_VERSION_PATCH = @kcts_VERSION_PATCH@
kcts_VERSION = @kcts_VERSION@

.EXPORT_ALL_VARIABLES: nsisinst appbundle

# Sources that are generated,
BUILT_SOURCES =
# not distributed,
nodist_kcts_SOURCES = $(BUILT_SOURCES)
# and deleted by the clean targets
CLEANFILES = $(BUILT_SOURCES)
CLEAN_LOCAL =

if MINGW32
if HAVE_WINDRES
.rc.o:
	$(WINDRES) -I$(srcdir)/include -I$(srcdir)/../data/win32 $< -O coff $@
endif
endif

install-data-local:
	if test -f $(srcdir)/../data/kcts.xpm; then \
	    $(mkinstalldirs) $(DESTDIR)/$(datadir)/pixmaps; \
	    $(INSTALL_DATA) $(srcdir)/../data/kcts.xpm $(DESTDIR)/$(datadir)/pixmaps; \
	fi
	if test -f $(srcdir)/../data/kcts.desktop; then \
	    $(mkinstalldirs) $(DESTDIR)/$(datadir)/applications; \
	    $(INSTALL_DATA) $(srcdir)/../data/kcts.desktop $(DESTDIR)/$(datadir)/applications; \
	fi

uninstall-local:
	rm -f $(DESTDIR)/$(datadir)/pixmaps/kcts.xpm
	rm -f $(DESTDIR)/$(datadir)/applications/kcts.desktop

FL_SRC = kctsdialog.fl

if HAVE_FLUID
flgen: $(FL_SRC)
	(cd $(srcdir)/include; \
	for f in $(FL_SRC); do \
	    c=$${f%.fl}.cxx; \
		h=$${f%.fl}.h; \
		h=$${h##*/}; \
	    $(FLUID) -c -o ../$$c -h $$h ../$$f; \
	done)
endif


INSTALLER_FILE = kcts-$(kcts_VERSION)_setup.exe
APPBUNDLE=kcts-$(kcts_VERSION)
APPBUNDLE_NOLIBS=$(APPBUNDLE)-nolibs

if HAVE_NSIS
nsisinst: $(bin_PROGRAMS)
	sh $(srcdir)/../scripts/mknsisinst.sh "$(srcdir)/../data" .
    CLEANFILES += $(INSTALLER_FILE)
endif

if DARWIN
appbundle: $(bin_PROGRAMS)
	sh $(srcdir)/../scripts/mkappbundle.sh "$(srcdir)/../data" .
    CLEAN_LOCAL += $(APPBUNDLE_NOLIBS) $(APPBUNDLE) $(APPBUNDLE)*.dmg
endif

#if HAVE_ASCIIDOC
#$(builddir)/../doc/guide.html: $(builddir)/../doc/guide.txt
#	$(MAKE) -C $(builddir)/../doc $(AM_MAKEFLAGS) guide.html

#$(builddir)/dialogs/guide.cxx: $(builddir)/../doc/guide.html
#	mkdir -p $(builddir)/dialogs
#	sed 's/"/\\"/g; s/$$/\\n\\/g; 1 s/.*/const char* szBeginner = "&/; $$ s/.*/&n";/' $< > $@
#dialogs/htmlstrings.cxx: $(builddir)/dialogs/guide.cxx

#distclean-local:
#	@if test "$(builddir)" != "$(srcdir)"; then \
#	    rm -f $(builddir)/dialogs/guide.cxx; \
#        fi
#endif

clean-local:
	-rm -rf $(CLEAN_LOCAL)

# Sources that we build. It is OK to have headers here.
kcts_SOURCES += \
	main.cxx \
	IOspec.cxx \
	kcts_io.cxx \
	kctsdialog.cxx \
	serialcomm.cxx \
	support.cxx \
	cstack.cxx \
	debug.cxx \
	icons.cxx \
	pixmaps.cxx \
	status.cxx \
	util.cxx \
	XYplot.cxx \
	include/cstack.h \
	include/debug.h \
	include/gettext.h \
	include/icons.h \
	include/pixmaps.h \
	include/IOspec.h \
	include/kcts.h \
	include/kcts_io.h \
	include/kctsdialog.h \
	include/kctsrc.h \
	include/serialcomm.h \
	include/support.h \
	include/status.h \
	include/util.h \
	include/XYplot.h 

# Additional source files that are distributed
EXTRA_DIST = \
	kcts.rc \
	kcts_icon.cxx \
	kctsdialog.fl 

# Additional non-source files that are distributed
# Additional source files that support non-Linux cross compilation
EXTRA_DIST += \
	$(srcdir)/../data/kcts.desktop \
	$(srcdir)/../data/kcts.xpm \
	$(srcdir)/../data/win32/fl_app.nsi \
	$(srcdir)/../data/win32/kcts.ico \
	$(srcdir)/../scripts/mknsisinst.sh \
	$(srcdir)/../scripts/build4mingw.sh \
	$(srcdir)/../data/mac/Info.plist.in \
	$(srcdir)/../data/mac/kcts.icns \
	$(srcdir)/../scripts/mkappbundle.sh \
	$(kcts_WIN32_SRC) \
	$(kcts_FL_SRC)
