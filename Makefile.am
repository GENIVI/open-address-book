## OpenAB C++: PIM helper Library

AUTOMAKE_OPTIONS = subdir-objects

ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}
AM_CPPFLAGS = -Wall -Wextra

SUBDIRS = src

dist_noinst_SCRIPTS = autogen.sh

if HAVE_DOXYGEN
SUBDIRS += doc
endif

if BUILD_EXAMPLES
SUBDIRS += examples
examples: src
endif

if BUILD_TESTS
SUBDIRS += tests
tests: src
endif

if COVERAGE_ENABLED
generate-coverage:
	$(MAKE) $(AM_MAKEFLAGS) -C tests generate-coverage
clean-coverage:
	$(MAKE) $(AM_MAKEFLAGS) -C tests clean-coverage
endif

if HAVE_CPPCHECK
cppcheck:
	cppcheck -I $(top_srcdir)/src --enable=all --template=gcc $(top_srcdir)/src
endif


$(SUBDIRS):
	$(MAKE) -C $@

.PHONY: $(SUBDIRS)
