CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS = -lssl -lcrypto -Wall -Wextra -pedantic
SRCDIR = src/

all:idxserver.o peernode.o 

idxserver.o : $(SRCDIR)idxserver.cpp 
	$(CXX) -o idxserver.o $(SRCDIR)idxserver.cpp $(CPPFLAGS)


peernode.o : $(SRCDIR)peernode.cpp
	$(CXX) -o peernode.o $(SRCDIR)peernode.cpp $(CPPFLAGS)


clean:
	$(RM) *.o