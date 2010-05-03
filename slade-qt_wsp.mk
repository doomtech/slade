.PHONY: clean All

All:
	@echo ----------Building project:[ szip - Debug ]----------
	@cd "szip" && "$(MAKE)" -f "szip.mk"
clean:
	@echo ----------Cleaning project:[ szip - Debug ]----------
	@cd "szip" && "$(MAKE)" -f "szip.mk" clean
