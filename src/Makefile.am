bin_PROGRAMS = scolily

scolily_SOURCES = main.cpp scolily.cpp scolily.h record.cpp record.h \
                  ui.h mainwindow.cpp finish-dialog.cpp settings-dialog.cpp lilypond-internal.cpp 

plugindir = $(libdir)/scolily
plugin_LTLIBRARIES = libmidi.la libabc.la
libmidi_la_SOURCES = midi-plugin.cpp
libabc_la_SOURCES = abc-plugin.cpp

desktopdir = $(datadir)/applications
desktop_DATA = scolily.desktop

icondir = $(datadir)/pixmaps
icon_DATA = scolily.svg scolily-logo.svg

INCLUDES = @FFT_CFLAGS@ @ALSA_CFLAGS@ @GTK_CFLAGS@ @GTHREAD_CFLAGS@

LDADD = @FFT_LIBS@ @ALSA_LIBS@ @GTK_LIBS@ @GTHREAD_LIBS@

AM_CPPFLAGS = -DLOCALEDIR=\"$(datadir)/locale\" -DICONDIR=\"$(icondir)\" -DLIBDIR=\"$(libdir)/scolily\"

