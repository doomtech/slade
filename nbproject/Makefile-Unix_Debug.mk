#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=

# Macros
PLATFORM=GNU-Linux-x86

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/Unix_Debug/${PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/MainWindow.o \
	${OBJECTDIR}/src/MemChunk.o \
	${OBJECTDIR}/src/Wad.o \
	${OBJECTDIR}/src/ListenerAnnouncer.o \
	${OBJECTDIR}/src/ZipWad.o \
	${OBJECTDIR}/src/WadManager.o \
	${OBJECTDIR}/src/LumpListPanel.o \
	${OBJECTDIR}/src/Lump.o \
	${OBJECTDIR}/src/Console.o \
	${OBJECTDIR}/src/WadPanel.o \
	${OBJECTDIR}/src/LumpArea.o \
	${OBJECTDIR}/src/MapEditorWindow.o \
	${OBJECTDIR}/src/WadManagerPanel.o \
	${OBJECTDIR}/src/MainApp.o \
	${OBJECTDIR}/src/TextLumpArea.o \
	${OBJECTDIR}/src/MapCanvas.o \
	${OBJECTDIR}/src/SLADEMap.o \
	${OBJECTDIR}/src/Tokenizer.o \
	${OBJECTDIR}/src/ConsolePanel.o

# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=`wx-config --cxxflags std,gl` 
CXXFLAGS=`wx-config --cxxflags std,gl` 

# Fortran Compiler Flags
FFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-Unix_Debug.mk dist/SLADE

dist/SLADE: ${OBJECTFILES}
	${MKDIR} -p dist
	${LINK.cc} `wx-config --libs std,gl` -o dist/SLADE  ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/src/MainWindow.o: src/MainWindow.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/MainWindow.o src/MainWindow.cpp

${OBJECTDIR}/src/MemChunk.o: src/MemChunk.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/MemChunk.o src/MemChunk.cpp

${OBJECTDIR}/src/Wad.o: src/Wad.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/Wad.o src/Wad.cpp

${OBJECTDIR}/src/ListenerAnnouncer.o: src/ListenerAnnouncer.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/ListenerAnnouncer.o src/ListenerAnnouncer.cpp

${OBJECTDIR}/src/ZipWad.o: src/ZipWad.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/ZipWad.o src/ZipWad.cpp

${OBJECTDIR}/src/WadManager.o: src/WadManager.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/WadManager.o src/WadManager.cpp

${OBJECTDIR}/src/LumpListPanel.o: src/LumpListPanel.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/LumpListPanel.o src/LumpListPanel.cpp

${OBJECTDIR}/src/Lump.o: src/Lump.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/Lump.o src/Lump.cpp

${OBJECTDIR}/src/Console.o: src/Console.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/Console.o src/Console.cpp

${OBJECTDIR}/src/WadPanel.o: src/WadPanel.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/WadPanel.o src/WadPanel.cpp

${OBJECTDIR}/src/LumpArea.o: src/LumpArea.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/LumpArea.o src/LumpArea.cpp

${OBJECTDIR}/src/MapEditorWindow.o: src/MapEditorWindow.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/MapEditorWindow.o src/MapEditorWindow.cpp

${OBJECTDIR}/src/WadManagerPanel.o: src/WadManagerPanel.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/WadManagerPanel.o src/WadManagerPanel.cpp

${OBJECTDIR}/src/MainApp.o: src/MainApp.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/MainApp.o src/MainApp.cpp

${OBJECTDIR}/src/TextLumpArea.o: src/TextLumpArea.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/TextLumpArea.o src/TextLumpArea.cpp

${OBJECTDIR}/src/MapCanvas.o: src/MapCanvas.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/MapCanvas.o src/MapCanvas.cpp

${OBJECTDIR}/src/SLADEMap.o: src/SLADEMap.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/SLADEMap.o src/SLADEMap.cpp

${OBJECTDIR}/src/Tokenizer.o: src/Tokenizer.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/Tokenizer.o src/Tokenizer.cpp

${OBJECTDIR}/src/ConsolePanel.o: src/ConsolePanel.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/ConsolePanel.o src/ConsolePanel.cpp

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
