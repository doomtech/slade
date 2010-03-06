##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Release
ProjectName            :=SLADE
ConfigurationName      :=Release
IntermediateDirectory  :=./Release
OutDir                 := $(IntermediateDirectory)
WorkspacePath          := "/home/simon/dev/slade/trunk"
ProjectPath            := "/home/simon/dev/slade/trunk"
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=Simon
Date                   :=07/03/10
CodeLitePath           :="/home/simon/.codelite"
LinkerName             :=g++
ArchiveTool            :=ar rcus
SharedObjectLinkerName :=g++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=
DebugSwitch            :=-gstab
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
CompilerName           :=g++
C_CompilerName         :=gcc
OutputFile             :=./dist/slade
Preprocessors          :=$(PreprocessorSwitch)__WX__ 
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=
MakeDirCommand         :=mkdir -p
CmpOptions             := -O2 $(shell wx-config --cxxflags std,gl,stc,aui --unicode=yes --debug=no) $(Preprocessors)
LinkOptions            :=  -s $(shell wx-config --debug=no --libs std,gl,stc,aui --unicode=yes)
IncludePath            :=  "$(IncludeSwitch)." 
RcIncludePath          :=
Libs                   :=$(LibrarySwitch)freeimage 
LibPath                := "$(LibraryPathSwitch)." 


##
## User defined environment variables
##
Objects=src/$(IntermediateDirectory)/GfxConvDialog$(ObjectSuffix) src/$(IntermediateDirectory)/ModifyOffsetsDialog$(ObjectSuffix) src/$(IntermediateDirectory)/PaletteDialog$(ObjectSuffix) src/$(IntermediateDirectory)/Clipboard$(ObjectSuffix) src/$(IntermediateDirectory)/Console$(ObjectSuffix) src/$(IntermediateDirectory)/CVar$(ObjectSuffix) src/$(IntermediateDirectory)/ListenerAnnouncer$(ObjectSuffix) src/$(IntermediateDirectory)/MainApp$(ObjectSuffix) src/$(IntermediateDirectory)/MemChunk$(ObjectSuffix) src/$(IntermediateDirectory)/Misc$(ObjectSuffix) \
	src/$(IntermediateDirectory)/Tokenizer$(ObjectSuffix) src/$(IntermediateDirectory)/WxStuff$(ObjectSuffix) src/$(IntermediateDirectory)/OpenGL$(ObjectSuffix) src/$(IntermediateDirectory)/ArchivePanel$(ObjectSuffix) src/$(IntermediateDirectory)/EntryListPanel$(ObjectSuffix) src/$(IntermediateDirectory)/DefaultEntryPanel$(ObjectSuffix) src/$(IntermediateDirectory)/EntryPanel$(ObjectSuffix) src/$(IntermediateDirectory)/GfxEntryPanel$(ObjectSuffix) src/$(IntermediateDirectory)/MultiEntryPanel$(ObjectSuffix) src/$(IntermediateDirectory)/PaletteEntryPanel$(ObjectSuffix) \
	src/$(IntermediateDirectory)/TextEntryPanel$(ObjectSuffix) src/$(IntermediateDirectory)/TextureXEntryPanel$(ObjectSuffix) src/$(IntermediateDirectory)/ZipArchivePanel$(ObjectSuffix) src/$(IntermediateDirectory)/ZipEntryListPanel$(ObjectSuffix) src/$(IntermediateDirectory)/ArchiveManagerPanel$(ObjectSuffix) src/$(IntermediateDirectory)/MainWindow$(ObjectSuffix) src/$(IntermediateDirectory)/EntryOperations$(ObjectSuffix) src/$(IntermediateDirectory)/MapLine$(ObjectSuffix) src/$(IntermediateDirectory)/MapSector$(ObjectSuffix) src/$(IntermediateDirectory)/MapSide$(ObjectSuffix) \
	src/$(IntermediateDirectory)/MapThing$(ObjectSuffix) src/$(IntermediateDirectory)/MapVertex$(ObjectSuffix) src/$(IntermediateDirectory)/SLADEMap$(ObjectSuffix) src/$(IntermediateDirectory)/MapCanvas$(ObjectSuffix) src/$(IntermediateDirectory)/MapEditorWindow$(ObjectSuffix) src/$(IntermediateDirectory)/Archive$(ObjectSuffix) src/$(IntermediateDirectory)/ArchiveEntry$(ObjectSuffix) src/$(IntermediateDirectory)/ArchiveManager$(ObjectSuffix) src/$(IntermediateDirectory)/WadArchive$(ObjectSuffix) src/$(IntermediateDirectory)/ZipArchive$(ObjectSuffix) \
	src/$(IntermediateDirectory)/EntryType$(ObjectSuffix) src/$(IntermediateDirectory)/SImage$(ObjectSuffix) src/$(IntermediateDirectory)/Palette$(ObjectSuffix) src/$(IntermediateDirectory)/PaletteManager$(ObjectSuffix) src/$(IntermediateDirectory)/CTexture$(ObjectSuffix) src/$(IntermediateDirectory)/TextureXList$(ObjectSuffix) src/$(IntermediateDirectory)/Icons$(ObjectSuffix) src/$(IntermediateDirectory)/GLTexture$(ObjectSuffix) src/$(IntermediateDirectory)/ColourBox$(ObjectSuffix) src/$(IntermediateDirectory)/CTextureCanvas$(ObjectSuffix) \
	src/$(IntermediateDirectory)/GfxCanvas$(ObjectSuffix) src/$(IntermediateDirectory)/ListView$(ObjectSuffix) src/$(IntermediateDirectory)/OGLCanvas$(ObjectSuffix) src/$(IntermediateDirectory)/PaletteCanvas$(ObjectSuffix) src/$(IntermediateDirectory)/PaletteChooser$(ObjectSuffix) src/$(IntermediateDirectory)/TextEditor$(ObjectSuffix) src/$(IntermediateDirectory)/ConsolePanel$(ObjectSuffix) src/$(IntermediateDirectory)/SplashWindow$(ObjectSuffix) 

