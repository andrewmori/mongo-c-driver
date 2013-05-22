dnl **************************************************************************
dnl Versioning Macros
dnl **************************************************************************
m4_define([mongoc_major_version], [0])
m4_define([mongoc_minor_version], [1])
m4_define([mongoc_micro_version], [0])
m4_define([mongoc_version],
          [mongoc_major_version.mongoc_minor_version.mongoc_micro_version])
m4_define([mongoc_binary_age], [m4_eval(100 * mongoc_minor_version +
           mongoc_micro_version)])
m4_define([lt_current], [m4_eval(100 * mongoc_minor_version +
           mongoc_micro_version - mongoc_interface_age)])
m4_define([lt_revision], [mongoc_interface_age])
m4_define([lt_age], [m4_eval(mongoc_binary_age - mongoc_interface_age)])


dnl **************************************************************************
dnl Initialize Automake
dnl **************************************************************************
AC_PREREQ([2.64])
AC_INIT([mongoc],[mongoc_version],[https://github.com/chergert/libmongoc])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])
AS_AM_REALLY_SILENT

# Initialize libtool
LT_PREREQ([2.2])
LT_INIT

AM_INIT_AUTOMAKE([1.11.1 no-dist-gzip dist-xz])
AM_PROG_CC_C_O


dnl **************************************************************************
dnl Versioning Definitions
dnl **************************************************************************
MONGOC_MAJOR_VERSION=mongoc_major_version
MONGOC_MINOR_VERSION=mongoc_minor_version
MONGOC_MICRO_VERSION=mongoc_micro_version
MONGOC_API_VERSION=1.0
MONGOC_VERSION=mongoc_version
AC_SUBST(MONGOC_MAJOR_VERSION)
AC_SUBST(MONGOC_MINOR_VERSION)
AC_SUBST(MONGOC_MICRO_VERSION)
AC_SUBST(MONGOC_API_VERSION)
AC_SUBST(MONGOC_VERSION)


dnl **************************************************************************
dnl Check for Required Headers
dnl **************************************************************************
AC_NEED_STDINT_H(mongoc/mongoc-stdint.h)


dnl **************************************************************************
dnl Check for libbson # TODO: use git submodule as fallback
dnl **************************************************************************
PKG_CHECK_MODULES(BSON, libbson-1.0)


dnl **************************************************************************
dnl Check for Python Support
dnl **************************************************************************
AX_PYTHON


dnl **************************************************************************
dnl Enable extra debugging options
dnl **************************************************************************
m4_define([debug_default],
	  [m4_if(m4_eval(mongoc_minor_version % 2), [1],
	  [yes], [minimum])])
AC_ARG_ENABLE([debug],
	      [AS_HELP_STRING([--enable-debug=@<:@no/minimum/yes@:>@],
	      		      [turn on debugging @<:@default=debug_default@:>@])],
	      		      [],
	      		      [enable_debug=debug_default])
AS_CASE([$enable_debug],
	[yes], [
		CFLAGS="$CFLAGS -Wall"
		CFLAGS="$CFLAGS -Werror"
	],
	[minimum], [
		CFLAGS="$CFLAGS -Wold-style-definition"
		CFLAGS="$CFLAGS -Wdeclaration-after-statement"
		CFLAGS="$CFLAGS -Wredundant-decls"
		CFLAGS="$CFLAGS -Wmissing-noreturn"
		CFLAGS="$CFLAGS -Wcast-align"
		CFLAGS="$CFLAGS -Wwrite-strings"
		CFLAGS="$CFLAGS -Winline"
		CFLAGS="$CFLAGS -Wformat"
		CFLAGS="$CFLAGS -Wformat-nonliteral"
		CFLAGS="$CFLAGS -Wformat-security"
		CFLAGS="$CFLAGS -Wswitch-enum"
		CFLAGS="$CFLAGS -Wswitch-default"
		CFLAGS="$CFLAGS -Winit-self"
		CFLAGS="$CFLAGS -Wmissing-include-dirs"
		CFLAGS="$CFLAGS -Wundef"
		CFLAGS="$CFLAGS -Waggregate-return"
		CFLAGS="$CFLAGS -Wmissing-format-attribute"
		CFLAGS="$CFLAGS -Wnested-externs"
		CFLAGS="$CFLAGS -Wshadow"
		CFLAGS="$CFLAGS -Wreturn-type"
		CFLAGS="$CFLAGS -Wunused-but-set-parameter"
		CFLAGS="$CFLAGS -Wuninitialized"
	],
	[no], [
	],
	[AC_MSG_ERROR([Unknown argument to --enable-debug])]
)


dnl **************************************************************************
dnl Output
dnl **************************************************************************
AC_CONFIG_FILES([
	Makefile
	libmongoc-1.0.pc
	mongoc/mongoc-version.h
])
AC_OUTPUT


echo ""
echo "libmongoc-$VERSION"
echo ""
echo "  Prefix ....................: ${prefix}"
echo "  Debug Level ...............: ${enable_debug}"
echo "  CFLAGS ....................: ${CFLAGS}"
echo ""