## OpenAB : PIM helper Library

AUTOMAKE_OPTIONS = subdir-objects

ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

TESTS = OpenAB_tests \
		OpenAB_Source_File_tests
		
pkgconfigdir = $(libdir)/pkgconfig

PLUGIN_FLAGS = -module -shared -export-dynamic -avoid-version

#1. Test plugin
#2. Wrong name for OpenAB plugin
#3. Plugin with unimplemented methods - linker error when loading
pkglib_LTLIBRARIES = \
		     libOpenAB_plugin_test.la \
		     wrong_name_for_OpenAB_plugin.la \
		     libOpenAB_plugin_unimplemented.la

libOpenAB_plugin_test_la_SOURCES = OpenAB/test_plugin.cpp
libOpenAB_plugin_test_la_CPPFLAGS = -I$(top_srcdir)/src
libOpenAB_plugin_test_la_LDFLAGS = $(PLUGIN_FLAGS)

wrong_name_for_OpenAB_plugin_la_SOURCES = OpenAB/test_plugin.cpp
wrong_name_for_OpenAB_plugin_la_CPPFLAGS = -I$(top_srcdir)/src
wrong_name_for_OpenAB_plugin_la_LDFLAGS = $(PLUGIN_FLAGS)

libOpenAB_plugin_unimplemented_la_SOURCES = OpenAB/unimplemented_plugin.cpp
libOpenAB_plugin_unimplemented_la_CPPFLAGS = -I$(top_srcdir)/src
libOpenAB_plugin_unimplemented_la_LDFLAGS = $(PLUGIN_FLAGS)

check_PROGRAMS = OpenAB_tests \
		 OpenAB_Source_File_tests \
		 OpenAB_Source_PBAP_tests
				 
OpenAB_tests_SOURCES = OpenAB/oab_tests_main.cpp \
					OpenAB/variant_tests.cpp \
					OpenAB/smart_ptr_tests.cpp \
					OpenAB/logger_tests.cpp \
					OpenAB/pim_contact_item_tests.cpp \
					OpenAB/pim_contact_item_index_tests.cpp \
					OpenAB/generic_params_tests.cpp \
					OpenAB/pluginManager_tests.cpp \
					OpenAB/strings_helper_tests.cpp \
					OpenAB/secure_string_tests.cpp \
					OpenAB/timestamp_tests.cpp \
					OpenAB/http_tests.cpp \
					OpenAB/sync_tests.cpp \
					OpenAB/CardDAVHelper_tests.cpp \
					OpenAB/CardDAVStorage_tests.cpp \
					../src/plugins/carddav/CardDAVHelper.cpp \
					../src/plugins/carddav/DAVHelper.cpp 
					
OpenAB_tests_CPPFLAGS = -I$(top_srcdir)/src $(GTEST_FLAGS) -DTESTING $(COVERAGE_CFLAGS) $(XML2_CFLAGS)
OpenAB_tests_LDADD = ../src/libOpenAB.la -ldl $(XML2_LIBS)
OpenAB_tests_LDFLAGS = -rdynamic -no-install $(GTEST_LIBS) -static $(COVERAGE_LDFLAGS)

OpenAB_Source_File_tests_SOURCES = plugins/Source/File/oab_source_file_tests_main.cpp \
				plugins/Source/File/oab_source_file_tests.cpp 
				
                             

OpenAB_Source_File_tests_CPPFLAGS = -I$(top_srcdir)/src $(GTEST_FLAGS) -DTESTING $(COVERAGE_CFLAGS)
OpenAB_Source_File_tests_LDADD = ../src/libOpenAB.la
OpenAB_Source_File_tests_LDFLAGS = -rdynamic -no-install $(GTEST_LIBS) $(COVERAGE_LDFLAGS)


OpenAB_Source_PBAP_tests_SOURCES = plugins/Source/PBAP/oab_source_pbap_tests_main.cpp \
				plugins/Source/PBAP/oab_source_pbap_tests.cpp

