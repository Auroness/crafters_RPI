# Makefile for Crafter RPI Engine, $Revision: 0.01 $
# Copyright (C) 2015, Mary C. Huston

CC=g++

RESETDATE = `touch comm.cpp`
CXXFLAGS = $(CFLAGS) 
CFLAGS = -ggdb -DMACOSX -wall -O0 -Wno-write-strings -MYSQLPP_MYSQL_HEADERS_BURIED 

#LFLAGS = -lz -lmysqlclient -lm -lmysqld -lmygcc -lmysqlpp -m32
#LFLAGS = -lz -lmysqlclient -lm -lmysqld -lmygcc -lmysqlpp -llua -lreadline -fno-inline
LFLAGS = -lm $(shell mysql_config --libs) -lmysqlpp -llua -lreadline -fno-inline

INSTALL_DIR = .
# directory where lua files are located
LUA= $(INSTALL_DIR)/lua
LUAINC= -I$(LUA)/include/
LUALIB= -L$(LUA)/lib/

# do not change these
CFLAGS+=$(LUAINC)
LFFLAGS=$(LUALIB)

INCLUDE = -I/usr/local/ -I/usr/local/lib -I/usr/local/mysql/ -I/usr/local/mysql/include/ \
-I/usr/local/mysql/lib/ -I/usr/local/include/ -I/usr/local/include/mysql/ \
-I/usr/include/mysql/ -I/usr/include/openssl/ -I/usr/include/mysql++ \
-I/usr/lib/mysql $(LUAINC)

LIBDIRS = -L/Library/ -L/usr/lib/ -L/usr/local/ -L/usr/local/include/ \
-L/usr/local/lib/ -L/usr/local/mysql/ -L/usr/local/mysql/lib/ \
-L/usr/local/mysql/include/ $(LUALIB)


 
OBJS = \
account.o act.informative.o act.movement.o act.offensive.o \
act.other.o astronomy.o character.o clan.o comm.o \
comm.chat.o comm.emote.o comm.verbal.o comm.written.o \
commands.o commerce.o \
constants.o crafts.o create_mobile.o db.o dwellings.o  \
group.o guest.o guides.o handler.o hash.o item_tossable.o larg_prog.o\
limits.o magic.o mobact.o mysql.o nanny.o net_link.o \
object_damage.o objects.o olc.o portal.o roles.o save.o \
server.o somatics.o staff.o structs.o utility.o weather.o  


GAME_OBJS = $(OBJS) main.cpp

HEADERS = \
account.h astronomy.h clan.h character.h constants.h \
comm.emote.h decl.h \
group.h net_link.h portal.h protos.h room.h server.h \
stringstack.h structs.h trigram.h utility.h utils.h weather.h

# default target
../bin/server: $(OBJS)
	if test -f ../bin || install -v -d ../bin; then \
$(CC) -o $@ $(INCLUDE) $(LIBDIRS) $(LFLAGS) $(CFLAGS) $(GAME_OBJS) $(PFLAGS); fi

%.o: %.cpp $(HEADERS)
	$(CC) -c $(CFLAGS) $< $(PFLAGS)

%.o: %.c $(HEADERS)
	$(CC) -c $(CFLAGS) $< $(PFLAGS)

tags:
	ctags *.cpp *.h

clean:
	rm -f *.o $(INSTALL_DIR)/bin/server