##
## Main Build Targets 
##
all: $(OutputFile)

$(OutputFile): makeDirStep $(Objects)
	@$(MakeDirCommand) $(@D)
	$(LinkerName) $(OutputSwitch)$(OutputFile) $(Objects) $(LibPath) $(Libs) $(LinkOptions)

makeDirStep:
	@test -d ./Release || $(MakeDirCommand) ./Release

PreBuild:


##
## Objects
##
src/$(IntermediateDirectory)/GfxConvDialog$(ObjectSuffix): src/GfxConvDialog.cpp src/$(IntermediateDirectory)/GfxConvDialog$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/GfxConvDialog.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/GfxConvDialog$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/GfxConvDialog$(DependSuffix): src/GfxConvDialog.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/GfxConvDialog$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/GfxConvDialog$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/GfxConvDialog.cpp"

src/$(IntermediateDirectory)/ModifyOffsetsDialog$(ObjectSuffix): src/ModifyOffsetsDialog.cpp src/$(IntermediateDirectory)/ModifyOffsetsDialog$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/ModifyOffsetsDialog.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/ModifyOffsetsDialog$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/ModifyOffsetsDialog$(DependSuffix): src/ModifyOffsetsDialog.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/ModifyOffsetsDialog$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/ModifyOffsetsDialog$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/ModifyOffsetsDialog.cpp"

src/$(IntermediateDirectory)/PaletteDialog$(ObjectSuffix): src/PaletteDialog.cpp src/$(IntermediateDirectory)/PaletteDialog$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/PaletteDialog.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/PaletteDialog$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/PaletteDialog$(DependSuffix): src/PaletteDialog.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/PaletteDialog$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/PaletteDialog$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/PaletteDialog.cpp"

src/$(IntermediateDirectory)/Clipboard$(ObjectSuffix): src/Clipboard.cpp src/$(IntermediateDirectory)/Clipboard$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/Clipboard.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/Clipboard$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/Clipboard$(DependSuffix): src/Clipboard.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/Clipboard$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/Clipboard$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/Clipboard.cpp"

