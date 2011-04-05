.PHONY: clean All

All:
	@echo ----------Building project:[ SLADE-osx - Release ]----------
	@"$(MAKE)" -w -f "SLADE-osx.mk" PrePreBuild && "$(MAKE)" -w -f "SLADE-osx.mk"
clean:
	@echo ----------Cleaning project:[ SLADE-osx - Release ]----------
	@"$(MAKE)" -w -f "SLADE-osx.mk" clean
