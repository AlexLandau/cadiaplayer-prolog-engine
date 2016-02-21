#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(CADIAPlayer, 3.0, hif@ru.is)
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_SRCDIR([agent/cadiaplayer.cpp])
AC_PREFIX_DEFAULT([`pwd`])
AC_CONFIG_HEADER([config.h])

AC_HEADER_ASSERT

AC_ARG_ENABLE([eclipse],
  AS_HELP_STRING([--enable-eclipse],
    [enable the use of eclipse prolog for reasoning])
)

# Yap is still needed, even if disabled, because it is too deep in the system
AC_ARG_ENABLE([yap],
  AS_HELP_STRING([--disable-yap],
    [disable the use of yap prolog for reasoning (libYap.so is still needed, because Yap is too deep in the system)])
)

AC_ARG_ENABLE([debug],
  AS_HELP_STRING([--enable-debug],
    [compile with debugging symbols (use "-g" instead of "-O3" when compiling)])
)

case $OSTYPE in
	linux*)
		AC_DEFINE([LINUX], [1], [linux operating system])
	;;
	darwin*)
		AC_DEFINE([MAC_OSX], [1], [mac operating system])
esac


# Checks for programs.

AC_PROG_CXX
AC_PROG_CC
AC_PROG_LEX
AC_PROG_RANLIB

# Checks for libraries.

AX_BOOST_BASE
AX_BOOST_SYSTEM
AC_CHECK_LIB([gmp], [__gmpz_init], ,[AC_MSG_ERROR([GNU MP not found, see http://gmplib.org/])])
AC_CHECK_LIB([ncurses], [tputs], ,[AC_MSG_ERROR([GNU ncurses library not found])])
AC_CHECK_LIB([readline], [readline], ,[AC_MSG_ERROR([GNU readline library not found])])
AC_CHECK_LIB([m], [fetestexcept], ,[AC_MSG_ERROR([GNU math library (libm) not found])])
AC_CHECK_LIB([dl], [dlopen], ,[AC_MSG_ERROR([libdl not found])])

# choose the reasoning library
AS_IF([test "x$enable_eclipse" = "xyes"], [
	AC_CHECK_LIB([eclipse], [ec_init], ,[AC_MSG_ERROR([eclipse prolog library was not found])])
	have_eclipse=1
	AC_DEFINE([USE_ECLIPSE], [1], [use Eclipse prolog])
])
AM_CONDITIONAL([HAVE_ECLIPSE], [test "x$have_eclipse" = x1])

AC_CHECK_LIB([Yap], [YAP_MkVarTerm], , [AC_MSG_ERROR([Yap library was not found])])
AS_IF([test "x$enable_yap" != "xno"], [
	have_yap=1
	AC_DEFINE([USE_YAP], [1], [use Yap prolog])
])
AM_CONDITIONAL([HAVE_YAP], [test "x$have_yap" = x1])

AS_IF([test "x$enable_breakthrough" != "xno"], [
	have_breakthrough=1
	AC_DEFINE([USE_BREAKTHROUGH], [1], [use Breakthrough specific controller])
])
AM_CONDITIONAL([HAVE_BREAKTHROUGH], [test "x$have_breakthrough" = x1])

# GLPK Gnu Linear Programming Kit
#AC_CHECK_LIB([glpk], [glp_create_prob], ,[AC_MSG_ERROR([GNU LP Kit not found, see http://www.gnu.org/software/glpk])])


# Checks for header files.
AC_CHECK_HEADERS([fcntl.h inttypes.h memory.h stddef.h stdint.h stdlib.h string.h unistd.h])
AS_IF([test "x$enable_eclipse" = "xyes"], [
	AC_CHECK_HEADER([eclipse/eclipse.h], [], [AC_MSG_ERROR([Header eclipse/eclipse.h not found! Check your prolog install (howto_setup_eclipse_prolog.txt)!])])
])

# Checks for typedefs, structures, and compiler characteristics.

AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.

AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([dup2 memset mkdir pow socket sqrt strtol strtoul])

# Set flags

AC_SUBST(CPPFLAGS, ["$CPPFLAGS $BOOST_CPPFLAGS"])
AC_SUBST(LDFLAGS, ["$LDFLAGS $BOOST_LDFLAGS"])
LIBS="${LIBS} ${BOOST_SYSTEM_LIB}"

AS_IF([test "x$enable_debug" = "xyes"], [
	compile_opt="-g"
], [
	compile_opt="-O3"
])

AC_SUBST(CPPFLAGS, ["$CPPFLAGS -Wno-deprecated"])
AC_SUBST(CFLAGS, ["$CFLAGS $compile_opt"])
AC_SUBST(CXXFLAGS, ["$CXXFLAGS $compile_opt"])

case $OSTYPE in
        darwin*)
		AC_SUBST(CFLAGS, ["$CFLAGS $compile_opt"])
		AC_SUBST(CXXFLAGS, ["$CXXFLAGS $compile_opt"])
esac

#AC_CONFIG_FILES([Makefile
#                 agent/Makefile
#                 client/Makefile
#                 play/parsing/Makefile
#                 scripts/Makefile
#                 server/Makefile
#                 test/Makefile
#                 utils/Makefile])				 
#AC_CONFIG_FILES([Makefile
#				agent/Makefile
#				play/parsing/Makefile
#				scripts/Makefile
#				server/Makefile
#				test/Makefile
#				utils/Makefile])				 
AC_CONFIG_FILES([Makefile
				agent/Makefile
				play/parsing/Makefile
				scripts/Makefile
				server/Makefile
				utils/Makefile])				 
#AC_CONFIG_FILES([Makefile
#                 agent/Makefile
#                 play/parsing/Makefile
#                 scripts/Makefile
#                 server/Makefile
#                 utils/Makefile])
AC_OUTPUT
