## OpenAB C++: PIM helper Library

AC_PREREQ([2.59])
AC_INIT([OpenAB], [1.0.1], [mateuszx.potrola@intel.com],
        [OpenAB], [http://www.intel.com])

AM_INIT_AUTOMAKE([1.11 -Wall no-define])

AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_RANLIB
AC_PROG_CC
AC_PROG_INSTALL

# AM_PROG_AR

LT_PREREQ([2.4.2])
LT_INIT([dlopen])

OPENAB_CURRENT=0
OPENAB_REVISION=2
OPENAB_AGE=0

OPENABLOG_CURRENT=0
OPENABLOG_REVISION=2
OPENABLOG_AGE=0

OPENABPLUGIN_CURRENT=0
OPENABPLUGIN_REVISION=2
OPENABPLUGIN_AGE=0

AC_SUBST(OPENAB_CURRENT)
AC_SUBST(OPENAB_REVISION)
AC_SUBST(OPENAB_AGE)

AC_SUBST(OPENABLOG_CURRENT)
AC_SUBST(OPENABLOG_REVISION)
AC_SUBST(OPENABLOG_AGE)

AC_SUBST(OPENABPLUGIN_CURRENT)
AC_SUBST(OPENABPLUGIN_REVISION)
AC_SUBST(OPENABPLUGIN_AGE)

dnl  Checks for libraries.
# json-c
PKG_CHECK_MODULES([JSON], [json-c])

#pthread
AC_CHECK_LIB(pthread, pthread_attr_init, PTHREAD_LIBS="-lpthread")
AC_SUBST(PTHREAD_LIBS)

# Doxygen
AC_ARG_ENABLE([doc],
              AC_HELP_STRING([--enable-doc=@<:@yes/no@:>@],[Generate the documentation @<:@default=yes@:>@]),
              [],
              [enable_doc=yes])

AC_CHECK_PROGS([DOXYGEN], [doxygen])
if test -z "$DOXYGEN";
    then AC_MSG_WARN([Doxygen not found - continuing without Doxygen support])
    enable_doc=no
fi
AM_CONDITIONAL([HAVE_DOXYGEN],[test -n "$DOXYGEN" && test "x$enable_doc" = "xyes"])
AM_COND_IF([HAVE_DOXYGEN],[AC_CONFIG_FILES([doc/Doxyfile doc/Makefile])])

# Examples
AC_ARG_ENABLE([examples],
              AC_HELP_STRING([--enable-examples=@<:@yes/no@:>@],[Enable OpenAP examples @<:@default=yes@:>@]),
              [],
              [enable_examples=yes])
AS_IF([test "x$enable_examples" = "xyes"], [
    #popt
    PKG_CHECK_MODULES([POPT], [popt])
    AC_CONFIG_FILES([examples/Makefile])
])
AM_CONDITIONAL([BUILD_EXAMPLES],[test "x$enable_examples" = "xyes"])

# Unit tests
AC_ARG_ENABLE([tests],
              AC_HELP_STRING([--enable-tests=@<:@yes/no@:>@],[Enable OpenAP unit tests @<:@default=yes@:>@]),
              [],
              [enable_tests=yes])

#Google Test
AC_CHECK_PROG(GTEST_CONFIG, gtest-config, "yes")
AC_CHECK_LIB(gtest, main, HAVE_GTEST_LIB="yes")
if test x"$GTEST_CONFIG" = "xyes" -a x"$HAVE_GTEST_LIB" = "xyes"; then
  GTEST_FLAGS=`gtest-config --cppflags --cxxflags`
  GTEST_LIBS="`gtest-config --ldflags` -lgtest -lpthread"
  AC_DEFINE(HAVE_GTEST, 1, [define if you have google gtest library])
  AC_SUBST(GTEST_LIBS)
  AC_SUBST(GTEST_FLAGS)
  AC_SUBST(HAVE_GTEST)
else
  enable_tests=no
fi

AS_IF([test -n "$GTEST_CONFIG" && test "x$enable_tests" = "xyes"], [
  AC_CONFIG_FILES([tests/Makefile])
])
AM_CONDITIONAL([BUILD_TESTS],[test -n "$GTEST_CONFIG" && test "x$enable_tests" = "xyes"])

#Code coverage
AC_ARG_ENABLE([coverage],
              AC_HELP_STRING([--enable-coverage=@<:@yes/no@:>@], [Enable coverage measurements of unit tests @<:@default=no@:>@]),
              [],
              [enable_coverage=no])

AC_CHECK_PROGS([LCOV], [lcov])
AC_CHECK_PROGS([GENHTML], [genhtml])
if test "x$enable_coverage" = "xyes"; then
    if test -z "$LCOV"; then
        AC_MSG_WARN([Could not find lcov tool, disabling coverage])
        enable_coverage=no
    fi
    if test -z "$GENHTML"; then
        AC_MSG_WARN([Could not find genhtml tool, disabling coverage])
        enable_coverage=no
    fi

    if test "x$enable_coverage" = "xyes"; then
        changequote({,})
        CFLAGS=`echo "$CFLAGS" | $SED -e 's/-O[0-9]*//g'`
        changequote([,])

        COVERAGE_CFLAGS="--coverage"
        COVERAGE_LDFLAGS="-lgcov"
        AC_SUBST(COVERAGE_CFLAGS)
        AC_SUBST(COVERAGE_LDFLAGS)
    fi
fi
AM_CONDITIONAL([COVERAGE_ENABLED],[test "x$enable_coverage" = "xyes"])


#cppcheck
AC_CHECK_PROGS([CPPCHECK], [cppcheck])
AM_CONDITIONAL([HAVE_CPPCHECK],[test "$CPPCHECK"])

AC_ARG_ENABLE([profiling],
              AC_HELP_STRING([--enable-profiling=@<:@yes/no@:>@],[Enable GPerf support @<:@default=no@:>@]),
              [],
              [enable_profiling=no])

AS_IF([test "x$enable_profiling" = "xyes"], [
    AC_CHECK_PROGS([GPERF], [gperf])
    CFLAGS="-pg $CFLAGS"
    LDFLAGS="-pg $LDFLAGS"
])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
# AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MMAP
AC_CHECK_FUNCS([gettimeofday mkfifo munmap strerror])

dnl Configure plugins
m4_esyscmd(./configure-plugins.sh)

AC_CONFIG_FILES([
    Makefile
    src/Makefile
    src/oab.pc
    src/plugin/oab-plugin.pc
])

AC_CONFIG_LINKS([tests/test_photo.jpeg:tests/test_photo.jpeg])
AC_CONFIG_LINKS([tests/vcard_single.vcf:tests/vcard_single.vcf])
AC_CONFIG_LINKS([tests/vcard_multiple.vcf:tests/vcard_multiple.vcf])
AC_CONFIG_LINKS([tests/vcard_incorrect.vcf:tests/vcard_incorrect.vcf])
AC_CONFIG_LINKS([tests/vcard_misformatted.vcf:tests/vcard_misformatted.vcf])
AC_CONFIG_LINKS([tests/vcards/vcard_0.vcf:tests/vcards/vcard_0.vcf])
AC_CONFIG_LINKS([tests/vcards/vcard_1.vcf:tests/vcards/vcard_1.vcf])
AC_CONFIG_LINKS([tests/vcards_sync_initial.vcf:tests/vcards_sync_initial.vcf])
AC_CONFIG_LINKS([tests/vcards_sync_one_modified.vcf:tests/vcards_sync_one_modified.vcf])
AC_CONFIG_LINKS([tests/vcards_sync_one_removed.vcf:tests/vcards_sync_one_removed.vcf])
AC_CONFIG_LINKS([tests/obex_mock.py:tests/obex_mock.py])

CFLAGS=`echo "$CFLAGS" -Wall -Wextra -Werror`
AC_OUTPUT

echo "
$PACKAGE_NAME $PACKAGE_VERSION configuration:
-----------------------------
  Source code location:       ${srcdir}
  Host System Type:           ${host}
  Compiler:                   ${CC}
  Standard CFLAGS:            ${CFLAGS} ${ac_devel_default_warnings} ${JSON_CFLAGS} ${COVERAGE_CFLAGS}
  Libraries:                  ${JSON_LIBS} ${PTHREAD_LIBS} ${COVERAGE_LDFLAGS}
  Install path (prefix):      ${prefix}
  
  Google Contacts enabled:    ${enable_google_contacts}
  CardDAV enabled:            ${enable_carddav}
  EDS Backend enabled:        ${enable_eds_backend}
  Documentation enabled:      ${enable_doc}
  Examples enabled:           ${enable_examples}
  Unit tests enabled:         ${enable_tests}
  Coverage measurement enabled:
                              ${enable_coverage}
  Profiling enabled:          ${enable_profiling}
  
  Now type 'make' to build $PACKAGE_NAME $PACKAGE_VERSION.
"
