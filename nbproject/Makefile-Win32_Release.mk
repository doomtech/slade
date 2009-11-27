.build-conf:
	@echo Tool collection not found.
	@echo Please specify existing tool collection in project properties
	@exit 1

# Clean Targets
.clean-conf:
	${RM} -r build/Win32_Release
	${RM} dist/SLADE.exe

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
