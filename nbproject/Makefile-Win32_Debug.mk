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
OBJECTDIR=build/Win32_Debug/${PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/MainWindow.o \
	${OBJECTDIR}/src/DefaultEntryPanel.o \
	${OBJECTDIR}/src/TextEditor.o \
	${OBJECTDIR}/src/ZipArchive.o \
	${OBJECTDIR}/src/ArchiveEntry.o \
	${OBJECTDIR}/src/TextEntryPanel.o \
	${OBJECTDIR}/src/WadArchive.o \
	${OBJECTDIR}/src/SLADEMap.o \
	${OBJECTDIR}/src/ConsolePanel.o \
	${OBJECTDIR}/src/Tokenizer.o \
	${OBJECTDIR}/src/ArchiveManagerPanel.o \
	${OBJECTDIR}/src/ArchiveManager.o \
	${OBJECTDIR}/src/MemChunk.o \
	${OBJECTDIR}/src/Archive.o \
	${OBJECTDIR}/src/ListenerAnnouncer.o \
	${OBJECTDIR}/src/Console.o \
	${OBJECTDIR}/src/ArchivePanel.o \
	${OBJECTDIR}/src/MapEditorWindow.o \
	${OBJECTDIR}/src/MainApp.o \
	${OBJECTDIR}/src/MapCanvas.o \
	${OBJECTDIR}/src/EntryListPanel.o \
	${OBJECTDIR}/src/EntryPanel.o

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
	${MAKE}  -f nbproject/Makefile-Win32_Debug.mk dist/SLADE.exe

dist/SLADE.exe: ${OBJECTFILES}
	${MKDIR} -p dist
	${LINK.cc} -o dist/SLADE ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/src/MainWindow.o: src/MainWindow.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/MainWindow.o src/MainWindow.cpp

${OBJECTDIR}/src/DefaultEntryPanel.o: src/DefaultEntryPanel.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/DefaultEntryPanel.o src/DefaultEntryPanel.cpp

${OBJECTDIR}/src/TextEditor.o: src/TextEditor.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/TextEditor.o src/TextEditor.cpp

${OBJECTDIR}/src/ZipArchive.o: src/ZipArchive.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/ZipArchive.o src/ZipArchive.cpp

${OBJECTDIR}/src/ArchiveEntry.o: src/ArchiveEntry.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/ArchiveEntry.o src/ArchiveEntry.cpp

${OBJECTDIR}/src/TextEntryPanel.o: src/TextEntryPanel.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/TextEntryPanel.o src/TextEntryPanel.cpp

${OBJECTDIR}/src/WadArchive.o: src/WadArchive.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/WadArchive.o src/WadArchive.cpp

${OBJECTDIR}/src/SLADEMap.o: src/SLADEMap.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/SLADEMap.o src/SLADEMap.cpp

${OBJECTDIR}/src/ConsolePanel.o: src/ConsolePanel.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/ConsolePanel.o src/ConsolePanel.cpp

${OBJECTDIR}/src/Tokenizer.o: src/Tokenizer.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/Tokenizer.o src/Tokenizer.cpp

${OBJECTDIR}/src/ArchiveManagerPanel.o: src/ArchiveManagerPanel.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/ArchiveManagerPanel.o src/ArchiveManagerPanel.cpp

${OBJECTDIR}/src/ArchiveManager.o: src/ArchiveManager.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/ArchiveManager.o src/ArchiveManager.cpp

${OBJECTDIR}/src/MemChunk.o: src/MemChunk.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/MemChunk.o src/MemChunk.cpp

${OBJECTDIR}/src/Archive.o: src/Archive.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/Archive.o src/Archive.cpp

${OBJECTDIR}/src/ListenerAnnouncer.o: src/ListenerAnnouncer.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/ListenerAnnouncer.o src/ListenerAnnouncer.cpp

${OBJECTDIR}/src/Console.o: src/Console.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/Console.o src/Console.cpp

${OBJECTDIR}/src/ArchivePanel.o: src/ArchivePanel.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/ArchivePanel.o src/ArchivePanel.cpp

${OBJECTDIR}/src/MapEditorWindow.o: src/MapEditorWindow.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/MapEditorWindow.o src/MapEditorWindow.cpp

${OBJECTDIR}/src/MainApp.o: src/MainApp.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/MainApp.o src/MainApp.cpp

${OBJECTDIR}/src/MapCanvas.o: src/MapCanvas.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/MapCanvas.o src/MapCanvas.cpp

${OBJECTDIR}/src/EntryListPanel.o: src/EntryListPanel.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/EntryListPanel.o src/EntryListPanel.cpp

${OBJECTDIR}/src/EntryPanel.o: src/EntryPanel.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/EntryPanel.o src/EntryPanel.cpp

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
