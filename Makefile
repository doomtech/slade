# Makefile by Jim Bentler on 7/16/2006
# BSD licensed (GPL compatible)
CXX=g++
CXXFLAGS=-O2
CXXFLAGS+=`wx-config --cxxflags`
LDFLAGS+=`wx-config --libs --gl-libs` -lwxscintilla
EXE?=slade
SRC=$(wildcard src/*.cpp)
OBJ=$(patsubst %.cpp,%.o,${SRC})

all: slade

slade: ${OBJ}
	$(CXX) $(LDFLAGS) ${OBJ} -o $(EXE)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

.PHONY : clean

clean:
	rm -f src/*.o slade

