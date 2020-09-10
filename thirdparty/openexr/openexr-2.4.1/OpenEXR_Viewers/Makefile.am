##
## SPDX-License-Identifier: BSD-3-Clause
## Copyright Contributors to the OpenEXR Project.
##

## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = playexr exrdisplay doc config

DIST_SUBDIRS = \
	$(SUBDIRS) 

EXTRA_DIST = \
	ChangeLog \
	LICENSE \
	README.md \
	bootstrap

dist-hook:
	cp -fpR $(abspath $(srcdir)/..)/cmake $(distdir)
	find $(distdir)/cmake -type d ! -perm -700 -exec chmod u+rwx {} \;


