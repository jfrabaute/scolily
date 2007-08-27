AC_INIT(src/main.cpp)

PACKAGE=scolily
VERSION=0.4.1

AM_INIT_AUTOMAKE($PACKAGE, $VERSION)

AM_GNU_GETTEXT([external])

AC_PROG_CC
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_PROG_INSTALL


PKG_CHECK_MODULES([FFT], fftw3f , ,
        AC_MSG_ERROR(This program connot be build without fftw3f))

PKG_CHECK_MODULES([ALSA], alsa , ,
        AC_MSG_ERROR(This program connot be build without alsa))

AC_ARG_ENABLE(gui,AC_HELP_STRING([--disable-gui],[disable gui]),[enable_gui=$enableval],[enable_gui=yes])
if test "$enable_gui" = "yes" ; then
	PKG_CHECK_MODULES([GTK], [gtkmm-2.4], [
			AC_DEFINE(GUI_ENABLED, [1], [Define to 1 if GTK+-2.0 is present])
		], [
			AC_ERROR(GTK+-2.0 not found)
		])
	PKG_CHECK_MODULES(GTHREAD, gthread-2.0, , AC_MSG_ERROR(GTHREAD 2.0 not found))
fi

AC_OUTPUT(Makefile src/Makefile po/Makefile.in)

