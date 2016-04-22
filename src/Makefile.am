## OpenAB C++: PIM helper Library

AUTOMAKE_OPTIONS = subdir-objects

ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

lib_LTLIBRARIES = libOpenAB.la

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = oab.pc

libOpenAB_includedir = $(includedir)/OpenAB

nobase_libOpenAB_include_HEADERS = \
     plugin/storage/Storage.hpp \
     plugin/storage/StorageItem.hpp \
     plugin/source/Source.hpp \
     plugin/sync/Sync.hpp \
     plugin/Plugin.hpp \
     plugin/GenericParameters.hpp \
     PIMItem/PIMItem.hpp \
     PIMItem/PIMItemIndex.hpp \
     PIMItem/Contact/PIMContactItem.hpp \
     PIMItem/Contact/PIMContactItemIndex.hpp \
     PIMItem/Calendar/PIMCalendarItem.hpp \
     PIMItem/Calendar/PIMCalendarItemIndex.hpp \
     helpers/Variant.hpp \
     helpers/PluginManager.hpp \
     helpers/PluginManagerTemplates.hpp \
     helpers/SmartPtr.hpp \
     helpers/SecureString.hpp \
     helpers/Log.hpp \
     helpers/StringHelper.hpp \
     helpers/TimeStamp.hpp \
     OpenAB.hpp

libOpenAB_la_SOURCES = \
	OpenAB_Init.cpp \
	helpers/Variant.cpp \
	helpers/PluginManager.cpp \
	helpers/SecureString.cpp \
	helpers/Log.cpp \
	helpers/StringHelper.cpp \
	helpers/TimeStamp.cpp \
	PIMItem/PIMItemIndex.cpp \
	PIMItem/Contact/PIMContactItem.cpp \
	PIMItem/Contact/PIMContactItemIndex.cpp \
	PIMItem/Contact/Pict.cpp \
	PIMItem/Calendar/PIMCalendarItem.cpp \
	PIMItem/Calendar/PIMCalendarItemIndex.cpp \
	plugin/Plugin.hpp \
	plugin/GenericParameters.cpp \
	plugin/source/Source.cpp \
	plugin/storage/Storage.cpp \
	plugin/storage/ContactsStorage.cpp \
	plugin/storage/CalendarStorage.cpp \
	plugin/sync/Sync.cpp

libOpenAB_la_CPPFLAGS = -I$(top_srcdir)/src -DPKGDIR=\"$(pkglibdir)\/\" ${CFLAGS} $(JSON_CFLAGS) $(COVERAGE_CFLAGS)
libOpenAB_la_LDFLAGS = -version-info $(OPENAB_CURRENT):$(OPENAB_REVISION):$(OPENAB_AGE) $(PTHREAD_LIBS) $(JSON_LIBS) $(COVERAGE_LDFLAGS)

if CARDDAV
libOpenAB_la_SOURCES += helpers/Http.cpp \
                     helpers/OAuth2HttpAuthorizer.cpp \
                     helpers/BasicHttpAuthorizer.cpp

nobase_libOpenAB_include_HEADERS += helpers/Http.hpp \
                                 helpers/OAuth2HttpAuthorizer.hpp \
                                 helpers/BasicHttpAuthorizer.hpp

libOpenAB_la_CPPFLAGS += $(CURL_CFLAGS)
libOpenAB_la_LDFLAGS += $(CURL_LIBS)
endif

PLUGIN_FLAGS = -module -export-dynamic -avoid-version

pkgconfig_DATA += plugin/oab-plugin.pc
pkglib_LTLIBRARIES = 

include $(top_srcdir)/src/plugins/plugins.am
