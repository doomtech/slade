.PHONY: clean All

All:
	@echo ----------Building project:[ SLADE - Release-osx ]----------
	@"$(MAKE)" -w -f "SLADE.mk"
clean:
	@echo ----------Cleaning project:[ SLADE - Release-osx ]----------
	@"$(MAKE)" -w -f "SLADE.mk" clean
