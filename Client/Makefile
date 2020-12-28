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

CFLAGS = -I$(INCLDIR) -pthread

DEPS 	= $(wildcard $(INCLDIR)/*.h)

SRCS	= $(wildcard $(SRCDIR)/*.c)

_OBJS	= $(patsubst $(SRCDIR)/%.c,%.o,$(SRCS))
OBJS	= $(addprefix $(OBJDIR)/,$(_OBJS))

.PHONY: 
.PHONY: all

all: $(BIN)

$(BIN): $(OBJS) $(BINDIR)
	$(CC) $(CFLAGS) $(OBJS) -o $(BIN)

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
