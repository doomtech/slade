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
CC=gcc.exe
CCC=g++.exe
CXX=g++.exe
FC=

# Macros
PLATFORM=MinGW-Windows

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/Win32_Release/${PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/MainWindow.o \
	${OBJECTDIR}/src/MemChunk.o \
	${OBJECTDIR}/src/Archive.o \
	${OBJECTDIR}/src/ListenerAnnouncer.o \
	${OBJECTDIR}/src/WadManager.o \
	${OBJECTDIR}/src/Console.o \
	${OBJECTDIR}/src/ZipArchive.o \
	${OBJECTDIR}/src/ArchiveEntry.o \
	${OBJECTDIR}/src/WadArchive.o \
	${OBJECTDIR}/src/WadPanel.o \
	${OBJECTDIR}/src/LumpArea.o \
	${OBJECTDIR}/src/MapEditorWindow.o \
	${OBJECTDIR}/src/WadManagerPanel.o \
	${OBJECTDIR}/src/MainApp.o \
	${OBJECTDIR}/src/TextLumpArea.o \
	${OBJECTDIR}/src/MapCanvas.o \
	${OBJECTDIR}/src/SLADEMap.o \
	${OBJECTDIR}/src/EntryListPanel.o \
	${OBJECTDIR}/src/ConsolePanel.o \
	${OBJECTDIR}/src/Tokenizer.o

# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lwx_mswu_aui-2.8 -lwx_mswu_html-2.8 -lwx_mswu_stc-2.8 -lwx_mswu_core-2.8 -lwx_baseu-2.8 -lwx_mswu_gl-2.8 -lwxzlib-2.8 -lwxregexu-2.8 -lwxexpat-2.8 -lwxtiff-2.8 -lwxpng-2.8 -lwxjpeg-2.8 -lwinmm -lcomctl32 -lrpcrt4 -lwsock32 -lglu32 -lopengl32 -lkernel32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid -lodbc32 -lodbccp32

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-Win32_Release.mk dist/SLADE.exe

dist/SLADE.exe: ${OBJECTFILES}
	${MKDIR} -p dist
	${LINK.cc} -o dist/SLADE -s ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/src/MainWindow.o: src/MainWindow.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/MainWindow.o src/MainWindow.cpp

${OBJECTDIR}/src/MemChunk.o: src/MemChunk.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/MemChunk.o src/MemChunk.cpp

${OBJECTDIR}/src/Archive.o: src/Archive.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/Archive.o src/Archive.cpp

${OBJECTDIR}/src/ListenerAnnouncer.o: src/ListenerAnnouncer.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/ListenerAnnouncer.o src/ListenerAnnouncer.cpp

${OBJECTDIR}/src/WadManager.o: src/WadManager.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/WadManager.o src/WadManager.cpp

${OBJECTDIR}/src/Console.o: src/Console.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/Console.o src/Console.cpp

${OBJECTDIR}/src/ZipArchive.o: src/ZipArchive.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/ZipArchive.o src/ZipArchive.cpp

${OBJECTDIR}/src/ArchiveEntry.o: src/ArchiveEntry.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/ArchiveEntry.o src/ArchiveEntry.cpp

${OBJECTDIR}/src/WadArchive.o: src/WadArchive.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/WadArchive.o src/WadArchive.cpp

${OBJECTDIR}/src/WadPanel.o: src/WadPanel.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/WadPanel.o src/WadPanel.cpp

${OBJECTDIR}/src/LumpArea.o: src/LumpArea.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/LumpArea.o src/LumpArea.cpp

${OBJECTDIR}/src/MapEditorWindow.o: src/MapEditorWindow.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/MapEditorWindow.o src/MapEditorWindow.cpp

${OBJECTDIR}/src/WadManagerPanel.o: src/WadManagerPanel.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/WadManagerPanel.o src/WadManagerPanel.cpp

${OBJECTDIR}/src/MainApp.o: src/MainApp.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/MainApp.o src/MainApp.cpp

${OBJECTDIR}/src/TextLumpArea.o: src/TextLumpArea.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/TextLumpArea.o src/TextLumpArea.cpp

${OBJECTDIR}/src/MapCanvas.o: src/MapCanvas.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/MapCanvas.o src/MapCanvas.cpp

${OBJECTDIR}/src/SLADEMap.o: src/SLADEMap.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/SLADEMap.o src/SLADEMap.cpp

${OBJECTDIR}/src/EntryListPanel.o: src/EntryListPanel.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/EntryListPanel.o src/EntryListPanel.cpp

${OBJECTDIR}/src/ConsolePanel.o: src/ConsolePanel.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/ConsolePanel.o src/ConsolePanel.cpp

${OBJECTDIR}/src/Tokenizer.o: src/Tokenizer.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/Tokenizer.o src/Tokenizer.cpp

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
