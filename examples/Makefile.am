## OpenAB : PIM helper Library

AUTOMAKE_OPTIONS = subdir-objects

ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

bin_PROGRAMS = example_OpenAB_Sync_File
example_OpenAB_Sync_File_SOURCES = test_OpenAB_Sync_File.cpp
example_OpenAB_Sync_File_LDADD = ../src/libOpenAB.la -ldl
example_OpenAB_Sync_File_CPPFLAGS = -I$(top_srcdir)/src
example_OpenAB_Sync_File_LDFLAGS = -rdynamic -no-install

bin_PROGRAMS += example_OpenAB_Sync_PBAP
example_OpenAB_Sync_PBAP_SOURCES = test_OpenAB_Sync_PBAP.cpp
example_OpenAB_Sync_PBAP_LDADD = ../src/libOpenAB.la -ldl
example_OpenAB_Sync_PBAP_CPPFLAGS = -I$(top_srcdir)/src
example_OpenAB_Sync_PBAP_LDFLAGS = -rdynamic -no-install

bin_PROGRAMS += example_OpenAB_Sync
example_OpenAB_Sync_SOURCES = test_OpenAB_Sync.cpp
example_OpenAB_Sync_LDADD = ../src/libOpenAB.la -ldl $(POPT_LIBS)
example_OpenAB_Sync_CPPFLAGS = -I$(top_srcdir)/src $(POPT_CFLAGS)
example_OpenAB_Sync_LDFLAGS = -rdynamic -no-install

#bin_PROGRAMS = test_plugin_constructor
#test_plugin_constructor_SOURCES = test_plugin_constructor.cpp
#test_plugin_constructor_LDADD = ../src/libOpenAB.la ../src/libOpenAB_log.la -ldl
#test_plugin_constructor_CPPFLAGS = -I$(top_srcdir)/src
#test_plugin_constructor_LDFLAGS = -rdynamic -no-install

bin_PROGRAMS += test_plugin_factories
test_plugin_factories_SOURCES = test_plugin_factories.cpp
test_plugin_factories_LDADD = ../src/libOpenAB.la -ldl
test_plugin_factories_CPPFLAGS = -I$(top_srcdir)/src
test_plugin_factories_LDFLAGS = -rdynamic -no-install

bin_PROGRAMS += test_vcard_parsing
test_vcard_parsing_SOURCES = test_vcard_parsing.cpp
test_vcard_parsing_LDADD = ../src/libOpenAB.la -ldl
test_vcard_parsing_CPPFLAGS = -I$(top_srcdir)/src
test_vcard_parsing_LDFLAGS = -rdynamic -no-install

bin_PROGRAMS += test_ical_parsing
test_ical_parsing_SOURCES = test_icalendar_parsing.cpp
test_ical_parsing_LDADD = ../src/libOpenAB.la -ldl
test_ical_parsing_CPPFLAGS = -I$(top_srcdir)/src
test_ical_parsing_LDFLAGS = -rdynamic -no-install

bin_PROGRAMS += test_file_download
test_file_download_SOURCES = test_file_download.cpp
test_file_download_LDADD = ../src/libOpenAB.la -ldl
test_file_download_CPPFLAGS = -I$(top_srcdir)/src
test_file_download_LDFLAGS = -rdynamic -no-install

bin_PROGRAMS += test_EDS_download
test_EDS_download_SOURCES = test_EDS_download.cpp
test_EDS_download_LDADD = ../src/libOpenAB.la -ldl
test_EDS_download_CPPFLAGS = -I$(top_srcdir)/src
test_EDS_download_LDFLAGS = -rdynamic -no-install

bin_PROGRAMS += test_PBAP_download
test_PBAP_download_SOURCES = test_PBAP_download.cpp
test_PBAP_download_LDADD = ../src/libOpenAB.la -ldl
test_PBAP_download_CPPFLAGS = -I$(top_srcdir)/src
test_PBAP_download_LDFLAGS = -rdynamic -no-install

bin_PROGRAMS += test_google_download
test_google_download_SOURCES = test_google_download.cpp
test_google_download_LDADD = ../src/libOpenAB.la -ldl
test_google_download_CPPFLAGS = -I$(top_srcdir)/src
test_google_download_LDFLAGS = -rdynamic -no-install

bin_PROGRAMS += test_carddav_download
test_carddav_download_SOURCES = test_carddav_download.cpp
test_carddav_download_LDADD = ../src/libOpenAB.la -ldl
test_carddav_download_CPPFLAGS = -I$(top_srcdir)/src
test_carddav_download_LDFLAGS = -rdynamic -no-install
