.PHONY: clean All

All:
	@echo ----------Building project:[ SLADE - Release-osx ]----------
	@"$(MAKE)" -w -f "SLADE.mk" && "$(MAKE)" -w -f "SLADE.mk" PostBuild
clean:
	@echo ----------Cleaning project:[ SLADE - Release-osx ]----------
	@"$(MAKE)" -w -f "SLADE.mk" clean
