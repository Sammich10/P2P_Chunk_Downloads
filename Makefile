CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS = -Wall -Wextra -pedantic
SRCDIR = src/

all:superpeer.o peernode.o 

superpeer.o : $(SRCDIR)superpeer.cpp 
	$(CXX) -o superpeer.o $(SRCDIR)superpeer.cpp $(CPPFLAGS)


peernode.o : $(SRCDIR)peernode.cpp
	$(CXX) -o peernode.o $(SRCDIR)peernode.cpp $(CPPFLAGS)


clean:
	$(RM) *.o