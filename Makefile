# Makefile by Jim Bentler on 7/16/2006
# BSD licensed (GPL compatible)
CXX=g++
CXXFLAGS=-Wall -O2
SHARE_DIR=data_files/
CXXFLAGS+=`wx-config --cxxflags` -Dstricmp=strcasecmp -Dstrnicmp=strncasecmp -DNO_MAP_VIEWER=1 -DSHARE_DIR="\"$(SHARE_DIR)\""
LDFLAGS+=`wx-config --libs --gl-libs`
EXE?=slade
SRC=$(wildcard src/*.cpp src/zdbsp/*.cpp)
OBJ=$(patsubst %.cpp,%.o,${SRC})

all: slade

slade: ${OBJ}
	$(CXX) $(LDFLAGS) ${OBJ} -o $(EXE)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

.PHONY : clean

clean:
	rm -f src/*.o slade