src/$(IntermediateDirectory)/Console$(ObjectSuffix): src/Console.cpp src/$(IntermediateDirectory)/Console$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/Console.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/Console$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/Console$(DependSuffix): src/Console.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/Console$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/Console$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/Console.cpp"

src/$(IntermediateDirectory)/CVar$(ObjectSuffix): src/CVar.cpp src/$(IntermediateDirectory)/CVar$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/CVar.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/CVar$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/CVar$(DependSuffix): src/CVar.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/CVar$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/CVar$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/CVar.cpp"

src/$(IntermediateDirectory)/ListenerAnnouncer$(ObjectSuffix): src/ListenerAnnouncer.cpp src/$(IntermediateDirectory)/ListenerAnnouncer$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/ListenerAnnouncer.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/ListenerAnnouncer$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/ListenerAnnouncer$(DependSuffix): src/ListenerAnnouncer.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/ListenerAnnouncer$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/ListenerAnnouncer$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/ListenerAnnouncer.cpp"

src/$(IntermediateDirectory)/MainApp$(ObjectSuffix): src/MainApp.cpp src/$(IntermediateDirectory)/MainApp$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/MainApp.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/MainApp$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/MainApp$(DependSuffix): src/MainApp.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/MainApp$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/MainApp$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/MainApp.cpp"

src/$(IntermediateDirectory)/MemChunk$(ObjectSuffix): src/MemChunk.cpp src/$(IntermediateDirectory)/MemChunk$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/MemChunk.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/MemChunk$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/MemChunk$(DependSuffix): src/MemChunk.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/MemChunk$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/MemChunk$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/MemChunk.cpp"

src/$(IntermediateDirectory)/Misc$(ObjectSuffix): src/Misc.cpp src/$(IntermediateDirectory)/Misc$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/Misc.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/Misc$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/Misc$(DependSuffix): src/Misc.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/Misc$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/Misc$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/Misc.cpp"

src/$(IntermediateDirectory)/Tokenizer$(ObjectSuffix): src/Tokenizer.cpp src/$(IntermediateDirectory)/Tokenizer$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/Tokenizer.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/Tokenizer$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/Tokenizer$(DependSuffix): src/Tokenizer.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/Tokenizer$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/Tokenizer$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/Tokenizer.cpp"

src/$(IntermediateDirectory)/WxStuff$(ObjectSuffix): src/WxStuff.cpp src/$(IntermediateDirectory)/WxStuff$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/WxStuff.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/WxStuff$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/WxStuff$(DependSuffix): src/WxStuff.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/WxStuff$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/WxStuff$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/WxStuff.cpp"

src/$(IntermediateDirectory)/OpenGL$(ObjectSuffix): src/OpenGL.cpp src/$(IntermediateDirectory)/OpenGL$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/OpenGL.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/OpenGL$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/OpenGL$(DependSuffix): src/OpenGL.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/OpenGL$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/OpenGL$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/OpenGL.cpp"

src/$(IntermediateDirectory)/ArchivePanel$(ObjectSuffix): src/ArchivePanel.cpp src/$(IntermediateDirectory)/ArchivePanel$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/ArchivePanel.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/ArchivePanel$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/ArchivePanel$(DependSuffix): src/ArchivePanel.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/ArchivePanel$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/ArchivePanel$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/ArchivePanel.cpp"

src/$(IntermediateDirectory)/EntryListPanel$(ObjectSuffix): src/EntryListPanel.cpp src/$(IntermediateDirectory)/EntryListPanel$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/EntryListPanel.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/EntryListPanel$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/EntryListPanel$(DependSuffix): src/EntryListPanel.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/EntryListPanel$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/EntryListPanel$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/EntryListPanel.cpp"

