# Note: plugindir is set in configure

##############################################################################
# TODO: change libgstplugin.la to something else, e.g. libmysomething.la     #
##############################################################################
plugin_LTLIBRARIES = libgstdlnasrc.la

##############################################################################
# TODO: for the next set of variables, name the prefix if you named the .la, #
#  e.g. libmysomething.la => libmysomething_la_SOURCES                       #
#                            libmysomething_la_CFLAGS                        #
#                            libmysomething_la_LIBADD                        #
#                            libmysomething_la_LDFLAGS                       #
##############################################################################

# sources used to compile this plug-in
sources = \
	gstdlnasrc.c \
	gstdlnasrc.h \
	util.c \
	util.h

AM_CFLAGS = $(GST_CFLAGS) $(SOUP_CFLAGS)

# compiler and linker flags used to compile this plugin, set in configure.ac
libgstdlnasrc_la_SOURCES = $(sources)
libgstdlnasrc_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstdlnasrc_la_LIBADD = $(GST_LIBS) $(SOUP_LIBS)
libgstdlnasrc_la_LIBTOOLFLAGS = --tag=disable-static

# convenient internal lib used for tests
libdlnasrc_internal_la_SOURCES = $(sources)

noinst_LTLIBRARIES = libdlnasrc-internal.la

# headers we need but don't want installed
noinst_HEADERS = \
	gstdlnasrc.h \
	util.h