OpenAB_Source_PBAP_tests_CPPFLAGS = -I$(top_srcdir)/src $(GTEST_FLAGS) -DTESTING $(COVERAGE_CFLAGS) $(XML2_CFLAGS)
OpenAB_Source_PBAP_tests_LDADD = ../src/libOpenAB.la -ldl $(XML2_LIBS)
OpenAB_Source_PBAP_tests_LDFLAGS = -rdynamic -no-install $(GTEST_LIBS) $(COVERAGE_LDFLAGS)

check_PROGRAMS += OpenAB_Storage_CardDAV_tests
TESTS += OpenAB_Storage_CardDAV_tests

OpenAB_Storage_CardDAV_tests_SOURCES = plugins/Storage/DAV/CardDAVStorage_tests_main.cpp \
				    plugins/Storage/DAV/CardDAVStorage_tests.cpp

OpenAB_Storage_CardDAV_tests_CPPFLAGS = -I$(top_srcdir)/src $(GTEST_FLAGS) -DTESTING $(COVERAGE_CFLAGS) $(EDS_CFLAGS) $(XML2_CFLAGS)
OpenAB_Storage_CardDAV_tests_LDADD = ../src/libOpenAB.la -ldl $(XML2_LIBS)
OpenAB_Storage_CardDAV_tests_LDFLAGS = -rdynamic -no-install $(GTEST_LIBS) $(COVERAGE_LDFLAGS) $(EDS_LIBS)

if EDS_BACKEND
check_PROGRAMS += OpenAB_Storage_EDS_tests
TESTS += OpenAB_Storage_EDS_tests
OpenAB_Storage_EDS_tests_SOURCES = plugins/Storage/EDS/oab_storage_eds_tests_main.cpp \
				plugins/Storage/EDS/oab_storage_eds_tests.cpp

OpenAB_Storage_EDS_tests_CPPFLAGS = -I$(top_srcdir)/src $(GTEST_FLAGS) -DTESTING $(COVERAGE_CFLAGS) $(EDS_CFLAGS) $(XML2_CFLAGS)
OpenAB_Storage_EDS_tests_LDADD = ../src/libOpenAB.la -ldl $(XML2_LIBS)
OpenAB_Storage_EDS_tests_LDFLAGS = -rdynamic -no-install $(GTEST_LIBS) $(COVERAGE_LDFLAGS) $(EDS_LIBS)


check_PROGRAMS += OpenAB_Sync_OneWay_tests
TESTS += OpenAB_Sync_OneWay_tests
OpenAB_Sync_OneWay_tests_SOURCES = plugins/Sync/OneWay/oab_sync_oneway_tests_main.cpp \
				plugins/Sync/OneWay/oab_sync_oneway_tests.cpp

OpenAB_Sync_OneWay_tests_CPPFLAGS = -I$(top_srcdir)/src $(GTEST_FLAGS) -DTESTING $(COVERAGE_CFLAGS) $(EDS_CFLAGS)
OpenAB_Sync_OneWay_tests_LDADD = ../src/libOpenAB.la -ldl
OpenAB_Sync_OneWay_tests_LDFLAGS = -rdynamic -no-install $(GTEST_LIBS) $(COVERAGE_LDFLAGS) $(EDS_LIBS)
endif

if COVERAGE_ENABLED
.PHONY: clean-coverage
clean-coverage:
	@echo Removing coverage data
	-find -name '*.gcda' -print | xargs -r rm
	$(LCOV) --directory $(top_builddir) -z
	-rm -rf coverage.info coveragereport

.PHONY: generate-coverage generate-coverage-report
generate-coverage: 
	$(MAKE) $(AM_MAKEFLAGS) -k check
	$(MAKE) $(AM_MAKEFLAGS) generate-coverage-report

generate-coverage-report:
	$(LCOV) --directory $(top_builddir) --base-directory . --capture --output-file coverage.info --no-checksum --compat-libtool
	$(LCOV) --remove coverage.info "/usr*" -o coverage.info
	$(LCOV) --remove coverage.info "*/ab/tests/*" -o coverage.info
	LANG=C $(GENHTML) --prefix $(top_builddir) --output-directory coveragereport --title "OpenAB Code Coverage" --legend --show-details coverage.info	
endif
