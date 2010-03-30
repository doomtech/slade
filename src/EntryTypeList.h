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
xx(EDF_WAD,				"wad",				detectWad)
xx(EDF_MUS,				"mus",				detectMus)
xx(EDF_MIDI,			"midi",				detectMidi)
xx(EDF_MOD_IT,			"mod_it",			detectModIt)
xx(EDF_MOD_XM,			"mod_xm",			detectModXm)
xx(EDF_MOD_S3M,			"mod_s3m",			detectModS3m)
xx(EDF_MOD_MOD,			"mod_mod",			detectModMod)
xx(EDF_SND_DOOM,		"snd_doom",			detectSndDoom)
xx(EDF_SND_WAV,			"snd_wav",			detectSndWav)
xx(EDF_SND_SPEAKER,		"snd_speaker",		detectSndSpeaker)
xx(EDF_ANIMATED,		"animated",			detectAnimated)
xx(EDF_SWITCHES,		"switches",			detectSwitches)
xb(EDF_TEXT,			"text")
xx(EDF_PNG,				"png",				detectPng)
xx(EDF_BMP,				"bmp",				detectBmp)
xx(EDF_PCX,				"pcx",				detectPcx)
xx(EDF_TGA,				"tga",				detectTga)
xx(EDF_TIFF,			"tiff",				detectTiff)
xx(EDF_JPEG,			"jpeg",				detectJpeg)
// Doom gfx format		
xy(EDF_GFX_DOOM,		"gfx_doom",			detectDoomGfx,			loadDoomGfx)
// Doom flat format and other raw images: 64x64, 64x128, 128x128, 256x256
xy(EDF_GFX_FLAT,		"gfx_flat",			detectDoomFlat,			loadDoomFlat)
// 320x?
xy(EDF_GFX_AUTOPAGE,	"autopage",			detectDoomFlat,			loadDoomFlat)
// 320x200
xy(EDF_GFX_FULLSCREEN,	"fullscreen",		detectDoomFlat,			loadDoomFlat)
// 16x16, 32x64 or 48x48
xy(EDF_GFX_STRIFESU,	"strifestart",		detectDoomFlat,			loadDoomFlat)
// Hexen planar graphic
xy(EDF_GFX_PLANAR,		"planar",			detectPlanar,			loadPlanar)
// ZDoom Hexen notches
xy(EDF_GFX_4BITCHUNK,	"4bitchunk",		detect4bitChunk,		load4bitChunk)
// Alpha Doom gfx format
xy(EDF_GFX_DOOM_ALPHA,	"gfx_doom_alpha",	detectDoomGfxAlpha,		loadDoomGfxA)
// Alpha Doom Raw and Header format
xy(EDF_GFX_DOOM_ARAH,	"gfx_doom_arah",	detectDoomGfxArah,		loadDoomArah)
// Beta Doom gfx format
xy(EDF_GFX_DOOM_BETA,	"gfx_doom_beta",	detectDoomGfxBeta,		loadDoomGfxB)
// Alpha Doom snea format
xy(EDF_GFX_DOOM_SNEA,	"gfx_doom_snea",	detectDoomGfxSnea,		loadDoomSnea)
// 10x12
xy(EDF_GFX_DOOM_GNUM,	"gfx_doom_gnum",	detectDoomFlat,			loadDoomFlat)
// ZDoom IMGZ graphic
xy(EDF_GFX_IMGZ,		"gfx_imgz",			detectImgz,				loadImgz)
// Alpha font format (huf)
xy(EDF_FON0,			"hufont",			detectFont0,			loadFont0)
// ZDoom console fonts (FON1)
xy(EDF_FON1,			"fon1",				detectFont1,			loadFont1)
// ZDoom big fonts (FON2)
xy(EDF_FON2,			"fon2",				detectFont2,			loadFont2)
// Byte Map Fonts (BMF)
xy(EDF_BMF,				"bmf",				detectBMF,				loadBMF)
// Monochrome, monospaced VGA font (fnt)
xy(EDF_MONOFONT,		"monofont",			detectFontM,			loadFontM)

#undef xa
#undef xb
#undef xx
#undef xy
