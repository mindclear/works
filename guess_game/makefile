Target=guess_server
CC=g++
INC=-I.
LIB=-lmysqlclient
    
CFLAGS:=-O0 -g $(INC)
CPPFLAGS:=$(CFLAGS)

MAINCPP:=$(shell echo *.cpp)
MAINOBJ:=$(patsubst %.cpp,%.o,$(MAINCPP))
MAINOBJ:=$(patsubst %.cc,%.o,$(MAINOBJ))
MAINDEP:=$(patsubst %.cpp,%.d,$(MAINCPP))
MAINDEP:=$(patsubst %.cc,%.d,$(MAINDEP))

all: $(Target)
deps: $(MAINDEP)

$(Target): $(MAINOBJ)
	$(CC) $^ $(CFLAGS) $(LIB) -o $@

main: $(MAINOBJ)
	echo $(MAINOBJ)

clean:
	rm -f $(MAINOBJ)
	rm -f $(MAINDEP) 
	rm -f $(Target)
