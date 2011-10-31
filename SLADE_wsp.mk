.PHONY: clean All

All:
	@echo ----------Building project:[ SLADE - Debug-osx ]----------
	@"$(MAKE)" -w -f "SLADE.mk" && "$(MAKE)" -w -f "SLADE.mk" PostBuild
clean:
	@echo ----------Cleaning project:[ SLADE - Debug-osx ]----------
	@"$(MAKE)" -w -f "SLADE.mk" clean
