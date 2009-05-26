# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-Win32_Debug.mk dist/SLADE.exe

dist/SLADE.exe: ${OBJECTFILES}
	${MKDIR} -p dist
	${LINK.cc} -o dist/SLADE ${OBJECTFILES} ${LDLIBSOPTIONS} 

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf:
	${RM} -r build/Win32_Debug
	${RM} dist/SLADE.exe

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