src/$(IntermediateDirectory)/DefaultEntryPanel$(ObjectSuffix): src/DefaultEntryPanel.cpp src/$(IntermediateDirectory)/DefaultEntryPanel$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/DefaultEntryPanel.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/DefaultEntryPanel$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/DefaultEntryPanel$(DependSuffix): src/DefaultEntryPanel.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/DefaultEntryPanel$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/DefaultEntryPanel$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/DefaultEntryPanel.cpp"

src/$(IntermediateDirectory)/EntryPanel$(ObjectSuffix): src/EntryPanel.cpp src/$(IntermediateDirectory)/EntryPanel$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/EntryPanel.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/EntryPanel$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/EntryPanel$(DependSuffix): src/EntryPanel.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/EntryPanel$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/EntryPanel$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/EntryPanel.cpp"

src/$(IntermediateDirectory)/GfxEntryPanel$(ObjectSuffix): src/GfxEntryPanel.cpp src/$(IntermediateDirectory)/GfxEntryPanel$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/GfxEntryPanel.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/GfxEntryPanel$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/GfxEntryPanel$(DependSuffix): src/GfxEntryPanel.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/GfxEntryPanel$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/GfxEntryPanel$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/GfxEntryPanel.cpp"

src/$(IntermediateDirectory)/MultiEntryPanel$(ObjectSuffix): src/MultiEntryPanel.cpp src/$(IntermediateDirectory)/MultiEntryPanel$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/MultiEntryPanel.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/MultiEntryPanel$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/MultiEntryPanel$(DependSuffix): src/MultiEntryPanel.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/MultiEntryPanel$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/MultiEntryPanel$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/MultiEntryPanel.cpp"

src/$(IntermediateDirectory)/PaletteEntryPanel$(ObjectSuffix): src/PaletteEntryPanel.cpp src/$(IntermediateDirectory)/PaletteEntryPanel$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/PaletteEntryPanel.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/PaletteEntryPanel$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/PaletteEntryPanel$(DependSuffix): src/PaletteEntryPanel.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/PaletteEntryPanel$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/PaletteEntryPanel$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/PaletteEntryPanel.cpp"

src/$(IntermediateDirectory)/TextEntryPanel$(ObjectSuffix): src/TextEntryPanel.cpp src/$(IntermediateDirectory)/TextEntryPanel$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/TextEntryPanel.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/TextEntryPanel$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/TextEntryPanel$(DependSuffix): src/TextEntryPanel.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/TextEntryPanel$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/TextEntryPanel$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/TextEntryPanel.cpp"

src/$(IntermediateDirectory)/TextureXEntryPanel$(ObjectSuffix): src/TextureXEntryPanel.cpp src/$(IntermediateDirectory)/TextureXEntryPanel$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/TextureXEntryPanel.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/TextureXEntryPanel$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/TextureXEntryPanel$(DependSuffix): src/TextureXEntryPanel.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/TextureXEntryPanel$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/TextureXEntryPanel$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/TextureXEntryPanel.cpp"

src/$(IntermediateDirectory)/ZipArchivePanel$(ObjectSuffix): src/ZipArchivePanel.cpp src/$(IntermediateDirectory)/ZipArchivePanel$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/ZipArchivePanel.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/ZipArchivePanel$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/ZipArchivePanel$(DependSuffix): src/ZipArchivePanel.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/ZipArchivePanel$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/ZipArchivePanel$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/ZipArchivePanel.cpp"

src/$(IntermediateDirectory)/ZipEntryListPanel$(ObjectSuffix): src/ZipEntryListPanel.cpp src/$(IntermediateDirectory)/ZipEntryListPanel$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/ZipEntryListPanel.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/ZipEntryListPanel$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/ZipEntryListPanel$(DependSuffix): src/ZipEntryListPanel.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/ZipEntryListPanel$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/ZipEntryListPanel$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/ZipEntryListPanel.cpp"

