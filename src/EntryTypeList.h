/**********************************************************************
 * List of entry types as defined by macros, so as to centralize all  *
 * that repetitive stuff into a single file and keep them all in sync *
 **********************************************************************/

xa(EDF_ANY,				"any",				0)
xx(EDF_PALETTE,			"palette",			detectPalette)
xx(EDF_TEXTUREX,		"texturex",			detectTextureX)
xx(EDF_PNAMES,			"pnames",			detectPnames)
xx(EDF_FNAMES,			"fnames",			detectPnames)
xx(EDF_ANSI,			"ansi",				detectAnsi)
xx(EDF_ANIMATED,		"animated",			detectAnimated)
xx(EDF_SWITCHES,		"switches",			detectSwitches)
xx(EDF_ZGLN,			"zgln",				detectZGLNodes)
xx(EDF_ZGL2,			"zgl2",				detectZGLNodes2)
xx(EDF_DMD,				"meshdmd",			detectDMD)
xx(EDF_MDL,				"meshmdl",			detectMDL)
xx(EDF_MD2,				"meshmd2",			detectMD2)
xx(EDF_MD3,				"meshmd3",			detectMD3)
xb(EDF_TEXT,			"text")
// Archive formats
xb(EDF_ARCHIVE_START,	"archivestart")
xx(EDF_WAD,				"wad",				detectWad)
xx(EDF_ZIP,				"zip",				detectZip)
xx(EDF_LIB,				"lib",				detectLib)
xx(EDF_DAT,				"dat",				detectDat)
xb(EDF_ARCHIVE_END,		"archiveend")
// Audio formats
xb(EDF_AUDIO_START,		"audiostart")
xx(EDF_MUS,				"mus",				detectMus)
xx(EDF_MIDI,			"midi",				detectMidi)
xx(EDF_MOD_IT,			"mod_it",			detectModIt)
xx(EDF_MOD_XM,			"mod_xm",			detectModXm)
xx(EDF_MOD_S3M,			"mod_s3m",			detectModS3m)
xx(EDF_MOD_MOD,			"mod_mod",			detectModMod)
xx(EDF_SND_DOOM,		"snd_doom",			detectSndDoom)
xx(EDF_SND_WAV,			"snd_wav",			detectSndWav)
xx(EDF_SND_OGG,			"snd_ogg",			detectSndOgg)
xx(EDF_SND_FLAC,		"snd_flac",			detectSndFlac)
xx(EDF_SND_SPEAKER,		"snd_speaker",		detectSndSpeaker)
xb(EDF_AUDIO_END,		"audioend")
// Graphic formats
xb(EDF_GRAPHIC_START,	"gfxstart")
xx(EDF_PNG,				"png",				detectPng)
xx(EDF_BMP,				"bmp",				detectBmp)
xx(EDF_GIF,				"gif",				detectGif)
xx(EDF_PCX,				"pcx",				detectPcx)
xx(EDF_TGA,				"tga",				detectTga)
xx(EDF_TIFF,			"tiff",				detectTiff)
xx(EDF_JPEG,			"jpeg",				detectJpeg)
xy(EDF_GFX_IMGZ,		"gfx_imgz",			detectImgz,				loadImgz)	// ZDoom IMGZ graphic
xy(EDF_FON0,			"hufont",			detectFont0,			loadFont0)	// Alpha font format (huf)
xy(EDF_FON1,			"fon1",				detectFont1,			loadFont1)	// ZDoom console fonts (FON1)
xy(EDF_FON2,			"fon2",				detectFont2,			loadFont2)	// ZDoom big fonts (FON2)
xy(EDF_BMF,				"bmf",				detectBMF,				loadBMF)	// Byte Map Fonts (BMF)
xy(EDF_MONOFONT,		"monofont",			detectFontM,			loadFontM)	// Monochrome, monospaced VGA font (fnt)
// Paletted graphic formats
xb(EDF_PALETTED_START,	"palettedstart")
xy(EDF_GFX_DOOM,		"gfx_doom",			detectDoomGfx,			loadDoomGfx)// Doom gfx format		
xy(EDF_GFX_FLAT,		"gfx_flat",			detectDoomFlat,			loadDoomFlat)// Doom flat format and other raw images: 64x64, 64x128, 128x128, 256x256
xy(EDF_GFX_AUTOPAGE,	"autopage",			detectDoomFlat,			loadDoomFlat)// 320x?
xy(EDF_GFX_FULLSCREEN,	"fullscreen",		detectDoomFlat,			loadDoomFlat)// 320x200
xy(EDF_GFX_STRIFESU,	"strifestart",		detectDoomFlat,			loadDoomFlat)// 16x16, 32x64 or 48x48
xy(EDF_GFX_LEGACY	,	"legacy",			detectDoomLegacy,		loadDoomLegacy)// Some Legacy impage format apparently?
xy(EDF_GFX_PLANAR,		"planar",			detectPlanar,			loadPlanar)	// Hexen planar graphic
xy(EDF_GFX_4BITCHUNK,	"4bitchunk",		detect4bitChunk,		load4bitChunk)// ZDoom Hexen notches
xy(EDF_GFX_DOOM_ALPHA,	"gfx_doom_alpha",	detectDoomGfxAlpha,		loadDoomGfxA)// Alpha Doom gfx format
xy(EDF_GFX_DOOM_ARAH,	"gfx_doom_arah",	detectDoomGfxArah,		loadDoomArah)// Alpha Doom Raw and Header format
xy(EDF_GFX_DOOM_BETA,	"gfx_doom_beta",	detectDoomGfxBeta,		loadDoomGfxB)// Beta Doom gfx format
xy(EDF_GFX_DOOM_SNEA,	"gfx_doom_snea",	detectDoomGfxSnea,		loadDoomSnea)// Alpha Doom snea format
xy(EDF_GFX_DOOM_GNUM,	"gfx_doom_gnum",	detectDoomFlat,			loadDoomFlat)// 10x12 raw
xy(EDF_SCSPRITE,		"scsprite",			detectSCSprite,			loadSCSprite)// Shadowcaster sprite format
xy(EDF_SCSPRITE,		"scwall",			detectSCWall,			loadSCWall)// Shadowcaster sprite format
xb(EDF_PALETTED_STOP,	"palettedstop")
xb(EDF_GRAPHIC_END,		"gfxend")

#undef xa
#undef xb
#undef xx
#undef xy
