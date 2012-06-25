.PHONY: clean All

All:
	@echo ----------Building project:[ SLADE - Release-CMake ]----------
	@"$(MAKE)" -w -f "SLADE.mk"
clean:
	@echo ----------Cleaning project:[ SLADE - Release-CMake ]----------
	@"$(MAKE)" -w -f "SLADE.mk" clean
