
CXX=g++
CXXFLAGS=-O2 -DNDEBUG
LDFLAGS=-lfreeimage -laudiere -lfluidsynth
CXXFLAGS+=`wx-config --version=2.9 --debug=no --cxxflags std,aui,gl,stc`
LDFLAGS+=`wx-config --version=2.9 --debug=no --libs std,aui,gl,stc`
EXE?=./dist/slade
SRC=$(wildcard src/*.cpp)
SRC+=$(wildcard src/qmus2mid/*.cpp)
OBJ=$(patsubst %.cpp,%.o,${SRC})

all: slade

slade: ${OBJ}
	$(CXX) ${OBJ} -o $(EXE) $(LDFLAGS)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

.PHONY : clean

clean:
	rm -f src/*.o slade

