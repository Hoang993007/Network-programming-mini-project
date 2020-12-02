.SUFFIXES:
.SUFFIXES: .c .h .o

SHELL = /bin/bash

CC 	= gcc
INCLDIR = inc
OBJDIR 	= obj
SRCDIR 	= src
BINDIR 	= bin

_BIN	= client
BIN	= $(addprefix $(BINDIR)/,$(_BIN))

CFLAGS = -I$(INCLDIR)

DEPS 	= $(wildcard $(INCLDIR)/*.h)

SRCS	= $(wildcard $(SRCDIR)/*.c)

_OBJS	= $(patsubst $(SRCDIR)/%.c,%.o,$(SRCS))
OBJS	= $(addprefix $(OBJDIR)/,$(_OBJS))

CLIENTOBJ	= $(OBJDIR)/client.o

.PHONY: 
.PHONY: all

all: $(BIN)

$(BIN): $(OBJS) $(BINDIR) $(CLIENTOBJ)
	$(CC) $(CFLAGS) $(OBJS) $(CLIENTOBJ) -o $(BIN)

$(CLIENTOBJ): client.c $(OBJDIR) $(OBJS)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

#Create file if not exits
$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o *~
