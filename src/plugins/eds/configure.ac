AC_ARG_ENABLE([eds-backend],
              AC_HELP_STRING([--enable-eds-backend=@<:@yes/no@:>@],[Enable Evolution Data Server backend @<:@default=yes@:>@]),
              [],
              [enable_eds_backend=yes])

AS_IF([test "x$enable_eds_backend" = "xyes"], [
    PKG_CHECK_MODULES([EDS],[libebook-1.2 libedata-book-1.2 libebook-contacts-1.2 libecal-1.2])
    PKG_CHECK_MODULES([ICAL],[libical])
])

AM_CONDITIONAL([EDS_BACKEND],[test "x$enable_eds_backend" = "xyes"])