src/$(IntermediateDirectory)/ArchiveManagerPanel$(ObjectSuffix): src/ArchiveManagerPanel.cpp src/$(IntermediateDirectory)/ArchiveManagerPanel$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/ArchiveManagerPanel.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/ArchiveManagerPanel$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/ArchiveManagerPanel$(DependSuffix): src/ArchiveManagerPanel.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/ArchiveManagerPanel$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/ArchiveManagerPanel$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/ArchiveManagerPanel.cpp"

src/$(IntermediateDirectory)/MainWindow$(ObjectSuffix): src/MainWindow.cpp src/$(IntermediateDirectory)/MainWindow$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/MainWindow.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/MainWindow$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/MainWindow$(DependSuffix): src/MainWindow.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/MainWindow$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/MainWindow$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/MainWindow.cpp"

src/$(IntermediateDirectory)/EntryOperations$(ObjectSuffix): src/EntryOperations.cpp src/$(IntermediateDirectory)/EntryOperations$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/EntryOperations.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/EntryOperations$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/EntryOperations$(DependSuffix): src/EntryOperations.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/EntryOperations$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/EntryOperations$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/EntryOperations.cpp"

src/$(IntermediateDirectory)/MapLine$(ObjectSuffix): src/MapLine.cpp src/$(IntermediateDirectory)/MapLine$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/MapLine.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/MapLine$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/MapLine$(DependSuffix): src/MapLine.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/MapLine$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/MapLine$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/MapLine.cpp"

src/$(IntermediateDirectory)/MapSector$(ObjectSuffix): src/MapSector.cpp src/$(IntermediateDirectory)/MapSector$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/MapSector.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/MapSector$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/MapSector$(DependSuffix): src/MapSector.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/MapSector$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/MapSector$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/MapSector.cpp"

src/$(IntermediateDirectory)/MapSide$(ObjectSuffix): src/MapSide.cpp src/$(IntermediateDirectory)/MapSide$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/MapSide.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/MapSide$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/MapSide$(DependSuffix): src/MapSide.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/MapSide$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/MapSide$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/MapSide.cpp"

src/$(IntermediateDirectory)/MapThing$(ObjectSuffix): src/MapThing.cpp src/$(IntermediateDirectory)/MapThing$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/MapThing.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/MapThing$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/MapThing$(DependSuffix): src/MapThing.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/MapThing$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/MapThing$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/MapThing.cpp"

src/$(IntermediateDirectory)/MapVertex$(ObjectSuffix): src/MapVertex.cpp src/$(IntermediateDirectory)/MapVertex$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/MapVertex.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/MapVertex$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/MapVertex$(DependSuffix): src/MapVertex.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/MapVertex$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/MapVertex$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/MapVertex.cpp"

src/$(IntermediateDirectory)/SLADEMap$(ObjectSuffix): src/SLADEMap.cpp src/$(IntermediateDirectory)/SLADEMap$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/SLADEMap.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/SLADEMap$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/SLADEMap$(DependSuffix): src/SLADEMap.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/SLADEMap$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/SLADEMap$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/SLADEMap.cpp"

src/$(IntermediateDirectory)/MapCanvas$(ObjectSuffix): src/MapCanvas.cpp src/$(IntermediateDirectory)/MapCanvas$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/MapCanvas.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/MapCanvas$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/MapCanvas$(DependSuffix): src/MapCanvas.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/MapCanvas$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/MapCanvas$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/MapCanvas.cpp"

src/$(IntermediateDirectory)/MapEditorWindow$(ObjectSuffix): src/MapEditorWindow.cpp src/$(IntermediateDirectory)/MapEditorWindow$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/MapEditorWindow.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/MapEditorWindow$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/MapEditorWindow$(DependSuffix): src/MapEditorWindow.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/MapEditorWindow$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/MapEditorWindow$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/MapEditorWindow.cpp"

src/$(IntermediateDirectory)/Archive$(ObjectSuffix): src/Archive.cpp src/$(IntermediateDirectory)/Archive$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/Archive.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/Archive$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/Archive$(DependSuffix): src/Archive.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/Archive$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/Archive$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/Archive.cpp"

