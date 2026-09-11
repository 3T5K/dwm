/* See LICENSE file for copyright and license details. */

/* appearance */
static const unsigned int borderpx   = 3;        /* border pixel of windows */
static const unsigned int ogap       = 30;       /* gap on outer edges */
static const unsigned int igap       = 10;       /* gap between windows */
static const unsigned int mgap       = 10;       /* monocle layout gap */
static const unsigned int snap       = 32;       /* snap pixel */
static const unsigned int defaulttag = 1;        /* default tag on startup */
static const int showbar             = 1;        /* 0 means no bar */
static const int topbar              = 1;        /* 0 means bottom bar */
static const int vertpad             = 10;       /* vertical padding of bar */
static const int sidepad             = 10;       /* horizontal padding of bar */
static const char *fonts[]           = { "CaskaydiaCoveNerdFont:size=13", "monospace:size=10" };
static const char dmenufont[]        = "monospace:size=10";
static const char col_gray1[]        = "#222222";
static const char col_gray2[]        = "#444444";
static const char col_gray3[]        = "#bbbbbb";
static const char col_gray4[]        = "#eeeeee";
static const char col_cyan[]         = "#005577";
static const char *colors[][3]       = {
	/*                    fg         bg         border  */
	[SchemeNorm]     = { "#cba6f7", "#1e1e2e", "#595959" },
	[SchemeSel]      = { "#1e1e2e", "#cba6f7", "#cba6f7" },
	[SchemeStatus]   = { "#cba6f7", "#1e1e2e", "#000000" }, // Statusbar right {text,background,not used but cannot be empty}
	[SchemeTagsSel]  = { "#1e1e2e", "#cba6f7", "#000000" }, // Tagbar left selected {text,background,not used but cannot be empty}
	[SchemeTagsNorm] = { "#cba6f7", "#1e1e2e", "#000000" }, // Tagbar left unselected {text,background,not used but cannot be empty}
	[SchemeInfoSel]  = { "#1e1e2e", "#cba6f7", "#000000" }, // infobar middle  selected {text,background,not used but cannot be empty}
	[SchemeInfoNorm] = { "#cba6f7", "#1e1e2e", "#000000" }, // infobar middle  unselected {text,background,not used but cannot be empty}
};

static const TileFocusCallback tilefocusfallback
    = tilefocusdefaultfallback;

static const int tilefocuscfg
    = TileFocusMasterXrayAc
    | TileFocusMasterLast
    | TileFocusStackXrayAc
    | TileFocusStackLast
    | TileFocusVertOverflow
    | TileFocusHorzOverflow
    ;

static const TileMoveCallback tilemovefallback = NULL;

static const int tilemovecfg
    = TileMoveVertOverflow
    | TileMoveHorzOverflow
    | TileMoveMasterXDeny
    | TileMoveMasterBottom
    | TileMoveStackXDeny
    | TileMoveStackTop
    ;

static const int tmpnmdectil = 0;

static const int tmpnmcfg
    = TmpNmRespectNmaster
    | TmpNmHookUnmanage
    | TmpNmHookToggleFloating
    ;

static const TileNmCtlCallback tilenmctlfallback
    = tilenmctldefaultfallback;

static const int tilenmctlcfg
    = TileNmCtlMasterPrefer
    | TileNmCtlStackPrefer
    ;

/* tagging */
static const char *tags[] = { "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       tags mask     isfloating   isfullscreen   monitor */
	{ "Gimp",     NULL,       NULL,       0,            1,           0,             -1 },
	{ "Firefox",  NULL,       NULL,       1 << 8,       0,           0,             -1 },
};

/* layout(s) */
static const float mfact     = 0.5;  /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */
static const int refreshrate = 60;   /* refresh rate (per second) for client move/resize */

static const Layout layouts[] = {
	/*               symbol     arrange function */
	[LtTile]    = { "[]=",      tile    }, /* first entry is default */
	[LtFloat]   = { "><>",      NULL    }, /* no layout function means floating behavior */
	[LtMonocle] = { "[M]",      monocle },
};

static const Layout *layoutring0[] = {
    &layouts[LtMonocle],
    &layouts[LtTile],
    NULL
};
