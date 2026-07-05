

#-----------------------------------------------------------#
#															#
#							LIBMESH5						#
#															#
#-----------------------------------------------------------#
#															#
#	Description:		multi-system makefile (gmake only)	#
#	Author:				Loic MARECHAL						#
#	Creation date:		feb 16 2007							#
#	Last modification:	may 29 2009							#
#															#
#-----------------------------------------------------------#


CC     = gcc
CFLAGS = -O3 -m64 -Wall


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
	cp $(SRCSDIR)/*.f90 $(INCDIR)


# Objects depends on headers

$(OBJS): $(HDRS)

# Fortran API

fortran: $(SRCSDIR)/api/generate_fortran_api.c
	$(CC) $(CFLAGS) -I$(SRCSDIR) -lm $< -o $(SRCSDIR)/generate_fortran_api

# Build the working directories

$(DIRS):
	@[ -d $@ ] || mkdir $@


# Remove temporary files

clean:
	rm -f $(OBJS)

# Build a dated archive including sources, patterns and makefile

tar: $(DIRS)
	tar czf $(ARCHDIR)/libmesh5.`date +"%Y.%m.%d"`.tgz sources tests Makefile

zip: $(DIRS)
	archive_lm5.sh
