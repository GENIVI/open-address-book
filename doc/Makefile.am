directory = $(top_srcdir)/doc/html

#dist_man_MANS = $(directory)/index.html
#$(directory)/index.html: doxyfile.stamp

# foohtmldir = $(htmldir)/foo/html

# nobase_dist_foohtml_DATA = $(directory)/* 

# dist_pkgdata_DATA = $(top_srcdir)/doc/html/*

doxyfile.stamp: clean-local
	$(DOXYGEN) Doxyfile
	echo Timestamp > doxyfile.stamp

CLEANFILES = doxyfile.stamp

install-data-local: doxyfile.stamp
	install -d $(htmldir)
	cp -a html $(htmldir)

all-local: doxyfile.stamp

clean-local:
	rm -rf html

