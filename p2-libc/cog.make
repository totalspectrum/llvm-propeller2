PREFIX = /usr/local/propeller
DEST = $(PREFIX)/propeller-elf
MODEL=cog

CC=propeller-elf-gcc
MKDIR = mkdir -p
CFLAGS=-Os -m$(MODEL) $(CHIP) -nostdinc -I./include
AR=propeller-elf-ar

VPATH=.:cog:misc:stdio:stdlib:string:sys:sys/propeller:time:drivers:fdlibm

STRING = memcpy.o memmove.o memset.o memchr.o memcmp.o \
	strcat.o strcmp.o strcpy.o strlen.o \
	strncat.o strncmp.o strncpy.o \
	strchr.o strrchr.o strspn.o strcspn.o strtok.o \
	strcoll.o strxfrm.o \
	strstr.o strpbrk.o \
	strerror.o

STDLIB = malloc.o calloc.o realloc.o atoi.o atof.o \
	qsort.o bsearch.o strtol.o strtoul.o \
	strtoll.o strtoull.o \
	rand.o div.o abs.o getenv.o

MISC = 	ctype.o toupper.o tolower.o isprint.o isspace.o isalnum.o isdigit.o \
        isblank.o isxdigit.o isupper.o islower.o isgraph.o ispunct.o \
	sbrk.o rdwr.o signal.o locale.o

COG = cogregs.o div.o udiv.o mask.o mult.o miniprintf.o clock.o \
    cogserial.o cogputs.o

OBJS = $(COG) $(MISC) $(STRING) $(STDLIB)

COGOBJS = $(addprefix $(OBJDIR)/cog/, $(OBJS))

all: $(OBJDIR)/$(MODEL) $(OBJDIR)/$(MODEL)/libcog$(SUFFIX).a $(OBJDIR)/$(MODEL)/crt0_cog$(SUFFIX).o $(OBJDIR)/$(MODEL)/crtend_cog$(SUFFIX).o

$(OBJDIR)/$(MODEL):
	$(MKDIR) $(OBJDIR)/$(MODEL)

$(OBJDIR)/$(MODEL)/libcog$(SUFFIX).a: $(COGOBJS)
	$(AR) rs $@ $^

$(OBJDIR)/$(MODEL)/%.o: %.s
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJDIR)/$(MODEL)/%.o: %.S
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJDIR)/$(MODEL)/%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<
