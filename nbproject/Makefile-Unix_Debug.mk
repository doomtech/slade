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
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_CONF=Unix_Debug
CND_DISTDIR=dist

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/ZipEntryListPanel.o \
	${OBJECTDIR}/src/MainWindow.o \
	${OBJECTDIR}/src/DefaultEntryPanel.o \
	${OBJECTDIR}/src/OGLCanvas.o \
	${OBJECTDIR}/src/TextEditor.o \
	${OBJECTDIR}/src/Palette.o \
	${OBJECTDIR}/src/PaletteEntryPanel.o \
	${OBJECTDIR}/src/ZipArchive.o \
	${OBJECTDIR}/src/ArchiveEntry.o \
	${OBJECTDIR}/src/TextEntryPanel.o \
	${OBJECTDIR}/src/WadArchive.o \
	${OBJECTDIR}/src/MapLine.o \
	${OBJECTDIR}/src/SLADEMap.o \
	${OBJECTDIR}/src/Tokenizer.o \
	${OBJECTDIR}/src/ConsolePanel.o \
	${OBJECTDIR}/src/PaletteManager.o \
	${OBJECTDIR}/src/ArchiveManagerPanel.o \
	${OBJECTDIR}/src/ArchiveManager.o \
	${OBJECTDIR}/src/Archive.o \
	${OBJECTDIR}/src/MemChunk.o \
	${OBJECTDIR}/src/GfxEntryPanel.o \
	${OBJECTDIR}/src/MapThing.o \
	${OBJECTDIR}/src/ListenerAnnouncer.o \
	${OBJECTDIR}/src/MapSector.o \
	${OBJECTDIR}/src/Console.o \
	${OBJECTDIR}/src/ArchivePanel.o \
	${OBJECTDIR}/src/MapSide.o \
	${OBJECTDIR}/src/GfxCanvas.o \
	${OBJECTDIR}/src/PaletteCanvas.o \
	${OBJECTDIR}/src/SImage.o \
	${OBJECTDIR}/src/MapEditorWindow.o \
	${OBJECTDIR}/src/MainApp.o \
	${OBJECTDIR}/src/GfxConvDialog.o \
	${OBJECTDIR}/src/MapCanvas.o \
	${OBJECTDIR}/src/EntryListPanel.o \
	${OBJECTDIR}/src/Misc.o \
	${OBJECTDIR}/src/EntryPanel.o \
	${OBJECTDIR}/src/MapVertex.o \
	${OBJECTDIR}/src/ZipArchivePanel.o \
	${OBJECTDIR}/src/CVar.o

# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=`wx-config --debug --cxxflags std,gl,stc,aui` 
CXXFLAGS=`wx-config --debug --cxxflags std,gl,stc,aui` 

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lfreeimage

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-Unix_Debug.mk dist/SLADE

dist/SLADE: ${OBJECTFILES}
	${MKDIR} -p dist
	${LINK.cc} `wx-config --debug --libs std,gl,stc,aui` -o dist/SLADE  ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/src/ZipEntryListPanel.o: nbproject/Makefile-${CND_CONF}.mk src/ZipEntryListPanel.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/ZipEntryListPanel.o src/ZipEntryListPanel.cpp

${OBJECTDIR}/src/MainWindow.o: nbproject/Makefile-${CND_CONF}.mk src/MainWindow.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/MainWindow.o src/MainWindow.cpp

${OBJECTDIR}/src/DefaultEntryPanel.o: nbproject/Makefile-${CND_CONF}.mk src/DefaultEntryPanel.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/DefaultEntryPanel.o src/DefaultEntryPanel.cpp

${OBJECTDIR}/src/OGLCanvas.o: nbproject/Makefile-${CND_CONF}.mk src/OGLCanvas.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/OGLCanvas.o src/OGLCanvas.cpp

${OBJECTDIR}/src/TextEditor.o: nbproject/Makefile-${CND_CONF}.mk src/TextEditor.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/TextEditor.o src/TextEditor.cpp

${OBJECTDIR}/src/Palette.o: nbproject/Makefile-${CND_CONF}.mk src/Palette.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/Palette.o src/Palette.cpp

${OBJECTDIR}/src/PaletteEntryPanel.o: nbproject/Makefile-${CND_CONF}.mk src/PaletteEntryPanel.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/PaletteEntryPanel.o src/PaletteEntryPanel.cpp

${OBJECTDIR}/src/ZipArchive.o: nbproject/Makefile-${CND_CONF}.mk src/ZipArchive.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/ZipArchive.o src/ZipArchive.cpp

${OBJECTDIR}/src/ArchiveEntry.o: nbproject/Makefile-${CND_CONF}.mk src/ArchiveEntry.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/ArchiveEntry.o src/ArchiveEntry.cpp

${OBJECTDIR}/src/TextEntryPanel.o: nbproject/Makefile-${CND_CONF}.mk src/TextEntryPanel.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/TextEntryPanel.o src/TextEntryPanel.cpp

${OBJECTDIR}/src/WadArchive.o: nbproject/Makefile-${CND_CONF}.mk src/WadArchive.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/WadArchive.o src/WadArchive.cpp

${OBJECTDIR}/src/MapLine.o: nbproject/Makefile-${CND_CONF}.mk src/MapLine.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/MapLine.o src/MapLine.cpp

