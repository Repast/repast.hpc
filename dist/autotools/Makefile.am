ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src/repast_hpc src/relogo src/zombie src/rumor
EXTRA_DIST = docs MANUAL_INSTALLATION

dist-hook:
	rm -rf `find $(distdir) -name .svn`
