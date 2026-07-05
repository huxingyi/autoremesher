
# Shell commands

MKDIR = mkdir
RM    = rm -f


# Definition of working directories

LIBNAME = libmesh.a
LIBDIR = $(HOME)/lib/$(ARCHI)
INCDIR = $(HOME)/include
OBJDIR  = objects/$(ARCHI)
DIRS    = objects $(LIBDIR) $(OBJDIR)


# Selection of the compiler and it's options depending on the architecture

ifeq ($(ARCHI),sparc)
	CC     = acc
	CFLAGS = -O4 -c -I. -I$(INCDIR) -libmil -cg92
endif

ifeq ($(ARCHI),hppa)
	CC     = c89
	CFLAGS = +O2 -c -I . -I $(INCDIR)
endif

ifeq ($(ARCHI),i86)
	CC     = gcc
	CFLAGS = -mpentiumpro -O6 -c -I . -I $(INCDIR) -Wuninitialized -Wunused
endif

ifeq ($(ARCHI),rs6000)
	CC     = c89
	CFLAGS = -O -c -I. -I$(INCDIR)
endif

ifeq ($(ARCHI),macintel)
	CC     = gcc
	CFLAGS = -O3 -m64 -c -I. -I$(INCDIR)
endif


# List of objects

OBJS = $(OBJDIR)/libmesh.o


# List of headers

INCS = *.h


# Making of the library

lib : $(DIRS) $(OBJS)
	cp $(OBJS) $(LIBDIR)/$(LIBNAME)
	cp $(INCS) $(INCDIR)


# Creation of objects

$(OBJDIR)/libmesh.o : libmesh.c
	$(CC) $(CFLAGS) libmesh.c -o $@


# Creation of working directories

$(DIRS):
	@[ -d $@ ] || $(MKDIR) $@


# Cleaning of the objects

clean:
	$(RM) $(OBJS)

