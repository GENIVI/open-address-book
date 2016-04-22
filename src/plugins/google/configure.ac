AC_ARG_ENABLE([google-contacts],
              AC_HELP_STRING([--enable-google-contacts=@<:@yes/no@:>@],[Enable Google Contacts plugin (using GData) @<:@default=no@:>@]),
              [],
              [enable_google_contacts=no])

AS_IF([test "x$enable_google_contacts" = "xyes"], [
    PKG_CHECK_MODULES([GDATA],[libgdata])
    PKG_CHECK_MODULES([SOUP],[libsoup-2.4])
])

AM_CONDITIONAL([GOOGLE_CONTACTS],[test "x$enable_google_contacts" = "xyes"])