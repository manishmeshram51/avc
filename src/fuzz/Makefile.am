noinst_PROGRAMS = pmdfuzzer

AM_CXXFLAGS = -I$(top_srcdir)/inc \
	$(REVENGE_GENERATORS_CFLAGS) \
	$(REVENGE_CFLAGS) \
	$(REVENGE_STREAM_CFLAGS) \
	$(DEBUG_CXXFLAGS)

pmdfuzzer_LDADD = \
	$(top_builddir)/src/lib/libpagemaker-@PMD_MAJOR_VERSION@.@PMD_MINOR_VERSION@.la \
	$(REVENGE_GENERATORS_LIBS) \
	$(REVENGE_LIBS) \
	$(REVENGE_STREAM_LIBS) \
	-lFuzzingEngine

pmdfuzzer_SOURCES = \
	pmdfuzzer.cpp
