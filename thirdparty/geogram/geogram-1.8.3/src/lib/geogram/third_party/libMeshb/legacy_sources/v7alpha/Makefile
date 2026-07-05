

#-----------------------------------------------------------#
#															#
#							LIBMESH7						#
#															#
#-----------------------------------------------------------#
#															#
#	Description:		multi-system makefile (gmake only)	#
#	Author:				Loic MARECHAL						#
#	Creation date:		dec 08 2015							#
#	Last modification:	dec 08 2015							#
#															#
#-----------------------------------------------------------#


CC     = /usr/bin/gcc
CFLAGS = -O3


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
OBJS = $(patsubst $(SRCSDIR)%, $(OBJSDIR)%, $(SRCS:.c=.a))


# Definition of the compiling implicit rule

$(OBJSDIR)/%.a : $(SRCSDIR)/%.c
	$(CC) -c $(CFLAGS) -I$(SRCSDIR) $< -o $@


# Install the library

$(LIBDIR)/$(LIB): $(DIRS) $(OBJS)
	cp $(OBJSDIR)/*.a $@
	cp $(SRCSDIR)/*.h $(INCDIR)
	cp $(SRCSDIR)/*.ins $(INCDIR)


# Objects depends on headers

$(OBJS): $(HDRS)

# Build the working directories

$(DIRS):
	@[ -d $@ ] || mkdir $@


# Remove temporary files

clean:
	rm -f $(OBJS)

# Build a dated archive including sources, patterns and makefile

tar: $(DIRS)
	tar czf $(ARCHDIR)/libmesh7.`date +"%Y.%m.%d"`.tgz sources tests Makefile

zip: $(DIRS)
	archive_lm7.sh
