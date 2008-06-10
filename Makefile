
incs=`wx-config --cflags`
libs=`wx-config --libs`

slade: LumpArea.o Lump.o LumpListPanel.o MainApp.o MainWindow.o MemChunk.o \
	Tokenizer.o Wad.o WadManager.o WadManagerPanel.o WadPanel.o ZipWad.o
	gcc ${libs} LumpArea.o Lump.o LumpListPanel.o MainApp.o MainWindow.o MemChunk.o \
	Tokenizer.o Wad.o WadManager.o WadManagerPanel.o WadPanel.o ZipWad.o \
		-o slade

LumpArea.o: LumpArea.cpp LumpArea.h
	gcc -c ${incs} LumpArea.cpp -o LumpArea.o

Lump.o: Lump.cpp Lump.h
	gcc -c ${incs} Lump.cpp -o Lump.o

LumpListPanel.o: LumpListPanel.cpp LumpListPanel.h
	gcc -c ${incs} LumpListPanel.cpp -o LumpListPanel.o

MainApp.o: MainApp.cpp MainApp.h Main.h WxStuff.h
	gcc -c ${incs} MainApp.cpp -o MainApp.o

MainWindow.o: MainWindow.cpp MainWindow.h
	gcc -c ${incs} MainWindow.cpp -o MainWindow.o

MemChunk.o: MemChunk.cpp MemChunk.h
	gcc -c ${incs} MemChunk.cpp -o MemChunk.o

Tokenizer.o: Tokenizer.cpp Tokenizer.h
	gcc -c ${incs} Tokenizer.cpp -o Tokenizer.o

Wad.o: Wad.cpp Wad.h
	gcc -c ${incs} Wad.cpp -o Wad.o

WadManager.o: WadManager.cpp WadManager.h
	gcc -c ${incs} WadManager.cpp -o WadManager.o

WadManagerPanel.o: WadManagerPanel.cpp WadManagerPanel.h
	gcc -c ${incs} WadManagerPanel.cpp -o WadManagerPanel.o

WadPanel.o: WadPanel.cpp WadPanel.h
	gcc -c ${incs} WadPanel.cpp -o WadPanel.o

ZipWad.o: ZipWad.cpp
	gcc -c ${incs} ZipWad.cpp -o ZipWad.o

clean:
	rm *.o slade -f


