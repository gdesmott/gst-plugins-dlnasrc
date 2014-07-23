programs_list = \
    test-util

AM_CFLAGS = \
	-I${top_srcdir}/src -I${top_builddir}/src \
	$(GST_CFLAGS) $(SOUP_CFLAGS)
AM_LDFLAGS = \
	$(GST_LIBS) \
	$(SOUP_LIBS) \
	$(GST_PLUGIN_LDFLAGS)
LDADD = \
	$(top_builddir)/src/libdlnasrc-internal.la

noinst_PROGRAMS = $(programs_list)

TESTS = $(programs_list)
