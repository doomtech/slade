.PHONY: clean All

All:
	@echo ----------Building project:[ SLADE - Debug ]----------
	@"$(MAKE)" -f "SLADE.mk"
clean:
	@echo ----------Cleaning project:[ SLADE - Debug ]----------
	@"$(MAKE)" -f "SLADE.mk" clean
