AC_ARG_ENABLE([carddav],
              AS_HELP_STRING([--enable-carddav=@<:@yes/no@:>@],[Enable CardDAV plugin @<:@default=yes@:>@]),
              [],
              [enable_carddav=yes])

AS_IF([test "x$enable_carddav" = "xyes"], [
    AC_CHECK_PROG(CURL_CONFIG, curl-config, "yes")
    AC_CHECK_LIB(curl, main, HAVE_CURL_LIB="yes")
    if test x"$CURL_CONFIG" = "xyes" -a x"$HAVE_CURL_LIB" = "xyes"; then
      CURL_CFLAGS="`curl-config --cflags`"
      CURL_LIBS="`curl-config --libs`"
      AC_DEFINE(HAVE_CURL, 1, [define if you have curl library])
      AC_SUBST(CURL_CFLAGS)
      AC_SUBST(CURL_LIBS)
      AC_SUBST(HAVE_CURL)
    else
      AC_ERROR([curl not found])
    fi

    if pkg-config --exists libxml-2.0 ; then
      XML2_LIBS=`pkg-config libxml-2.0 --libs`
      XML2_CFLAGS=`pkg-config libxml-2.0 --cflags`
      AC_DEFINE(HAVE_XML2, 1, [define if you have xml2 library])
      AC_SUBST(XML2_CFLAGS)
      AC_SUBST(XML2_LIBS)
      AC_SUBST(HAVE_XML2)
    else
      AC_ERROR([xml not found])
    fi


])

AM_CONDITIONAL([CARDDAV],[test "x$enable_carddav" = "xyes"])


with_dav_user_agent="Mozilla/5.0"
AC_ARG_WITH(useragent,
            AS_HELP_STRING([--with-useragent=USER_AGENT],
                           [User agent to be used for DAV request @<:@default="Mozilla/5.0"@:>@]
                           ),
            [
             with_dav_user_agent="$withval"
            ]
)
AC_DEFINE_UNQUOTED(DAV_USER_AGENT, "${with_dav_user_agent}", [DAV requests user agent])
AC_SUBST(DAV_USER_AGENT)