${OBJECTDIR}/src/SLADEMap.o: nbproject/Makefile-${CND_CONF}.mk src/SLADEMap.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/SLADEMap.o src/SLADEMap.cpp

${OBJECTDIR}/src/Tokenizer.o: nbproject/Makefile-${CND_CONF}.mk src/Tokenizer.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/Tokenizer.o src/Tokenizer.cpp

${OBJECTDIR}/src/ConsolePanel.o: nbproject/Makefile-${CND_CONF}.mk src/ConsolePanel.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/ConsolePanel.o src/ConsolePanel.cpp

${OBJECTDIR}/src/PaletteManager.o: nbproject/Makefile-${CND_CONF}.mk src/PaletteManager.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/PaletteManager.o src/PaletteManager.cpp

${OBJECTDIR}/src/ArchiveManagerPanel.o: nbproject/Makefile-${CND_CONF}.mk src/ArchiveManagerPanel.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/ArchiveManagerPanel.o src/ArchiveManagerPanel.cpp

${OBJECTDIR}/src/ArchiveManager.o: nbproject/Makefile-${CND_CONF}.mk src/ArchiveManager.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/ArchiveManager.o src/ArchiveManager.cpp

${OBJECTDIR}/src/Archive.o: nbproject/Makefile-${CND_CONF}.mk src/Archive.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/Archive.o src/Archive.cpp

${OBJECTDIR}/src/MemChunk.o: nbproject/Makefile-${CND_CONF}.mk src/MemChunk.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/MemChunk.o src/MemChunk.cpp

${OBJECTDIR}/src/GfxEntryPanel.o: nbproject/Makefile-${CND_CONF}.mk src/GfxEntryPanel.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/GfxEntryPanel.o src/GfxEntryPanel.cpp

${OBJECTDIR}/src/MapThing.o: nbproject/Makefile-${CND_CONF}.mk src/MapThing.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/MapThing.o src/MapThing.cpp

${OBJECTDIR}/src/ListenerAnnouncer.o: nbproject/Makefile-${CND_CONF}.mk src/ListenerAnnouncer.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/ListenerAnnouncer.o src/ListenerAnnouncer.cpp

${OBJECTDIR}/src/MapSector.o: nbproject/Makefile-${CND_CONF}.mk src/MapSector.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/MapSector.o src/MapSector.cpp

${OBJECTDIR}/src/Console.o: nbproject/Makefile-${CND_CONF}.mk src/Console.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/Console.o src/Console.cpp

${OBJECTDIR}/src/ArchivePanel.o: nbproject/Makefile-${CND_CONF}.mk src/ArchivePanel.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/ArchivePanel.o src/ArchivePanel.cpp

${OBJECTDIR}/src/MapSide.o: nbproject/Makefile-${CND_CONF}.mk src/MapSide.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/MapSide.o src/MapSide.cpp

${OBJECTDIR}/src/GfxCanvas.o: nbproject/Makefile-${CND_CONF}.mk src/GfxCanvas.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/GfxCanvas.o src/GfxCanvas.cpp

${OBJECTDIR}/src/PaletteCanvas.o: nbproject/Makefile-${CND_CONF}.mk src/PaletteCanvas.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/PaletteCanvas.o src/PaletteCanvas.cpp

${OBJECTDIR}/src/SImage.o: nbproject/Makefile-${CND_CONF}.mk src/SImage.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/SImage.o src/SImage.cpp

${OBJECTDIR}/src/MapEditorWindow.o: nbproject/Makefile-${CND_CONF}.mk src/MapEditorWindow.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/MapEditorWindow.o src/MapEditorWindow.cpp

${OBJECTDIR}/src/MainApp.o: nbproject/Makefile-${CND_CONF}.mk src/MainApp.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/MainApp.o src/MainApp.cpp

${OBJECTDIR}/src/GfxConvDialog.o: nbproject/Makefile-${CND_CONF}.mk src/GfxConvDialog.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/GfxConvDialog.o src/GfxConvDialog.cpp

${OBJECTDIR}/src/MapCanvas.o: nbproject/Makefile-${CND_CONF}.mk src/MapCanvas.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/MapCanvas.o src/MapCanvas.cpp

${OBJECTDIR}/src/EntryListPanel.o: nbproject/Makefile-${CND_CONF}.mk src/EntryListPanel.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/EntryListPanel.o src/EntryListPanel.cpp

${OBJECTDIR}/src/Misc.o: nbproject/Makefile-${CND_CONF}.mk src/Misc.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/Misc.o src/Misc.cpp

${OBJECTDIR}/src/EntryPanel.o: nbproject/Makefile-${CND_CONF}.mk src/EntryPanel.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/EntryPanel.o src/EntryPanel.cpp

${OBJECTDIR}/src/MapVertex.o: nbproject/Makefile-${CND_CONF}.mk src/MapVertex.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/MapVertex.o src/MapVertex.cpp

${OBJECTDIR}/src/ZipArchivePanel.o: nbproject/Makefile-${CND_CONF}.mk src/ZipArchivePanel.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/ZipArchivePanel.o src/ZipArchivePanel.cpp

${OBJECTDIR}/src/CVar.o: nbproject/Makefile-${CND_CONF}.mk src/CVar.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -D__WXDEBUG__ -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/CVar.o src/CVar.cpp

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
