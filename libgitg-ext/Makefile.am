lib_LTLIBRARIES = libgitg-ext-1.0.la

AM_CPPFLAGS =								\
	-I$(top_srcdir)							\
	-I$(srcdir)							\
	$(LIBGITG_EXT_CFLAGS)						\
	$(WARN_CFLAGS)							\
	-DDATADIR=\""$(datadir)"\"					\
	-DLIBDIR=\""$(libdir)"\"

COMMON_VALA_FLAGS =				\
	--pkg Ggit-1.0 				\
	--pkg libgitg-1.0			\
	--pkg gio-2.0				\
	--pkg gtk+-3.0				\
	--pkg libsoup-2.4			\
	--pkg webkit2gtk-3.0			\
	--pkg gee-0.8				\
	--basedir $(top_srcdir)			\
	--vapidir $(top_srcdir)/vapi		\
	--vapidir $(top_builddir)/libgitg

AM_VALAFLAGS = 					\
	$(COMMON_VALA_FLAGS)			\
	--gir GitgExt-1.0.gir			\
	--girdir $(top_builddir)/libgitg	\
	--includedir libgitg-ext		\
	--header libgitg-ext.h			\
	--library libgitg-ext-1.0		\
	$(GITG_VALAFLAGS)

libgitg_ext_1_0_la_LDFLAGS = \
	-export-dynamic -no-undefined -export-symbols-regex "^[^_].*"

libgitg_ext_1_0_la_LIBADD = $(LIBGITG_EXT_LIBS)

VALA_FILES =					\
	gitg-ext-assembly-info.vala		\
	gitg-ext-application.vala		\
	gitg-ext-ui-element.vala		\
	gitg-ext-activity.vala			\
	gitg-ext-message-id.vala		\
	gitg-ext-message.vala			\
	gitg-ext-message-bus.vala		\
	gitg-ext-history.vala			\
	gitg-ext-history-panel.vala		\
	gitg-ext-command-line.vala		\
	gitg-ext-preferences.vala		\
	gitg-ext-ui.vala

libgitg_ext_1_0_la_SOURCES =		\
	$(VALA_FILES)			\
	gitg-ext-resources.c

headerdir = $(prefix)/include/libgitg-ext-1.0/libgitg-ext
header_DATA = libgitg-ext.h

vapidir = $(prefix)/share/vala/vapi
vapi_DATA = libgitg-ext-1.0.vapi

GitgExt-1.0.gir: libgitg-ext-1.0.la
libgitg-ext-1.0.vapi: libgitg-ext-1.0.la
libgitg-ext.h: libgitg-ext-1.0.la

# Ignore all warnings for vala code...
libgitg_ext_1_0_la_CFLAGS = \
	-w

if ENABLE_DOCS
docs: $(VALA_FILES)
	$(VALADOC) --directory $(top_builddir)/docs --package-name gitgext $(COMMON_VALA_FLAGS) $(VALA_FILES)
else
docs:
endif

girdir = $(INTROSPECTION_GIRDIR)
gir_DATA = GitgExt-1.0.gir

typelibdir = $(INTROSPECTION_TYPELIBDIR)
typelib_DATA = GitgExt-1.0.typelib

%.typelib: %.gir
	$(INTROSPECTION_COMPILER) $(INTROSPECTION_COMPILER_ARGS) --includedir=. --includedir=$(top_builddir)/libgitg -o $@ $<

if ENABLE_PYTHON
overridesdir = $(pyoverridesdir)
overrides_PYTHON = \
	GitgExt.py
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libgitg-ext-1.0.pc

gitg-ext-resources.c: resources/resources.xml $(shell $(GLIB_COMPILE_RESOURCES) --generate-dependencies --sourcedir $(srcdir)/resources resources/resources.xml)
	$(GLIB_COMPILE_RESOURCES) --generate-source 	\
		--sourcedir $(srcdir)/resources 	\
		--target "$@" "$<"

gitg-ext-resources.h: resources/resources.xml $(shell $(GLIB_COMPILE_RESOURCES) --generate-dependencies --sourcedir $(srcdir)/resources resources/resources.xml)
	$(GLIB_COMPILE_RESOURCES) --generate-header 	\
		--sourcedir $(srcdir)/resources 	\
		--target "$@" "$<"

BUILT_SOURCES =				\
	$(gir_DATA)			\
	libgitg-ext.h			\
	gitg-ext-resources.c		\
	gitg-ext-resources.h

EXTRA_DIST =				\
	$(pkgconfig_DATA)		\
	resources/resources.xml		\
	$(shell $(GLIB_COMPILE_RESOURCES) --generate-dependencies --sourcedir $(srcdir)/resources resources/resources.xml)

CLEANFILES = 					\
	$(VALA_FILES:.vala=.c)			\
	GitgExt-1.0.gir				\
	GitgExt-1.0.typelib			\
	libgitg-ext-1.0.vapi			\
	libgitg_ext_1_0_la_vala.stamp

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)

-include $(top_srcdir)/git.mk
-include $(top_srcdir)/valadoc.mk

# vi:ts=8:noet
