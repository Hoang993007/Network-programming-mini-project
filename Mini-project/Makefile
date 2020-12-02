.SUFFIXES:
.SUFFIXES: .c .h .o

SHELL = /bin/bash

CC 	= gcc

CLIENTOBJ	= ./Client/obj/client.o
SERVEROBJ	= ./Server/obj/server.o

SERVERSRCDIR 	= ./Server/src
SERVERSRCS	= $(wildcard $(SERVERSRCDIR)/*.c)

.PHONY: 
.PHONY: all

all: client server

client: $(CLIENTOBJ) ./Client/client.c
	make -C ./Client -f Makefile
	cp ./Client/bin/client .

server: $(SERVEROBJ) ./Server/server.c $(SERVERSRCS)
	make -C ./Server -f Makefile
	cp ./Server/bin/server .
