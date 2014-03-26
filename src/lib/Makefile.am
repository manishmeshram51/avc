if PLATFORM_WIN32
version_info = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) -avoid-version
else
version_info = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
endif

lib_LTLIBRARIES = libpagemaker-@PMD_MAJOR_VERSION@.@PMD_MINOR_VERSION@.la
libpagemaker_@PMD_MAJOR_VERSION@_@PMD_MINOR_VERSION@_includedir = $(includedir)/libpagemaker-@PMD_MAJOR_VERSION@.@PMD_MINOR_VERSION@/libpagemaker
libpagemaker_@PMD_MAJOR_VERSION@_@PMD_MINOR_VERSION@_include_HEADERS = \
	$(top_srcdir)/inc/libpagemaker/libpagemaker.h \
	$(top_srcdir)/inc/libpagemaker/PMDocument.h

AM_CXXFLAGS = -I$(top_srcdir)/inc $(YAML_CFLAGS) $(REVENGE_CFLAGS) $(REVENGE_STREAM_CFLAGS) $(DEBUG_CXXFLAGS)

libpagemaker_@PMD_MAJOR_VERSION@_@PMD_MINOR_VERSION@_la_LIBADD  = $(YAML_LIBS) $(REVENGE_LIBS) @LIBPMD_WIN32_RESOURCE@
libpagemaker_@PMD_MAJOR_VERSION@_@PMD_MINOR_VERSION@_la_DEPENDENCIES = @LIBPMD_WIN32_RESOURCE@
libpagemaker_@PMD_MAJOR_VERSION@_@PMD_MINOR_VERSION@_la_LDFLAGS = $(version_info) -export-dynamic -no-undefines
libpagemaker_@PMD_MAJOR_VERSION@_@PMD_MINOR_VERSION@_la_SOURCES = \
	PMDocument.cpp \
	PMDInternalStream.cpp \
	PMDInternalStream.h \
	libpagemaker_utils.cpp \
	libpagemaker_utils.h \
	OutputShape.h \
	PMDPage.h \
	PMDParser.cpp \
	PMDParser.h \
	PMDCollector.cpp \
	PMDCollector.h \
	PMDExceptions.h \
	PMDRecord.h \
	offsets.h \
	constants.h \
	Units.h \
	geometry.cpp \
	geometry.h \
	OutputShape.cpp \
	yaml_utils.cpp \
	yaml_utils.h

if OS_WIN32

@LIBPMD_WIN32_RESOURCE@ : libpagemaker.rc $(libpagemaker_@PMD_MAJOR_VERSION@_@PMD_MINOR_VERSION@_la_OBJECTS)
	chmod +x $(top_srcdir)/build/win32/*compile-resource
	WINDRES=@WINDRES@ $(top_srcdir)/build/win32/lt-compile-resource libpagemaker.rc @LIBPMD_WIN32_RESOURCE@

endif


EXTRA_DIST = \
	libpagemaker.rc.in

# These may be in the builddir too
BUILD_EXTRA_DIST = \
	libpagemaker.rc
