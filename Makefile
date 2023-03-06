CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS = -Wall -Wextra -pedantic
SRCDIR = src/

all:server.o peernode.o 

server.o : $(SRCDIR)server.cpp 
	$(CXX) -o server.o $(SRCDIR)server.cpp $(CPPFLAGS)


peernode.o : $(SRCDIR)peernode.cpp
	$(CXX) -o peernode.o $(SRCDIR)peernode.cpp $(CPPFLAGS)


clean:
	$(RM) *.o