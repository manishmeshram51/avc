if BUILD_TOOLS

bin_PROGRAMS = pmd2yaml

AM_CXXFLAGS = -I$(top_srcdir)/inc $(YAML_CFLAGS) $(REVENGE_CFLAGS) $(REVENGE_GENERATORS_CFLAGS) $(REVENGE_STREAM_CFLAGS) $(DEBUG_CXXFLAGS)

pmd2yaml_DEPENDENCIES = @PMD2YAML_WIN32_RESOURCE@

pmd2yaml_LDADD = ../../lib/libpagemaker-@PMD_MAJOR_VERSION@.@PMD_MINOR_VERSION@.la $(YAML_LIBS) $(REVENGE_LIBS) $(REVENGE_GENERATORS_LIBS) $(REVENGE_STREAM_LIBS) @PMD2YAML_WIN32_RESOURCE@ 

pmd2yaml_SOURCES = \
	pmd2yaml.cpp

if OS_WIN32

@PMD2YAML_WIN32_RESOURCE@ : pmd2yaml.rc $(pmd2yaml_OBJECTS)
	chmod +x $(top_srcdir)/build/win32/*compile-resource
	WINDRES=@WINDRES@ $(top_srcdir)/build/win32/lt-compile-resource pmd2yaml.rc @PMD2YAML_WIN32_RESOURCE@
endif

EXTRA_DIST = \
	$(pmd2yaml_SOURCES) \
	pmd2yaml.rc.in

# These may be in the builddir too
BUILD_EXTRA_DIST = \
	pmd2yaml.rc	 
 
endif
