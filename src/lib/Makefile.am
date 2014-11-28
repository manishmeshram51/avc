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

AM_CXXFLAGS = -I$(top_srcdir)/inc $(REVENGE_CFLAGS) $(REVENGE_STREAM_CFLAGS) $(DEBUG_CXXFLAGS) -DLIBPAGEMAKER_BUILD

libpagemaker_@PMD_MAJOR_VERSION@_@PMD_MINOR_VERSION@_la_LIBADD  = $(REVENGE_LIBS) @LIBPMD_WIN32_RESOURCE@
libpagemaker_@PMD_MAJOR_VERSION@_@PMD_MINOR_VERSION@_la_DEPENDENCIES = @LIBPMD_WIN32_RESOURCE@
libpagemaker_@PMD_MAJOR_VERSION@_@PMD_MINOR_VERSION@_la_LDFLAGS = $(version_info) -export-dynamic -no-undefined
libpagemaker_@PMD_MAJOR_VERSION@_@PMD_MINOR_VERSION@_la_SOURCES = \
	OutputShape.cpp \
	OutputShape.h \
	PMDCollector.cpp \
	PMDCollector.h \
	PMDExceptions.h \
	PMDPage.h \
	PMDParser.cpp \
	PMDParser.h \
	PMDRecord.h \
	PMDTypes.h \
	PMDocument.cpp \
	Units.h \
	constants.h \
	geometry.cpp \
	geometry.h \
	libpagemaker_utils.cpp \
	libpagemaker_utils.h \
	offsets.h

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
