/*      ______                           __
 *     / ____/__  ____  ___  _________ _/ /
 *    / / __/ _ \/ __ \/ _ \/ ___/ __ `/ / 
 *   / /_/ /  __/ / / /  __/ /  / /_/ / /  
 *   \____/\___/_/ /_/\___/_/   \__,_/_/   
 */

static const unsigned int mgap            = 10  ;
static const unsigned int ogap            = 10  ;
static const unsigned int igap            = 5   ;
static const unsigned int sgap            = 10  ;
static const unsigned int defaulttag      = 1   ;
static const unsigned int borderpx        = 3   ;
static const unsigned int snap            = 32  ;
static const float        mfact           = 0.5 ;
static const int          topbar          = 1   ;
static const int          vertpad         = 0   ;
static const int          sidepad         = 0   ;
static const int          showbar         = 1   ;
static const int          nmaster         = 1   ;
static const int          resizehints     = 1   ;
static const int          lockfullscreen  = 1   ;
static const int          refreshrate     = 60  ;
static const int          tmpnmdectil     = 0   ;
static const int          swallowfloating = 0   ;

static const char *fonts[] =
{
    "CaskaydiaCoveNerdFont:size=14",
    "JetBrainsMonoNerdFont:size=13",
    "monospace:size=10"
};

static const char *tags[] =
{
    "I"   , "II"  , "III" ,
    "IV"  , "V"   , "VI"  ,
    "VII" , "VII" , "IX"  ,
};

static const Layout layouts[] =
{
	[LtTile]    = { "[]=", tile    },
	[LtFloat]   = { "><>", NULL    },
	[LtMonocle] = { "[M]", monocle },
};

static const Layout *layoutring0[] =
{
    &layouts[LtMonocle],
    &layouts[LtTile   ],
    NULL
};

static const RLPair rlmap[] =
{
    { .from = monocle, .to = tile },
    { .from = NULL,    .to = tile },
};

static const TileFocusCallback tilefocusfallback
    = tilefocusdefaultfallback;

static const TileNmCtlCallback tilenmctlfallback
    = tilenmctldefaultfallback;

static const TileMoveCallback  tilemovefallback
    = NULL;

static const TileFFBCallback   tileffbfallback
    = NULL;

static const TileZoomCallback  tilezoomfallback
    = zoom;

static const int tilefocuscfg
    = TileFocusMasterXrayAc
    | TileFocusMasterLast
    | TileFocusStackXrayAc
    | TileFocusStackLast
    | TileFocusVertOverflow
    | TileFocusHorzOverflow
    ;

static const int tilemovecfg
    = TileMoveVertOverflow
    | TileMoveHorzOverflow
    | TileMoveMasterXDeny
    | TileMoveMasterBottom
    | TileMoveStackXDeny
    | TileMoveStackTop
    ;

static const int tilezoomcfg
    = TileZoomMtSTop
    | TileZoomMtSExp
    | TileZoomStMTop
    | TileZoomStMExp
    ;

static const int tmpnmcfg
    = TmpNmPrefer
    | TmpNmRespectNmaster
    | TmpNmHookUnmanage
    | TmpNmHookToggleFloating
    ;

static const int tilenmctlcfg
    = TileNmCtlMasterPrefer
    | TileNmCtlStackPrefer
    ;

static const int rstackscfg
    = RStacksMasterReset
    | RStacksStackReset
    ;

static const int rmfactcfg
    = RMfactResetOnEmptyMaster
    | RMfactResetOnEmptyStack
    | RMfactHookArrangeMon
    ;

static const unsigned int rlcfg
    = RLHookUnmanage
    | RLHookToggleFloating
    ;
