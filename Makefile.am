ACLOCAL_AMFLAGS = -I m4

pkgconfdir = $(libdir)/pkgconfig

pkgconf_DATA = libpagemaker-@PMD_MAJOR_VERSION@.@PMD_MINOR_VERSION@.pc

SUBDIRS = src inc build docs

EXTRA_DIST = \
	AUTHORS \
	COPYING \
	HACKING \
	NEWS \
	NOTES \
	README \
	autogen.sh \
	libpagemaker.pc.in

distclean-local:
	rm -rf *.cache *~ *.pc

dist-hook:
	git log --date=short --pretty="format:@%cd  %an  <%ae>  [%H]%n%n%s%n%n%e%b" | sed -e "s|^\([^@]\)|\t\1|" -e "s|^@||" >$(distdir)/ChangeLog

astyle:
	astyle --options=astyle.options \*.cpp \*.h
