.PHONY: clean All

All:
	@echo ----------Building project:[ SLADE - Release ]----------
	@"mingw32-make.exe"  -j 4 -f "SLADE.mk"
clean:
	@echo ----------Cleaning project:[ SLADE - Release ]----------
	@"mingw32-make.exe"  -j 4 -f "SLADE.mk" clean
