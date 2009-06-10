# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-Win32_Release.mk dist/SLADE.exe

dist/SLADE.exe: ${OBJECTFILES}
	${MKDIR} -p dist
	${LINK.cc} -Wl,-subsystem,windows -o dist/SLADE -s ${OBJECTFILES} ${LDLIBSOPTIONS} 

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf:
	${RM} -r build/Win32_Release
	${RM} dist/SLADE.exe

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