src/$(IntermediateDirectory)/ArchiveEntry$(ObjectSuffix): src/ArchiveEntry.cpp src/$(IntermediateDirectory)/ArchiveEntry$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/ArchiveEntry.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/ArchiveEntry$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/ArchiveEntry$(DependSuffix): src/ArchiveEntry.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/ArchiveEntry$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/ArchiveEntry$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/ArchiveEntry.cpp"

src/$(IntermediateDirectory)/ArchiveManager$(ObjectSuffix): src/ArchiveManager.cpp src/$(IntermediateDirectory)/ArchiveManager$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/ArchiveManager.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/ArchiveManager$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/ArchiveManager$(DependSuffix): src/ArchiveManager.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/ArchiveManager$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/ArchiveManager$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/ArchiveManager.cpp"

src/$(IntermediateDirectory)/WadArchive$(ObjectSuffix): src/WadArchive.cpp src/$(IntermediateDirectory)/WadArchive$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/WadArchive.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/WadArchive$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/WadArchive$(DependSuffix): src/WadArchive.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/WadArchive$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/WadArchive$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/WadArchive.cpp"

src/$(IntermediateDirectory)/ZipArchive$(ObjectSuffix): src/ZipArchive.cpp src/$(IntermediateDirectory)/ZipArchive$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/ZipArchive.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/ZipArchive$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/ZipArchive$(DependSuffix): src/ZipArchive.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/ZipArchive$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/ZipArchive$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/ZipArchive.cpp"

src/$(IntermediateDirectory)/EntryType$(ObjectSuffix): src/EntryType.cpp src/$(IntermediateDirectory)/EntryType$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/EntryType.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/EntryType$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/EntryType$(DependSuffix): src/EntryType.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/EntryType$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/EntryType$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/EntryType.cpp"

src/$(IntermediateDirectory)/SImage$(ObjectSuffix): src/SImage.cpp src/$(IntermediateDirectory)/SImage$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/SImage.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/SImage$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/SImage$(DependSuffix): src/SImage.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/SImage$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/SImage$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/SImage.cpp"

src/$(IntermediateDirectory)/Palette$(ObjectSuffix): src/Palette.cpp src/$(IntermediateDirectory)/Palette$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/Palette.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/Palette$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/Palette$(DependSuffix): src/Palette.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/Palette$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/Palette$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/Palette.cpp"

src/$(IntermediateDirectory)/PaletteManager$(ObjectSuffix): src/PaletteManager.cpp src/$(IntermediateDirectory)/PaletteManager$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/PaletteManager.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/PaletteManager$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/PaletteManager$(DependSuffix): src/PaletteManager.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/PaletteManager$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/PaletteManager$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/PaletteManager.cpp"

src/$(IntermediateDirectory)/CTexture$(ObjectSuffix): src/CTexture.cpp src/$(IntermediateDirectory)/CTexture$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/CTexture.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/CTexture$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/CTexture$(DependSuffix): src/CTexture.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/CTexture$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/CTexture$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/CTexture.cpp"

src/$(IntermediateDirectory)/TextureXList$(ObjectSuffix): src/TextureXList.cpp src/$(IntermediateDirectory)/TextureXList$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/TextureXList.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/TextureXList$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/TextureXList$(DependSuffix): src/TextureXList.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/TextureXList$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/TextureXList$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/TextureXList.cpp"

src/$(IntermediateDirectory)/Icons$(ObjectSuffix): src/Icons.cpp src/$(IntermediateDirectory)/Icons$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/Icons.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/Icons$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/Icons$(DependSuffix): src/Icons.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/Icons$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/Icons$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/Icons.cpp"

src/$(IntermediateDirectory)/GLTexture$(ObjectSuffix): src/GLTexture.cpp src/$(IntermediateDirectory)/GLTexture$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/GLTexture.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/GLTexture$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/GLTexture$(DependSuffix): src/GLTexture.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/GLTexture$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/GLTexture$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/GLTexture.cpp"

