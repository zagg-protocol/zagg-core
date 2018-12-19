# SUBDIRS  = lib
AUTOMAKE_OPTIONS = foreign
bin_PROGRAMS = utxo
utxo_SOURCES = src/main.cpp   

noinst_LIBRARIES  = lib/libutxo.a
lib_libutxo_a_SOURCES = lib/add.cpp

LDADD = lib/libutxo.a




# Suppress any installation of SUBDIRS, which may be distributed separately
install-data:
	@:
install-dvi:
	@:
install-exec: all
	@:
install-html:
	@:
install-info:
	@:
install-pdf:
	@:
install-ps:
	@:
install: all
	@:
installcheck:
	@:
installdirs:
	@:
uninstall:
	@: