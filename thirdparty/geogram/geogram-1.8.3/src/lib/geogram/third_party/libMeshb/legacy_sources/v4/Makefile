

#-----------------------------------------------------------#
#															#
#							LIBMESH4						#
#															#
#-----------------------------------------------------------#
#															#
#	Description:		multi-system makefile (gmake only)	#
#	Author:				Loic MARECHAL						#
#	Creation date:		feb 11 2005							#
#	Last modification:	feb 18 2005							#
#															#
#-----------------------------------------------------------#


# Choose of the compiler depending on the architecture

ifeq ($(ARCHI), ppc)
	CC      = xlc
endif

ifeq ($(ARCHI), sparc)
	CC = acc
endif

ifeq ($(ARCHI), hppa)
	CC = c89
endif

ifeq ($(ARCHI), ia64)
	CC = c89
endif

ifeq ($(ARCHI), i86)
	CC = iccbin
endif

ifeq ($(ARCHI), mips)
	CC = c89
endif


# Working directories

LIBDIR  = $(HOME)/lib/$(ARCHI)
INCDIR  = $(HOME)/include
SRCSDIR = sources
OBJSDIR = objects/$(ARCHI)
ARCHDIR = archives
DIRS    = objects $(LIBDIR) $(OBJSDIR) $(ARCHDIR) $(INCDIR)
VPATH   = $(SRCSDIR)


# Files to be compiled

SRCS = $(wildcard $(SRCSDIR)/*.c)
HDRS = $(wildcard $(SRCSDIR)/*.h)
OBJS = $(patsubst $(SRCSDIR)%, $(OBJSDIR)%, $(SRCS:.c=.o))
LIB = libmesh4.a


# Definition of the compiling implicit rule

$(OBJSDIR)/%.o : $(SRCSDIR)/%.c
	$(CC) -c -O -I$(SRCSDIR) $< -o $@


# Install the library

$(LIBDIR)/$(LIB): $(DIRS) $(OBJS)
	cp $(OBJSDIR)/libmesh4.o $@
	cp $(SRCSDIR)/*.h $(INCDIR)


# Objects depends on headers

$(OBJS): $(HDRS)


# Build the working directories

$(DIRS):
	@[ -d $@ ] || mkdir $@


# Remove temporary files

clean:
	rm -f $(OBJS) $(LIBDIR)/$(LIB)

# Build a dated archive including sources, patterns and makefile

tar: $(DIRS)
	tar czf $(ARCHDIR)/libmesh4.`date +"%Y.%m.%d"`.tgz sources docs tests Makefile