src/$(IntermediateDirectory)/ColourBox$(ObjectSuffix): src/ColourBox.cpp src/$(IntermediateDirectory)/ColourBox$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/ColourBox.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/ColourBox$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/ColourBox$(DependSuffix): src/ColourBox.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/ColourBox$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/ColourBox$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/ColourBox.cpp"

src/$(IntermediateDirectory)/CTextureCanvas$(ObjectSuffix): src/CTextureCanvas.cpp src/$(IntermediateDirectory)/CTextureCanvas$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/CTextureCanvas.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/CTextureCanvas$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/CTextureCanvas$(DependSuffix): src/CTextureCanvas.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/CTextureCanvas$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/CTextureCanvas$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/CTextureCanvas.cpp"

src/$(IntermediateDirectory)/GfxCanvas$(ObjectSuffix): src/GfxCanvas.cpp src/$(IntermediateDirectory)/GfxCanvas$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/GfxCanvas.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/GfxCanvas$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/GfxCanvas$(DependSuffix): src/GfxCanvas.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/GfxCanvas$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/GfxCanvas$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/GfxCanvas.cpp"

src/$(IntermediateDirectory)/ListView$(ObjectSuffix): src/ListView.cpp src/$(IntermediateDirectory)/ListView$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/ListView.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/ListView$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/ListView$(DependSuffix): src/ListView.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/ListView$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/ListView$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/ListView.cpp"

src/$(IntermediateDirectory)/OGLCanvas$(ObjectSuffix): src/OGLCanvas.cpp src/$(IntermediateDirectory)/OGLCanvas$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/OGLCanvas.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/OGLCanvas$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/OGLCanvas$(DependSuffix): src/OGLCanvas.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/OGLCanvas$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/OGLCanvas$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/OGLCanvas.cpp"

src/$(IntermediateDirectory)/PaletteCanvas$(ObjectSuffix): src/PaletteCanvas.cpp src/$(IntermediateDirectory)/PaletteCanvas$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/PaletteCanvas.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/PaletteCanvas$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/PaletteCanvas$(DependSuffix): src/PaletteCanvas.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/PaletteCanvas$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/PaletteCanvas$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/PaletteCanvas.cpp"

src/$(IntermediateDirectory)/PaletteChooser$(ObjectSuffix): src/PaletteChooser.cpp src/$(IntermediateDirectory)/PaletteChooser$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/PaletteChooser.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/PaletteChooser$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/PaletteChooser$(DependSuffix): src/PaletteChooser.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/PaletteChooser$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/PaletteChooser$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/PaletteChooser.cpp"

src/$(IntermediateDirectory)/TextEditor$(ObjectSuffix): src/TextEditor.cpp src/$(IntermediateDirectory)/TextEditor$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/TextEditor.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/TextEditor$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/TextEditor$(DependSuffix): src/TextEditor.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/TextEditor$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/TextEditor$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/TextEditor.cpp"

src/$(IntermediateDirectory)/ConsolePanel$(ObjectSuffix): src/ConsolePanel.cpp src/$(IntermediateDirectory)/ConsolePanel$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/ConsolePanel.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/ConsolePanel$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/ConsolePanel$(DependSuffix): src/ConsolePanel.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/ConsolePanel$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/ConsolePanel$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/ConsolePanel.cpp"

src/$(IntermediateDirectory)/SplashWindow$(ObjectSuffix): src/SplashWindow.cpp src/$(IntermediateDirectory)/SplashWindow$(DependSuffix)
	@test -d src/Release || $(MakeDirCommand) src/Release
	$(CompilerName) $(SourceSwitch) "/home/simon/dev/slade/trunk/src/SplashWindow.cpp" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/SplashWindow$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/SplashWindow$(DependSuffix): src/SplashWindow.cpp
	@test -d src/Release || $(MakeDirCommand) src/Release
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/SplashWindow$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/SplashWindow$(DependSuffix) -MM "/home/simon/dev/slade/trunk/src/SplashWindow.cpp"


