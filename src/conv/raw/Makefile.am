if BUILD_TOOLS

bin_PROGRAMS = pmd2raw

AM_CXXFLAGS = -I$(top_srcdir)/inc $(YAML_CFLAGS) $(REVENGE_CFLAGS) $(REVENGE_GENERATORS_CFLAGS) $(REVENGE_STREAM_CFLAGS) $(DEBUG_CXXFLAGS)

pmd2raw_DEPENDENCIES = @PMD2RAW_WIN32_RESOURCE@

pmd2raw_LDADD = ../../lib/libpagemaker-@PMD_MAJOR_VERSION@.@PMD_MINOR_VERSION@.la $(YAML_LIBS) $(REVENGE_LIBS) $(REVENGE_GENERATORS_LIBS) $(REVENGE_STREAM_LIBS) @PMD2RAW_WIN32_RESOURCE@ 

pmd2raw_SOURCES = \
	pmd2raw.cpp

if OS_WIN32

@PMD2RAW_WIN32_RESOURCE@ : pmd2raw.rc $(pmd2raw_OBJECTS)
	chmod +x $(top_srcdir)/build/win32/*compile-resource
	WINDRES=@WINDRES@ $(top_srcdir)/build/win32/lt-compile-resource pmd2raw.rc @PMD2RAW_WIN32_RESOURCE@
endif

EXTRA_DIST = \
	$(pmd2raw_SOURCES) \
	pmd2raw.rc.in

# These may be in the builddir too
BUILD_EXTRA_DIST = \
	pmd2raw.rc	 
 
endif
