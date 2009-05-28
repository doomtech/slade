# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-Unix_Debug.mk dist/SLADE

dist/SLADE: ${OBJECTFILES}
	${MKDIR} -p dist
	${LINK.cc} `wx-config --libs std,gl,stc` -o dist/SLADE  ${OBJECTFILES} ${LDLIBSOPTIONS} 

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf:
	${RM} -r build/Unix_Debug
	${RM} dist/SLADE

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
