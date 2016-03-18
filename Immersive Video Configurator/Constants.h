//
// Panofelx Configurator
// by Matthias Lindner
//
// Constants
//

#ifndef __PCCONSTANTS_H__
#define __PCCONSTANTS_H__

const uint32 OPEN_IMAGE						= 'opim';
const uint32 OPEN_MOVIE						= 'opmv';
const uint32 SAVE_IMAGE						= 'saim';
const uint32 SAVE_MOVIE						= 'samv';
const uint32 OPEN_FILE_PANEL				= 'ofpl';
const uint32 DRAG_TO_TRACKER				= 'drat';

const uint32 SAVE_FILE_PANEL				= 'sfpl';
const uint32 SAVE_FILE_PANEL_FORMAT			= 'sfpf';
const uint32 SAVE_FILE_PANEL_SETTINGS		= 'sfps';

//Messages for menu commands

const uint32 MENU_FILE_NEW					= 'MFnw';
const uint32 MENU_FILE_OPEN					= 'MFop';
const uint32 MENU_FILE_CLOSE				= 'MFcl';
const uint32 MENU_FILE_SAVE					= 'MFsv';
const uint32 MENU_FILE_SAVEAS				= 'MFsa';
const uint32 MENU_FILE_EXPORT				= 'MFex';
const uint32 MENU_FILE_PAGESETUP			= 'MFps';
const uint32 MENU_FILE_PRINT				= 'MFpr';
const uint32 MENU_FILE_QUIT					= 'MFqu';
const uint32 MENU_EDIT_PREFERENCES			= 'MEpr';

const uint32 MENU_FILE_SAVE_ENABLE			= 'MFSE';
const uint32 PLUGINLIST_SELECTION_CHANGED	= 'PLSc';


const uint32 MESSURE_MENU_PIXEL				= 'MMPi';
const uint32 MESSURE_MENU_PERCENT			= 'MMPr';

//Other Messages

const uint32 PLUGIN_WIN_QUIT				= 'PLWQ';
const uint32 OUTPUT_WIN_QUIT				= 'OUWQ';
const uint32 PREFERENCE_WIN_QUIT			= 'PRWQ';


const uint32 OUTPUT_HEIGHT_CHANGED			= 'OHCg';
const uint32 OUTPUT_WIDTH_CHANGED			= 'OWCg';

const uint32 SCALE							= 'Scal';
const uint32 EXPORT_IMAGE					= 'EXPI';

const uint32 TEST_REQUEST					= 'TeRe';
/*const uint32 AIGHT_HUNDRET_PERCENT				= 800;
const uint32 FOUR_HUNDRET_PERCENT				= 400;
const uint32 TWO_HUNDRET_PERCENT				= 200;
const uint32 ONE_HUNDRET_SEVENTY_FIVE_PERCENT	= 175;
const uint32 ONE_HUNDRET_FIFTY_PERCENT			= 150;
const uint32 ONE_HUNDRET_TWENTY_FIVE__PERCENT	= 125;
const uint32 ONE_HUNDRET_PERCENT				= 100;
const uint32 SEVENTY_FIVE_PERCENT				= 75;
const uint32 FIFTY_PERCENT						= 50;
const uint32 TWENTY_FIVE_PERCENT				= 25;*/

const uint32 FILE_SAVE						= 'FLsv';

const uint32 RECALC_LOOKUPTABLE				= 'RLT';
const uint32 CALC_NEW_LOOK_UP_TABLE			= 'NLUT';
const uint32 LOOK_UP_TABLE_CALCULATED		= 'LUTC';

const rgb_color white = {255, 255, 255, 255};
const rgb_color black = {0, 0,0, 255};
const rgb_color red = {255, 0,0, 255};

#endif