-include src/$(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) src/$(IntermediateDirectory)/GfxConvDialog$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/GfxConvDialog$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/GfxConvDialog$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/ModifyOffsetsDialog$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/ModifyOffsetsDialog$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/ModifyOffsetsDialog$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/PaletteDialog$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/PaletteDialog$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/PaletteDialog$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/Clipboard$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/Clipboard$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/Clipboard$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/Console$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/Console$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/Console$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/CVar$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/CVar$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/CVar$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/ListenerAnnouncer$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/ListenerAnnouncer$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/ListenerAnnouncer$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/MainApp$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/MainApp$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/MainApp$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/MemChunk$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/MemChunk$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/MemChunk$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/Misc$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/Misc$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/Misc$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/Tokenizer$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/Tokenizer$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/Tokenizer$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/WxStuff$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/WxStuff$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/WxStuff$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/OpenGL$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/OpenGL$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/OpenGL$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/ArchivePanel$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/ArchivePanel$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/ArchivePanel$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/EntryListPanel$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/EntryListPanel$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/EntryListPanel$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/DefaultEntryPanel$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/DefaultEntryPanel$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/DefaultEntryPanel$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/EntryPanel$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/EntryPanel$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/EntryPanel$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/GfxEntryPanel$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/GfxEntryPanel$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/GfxEntryPanel$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/MultiEntryPanel$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/MultiEntryPanel$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/MultiEntryPanel$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/PaletteEntryPanel$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/PaletteEntryPanel$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/PaletteEntryPanel$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/TextEntryPanel$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/TextEntryPanel$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/TextEntryPanel$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/TextureXEntryPanel$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/TextureXEntryPanel$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/TextureXEntryPanel$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/ZipArchivePanel$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/ZipArchivePanel$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/ZipArchivePanel$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/ZipEntryListPanel$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/ZipEntryListPanel$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/ZipEntryListPanel$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/ArchiveManagerPanel$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/ArchiveManagerPanel$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/ArchiveManagerPanel$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/MainWindow$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/MainWindow$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/MainWindow$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/EntryOperations$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/EntryOperations$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/EntryOperations$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/MapLine$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/MapLine$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/MapLine$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/MapSector$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/MapSector$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/MapSector$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/MapSide$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/MapSide$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/MapSide$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/MapThing$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/MapThing$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/MapThing$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/MapVertex$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/MapVertex$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/MapVertex$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/SLADEMap$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/SLADEMap$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/SLADEMap$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/MapCanvas$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/MapCanvas$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/MapCanvas$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/MapEditorWindow$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/MapEditorWindow$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/MapEditorWindow$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/Archive$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/Archive$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/Archive$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/ArchiveEntry$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/ArchiveEntry$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/ArchiveEntry$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/ArchiveManager$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/ArchiveManager$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/ArchiveManager$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/WadArchive$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/WadArchive$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/WadArchive$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/ZipArchive$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/ZipArchive$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/ZipArchive$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/EntryType$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/EntryType$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/EntryType$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/SImage$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/SImage$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/SImage$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/Palette$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/Palette$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/Palette$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/PaletteManager$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/PaletteManager$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/PaletteManager$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/CTexture$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/CTexture$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/CTexture$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/TextureXList$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/TextureXList$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/TextureXList$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/Icons$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/Icons$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/Icons$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/GLTexture$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/GLTexture$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/GLTexture$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/ColourBox$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/ColourBox$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/ColourBox$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/CTextureCanvas$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/CTextureCanvas$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/CTextureCanvas$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/GfxCanvas$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/GfxCanvas$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/GfxCanvas$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/ListView$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/ListView$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/ListView$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/OGLCanvas$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/OGLCanvas$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/OGLCanvas$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/PaletteCanvas$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/PaletteCanvas$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/PaletteCanvas$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/PaletteChooser$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/PaletteChooser$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/PaletteChooser$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/TextEditor$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/TextEditor$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/TextEditor$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/ConsolePanel$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/ConsolePanel$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/ConsolePanel$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/SplashWindow$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/SplashWindow$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/SplashWindow$(PreprocessSuffix)
	$(RM) $(OutputFile)


