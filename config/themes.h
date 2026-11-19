/*     ________                            
 *    /_  __/ /_  ___  ____ ___  ___  _____
 *     / / / __ \/ _ \/ __ `__ \/ _ \/ ___/
 *    / / / / / /  __/ / / / / /  __(__  ) 
 *   /_/ /_/ /_/\___/_/ /_/ /_/\___/____/  
 */

#ifndef DWM_THEME
#  define DWM_THEME 1
#endif

#define COL_NIL "#000000"

#if DWM_THEME == 1 /* Catppuccin */
#  define COL_STATUSBAR_TEXT                "#cba6f7"
#  define COL_STATUSBAR_BG                  "#1e1e2e"
#  define COL_TAGBAR_ACTIVE_TEXT            "#1e1e2e"
#  define COL_TAGBAR_ACTIVE_BG              "#cba6f7"
#  define COL_TAGBAR_NORMAL_TEXT            "#cba6f7"
#  define COL_TAGBAR_NORMAL_BG              "#1e1e2e"
#  define COL_INFOBAR_ACTIVE_TEXT           "#1e1e2e"
#  define COL_INFOBAR_ACTIVE_BG             "#cba6f7"
#  define COL_INFOBAR_NORMAL_TEXT           "#cba6f7"
#  define COL_INFOBAR_NORMAL_BG             "#1e1e2e"
#  define COL_SPAD_BORDER_ACTIVE            "#cba6f7"
#  define COL_SPAD_BORDER_NORMAL            "#595959"
#  define COL_FLOATLT_BORDER_ACTIVE         "#cba6f7"
#  define COL_FLOATLT_BORDER_NORMAL         "#1e1e2e"
#  define COL_FLOATWIN_BORDER_ACTIVE        "#cba6f7"
#  define COL_FLOATWIN_BORDER_NORMAL        "#1e1e2e"
#  define COL_MONOCLE_BORDER_ACTIVE         "#cba6f7"
#  define COL_MONOCLE_BORDER_NORMAL         "#1e1e2e"
#  define COL_TILE_MASTER_EXP_BORDER_ACTIVE "#cba6f7"
#  define COL_TILE_MASTER_EXP_BORDER_NORMAL "#1e1e2e"
#  define COL_TILE_MASTER_COL_BORDER_ACTIVE "#cba6f7"
#  define COL_TILE_MASTER_COL_BORDER_NORMAL "#1e1e2e"
#  define COL_TILE_STACK_EXP_BORDER_ACTIVE  "#cba6f7"
#  define COL_TILE_STACK_EXP_BORDER_NORMAL  "#1e1e2e"
#  define COL_TILE_STACK_COL_BORDER_ACTIVE  "#cba6f7"
#  define COL_TILE_STACK_COL_BORDER_NORMAL  "#1e1e2e"
#else
#  error "bad theme"
#endif

static const char *colors[][3] =
{
	[SchemeNorm         ] = { COL_NIL
                            , COL_NIL
                            , COL_NIL
                            },
	[SchemeSel          ] = { COL_NIL
                            , COL_NIL
                            , COL_NIL
                            },
	[SchemeStatus       ] = { COL_STATUSBAR_TEXT
                            , COL_STATUSBAR_BG
                            , COL_NIL
                            },
	[SchemeTagsSel      ] = { COL_TAGBAR_ACTIVE_TEXT
                            , COL_TAGBAR_ACTIVE_BG
                            , COL_NIL
                            },
	[SchemeTagsNorm     ] = { COL_TAGBAR_NORMAL_TEXT
                            , COL_TAGBAR_NORMAL_BG
                            , COL_NIL
                            },
	[SchemeInfoSel      ] = { COL_INFOBAR_ACTIVE_TEXT
                            , COL_INFOBAR_ACTIVE_BG
                            , COL_NIL
                            },
	[SchemeInfoNorm     ] = { COL_INFOBAR_NORMAL_TEXT
                            , COL_INFOBAR_NORMAL_BG
                            , COL_NIL
                            },
    [SchemeTileMasterExp] = { COL_TILE_MASTER_EXP_BORDER_ACTIVE
                            , COL_TILE_MASTER_EXP_BORDER_NORMAL
                            , COL_NIL
                            },
    [SchemeTileMasterCol] = { COL_TILE_MASTER_COL_BORDER_ACTIVE
                            , COL_TILE_MASTER_COL_BORDER_NORMAL
                            , COL_NIL
                            },
    [SchemeTileStackExp ] = { COL_TILE_STACK_EXP_BORDER_ACTIVE
                            , COL_TILE_STACK_EXP_BORDER_NORMAL
                            , COL_NIL
                            },
    [SchemeTileStackCol ] = { COL_TILE_STACK_COL_BORDER_ACTIVE
                            , COL_TILE_STACK_COL_BORDER_NORMAL
                            , COL_NIL
                            },
    [SchemeMonocle      ] = { COL_MONOCLE_BORDER_ACTIVE
                            , COL_MONOCLE_BORDER_NORMAL
                            , COL_NIL
                            },
    [SchemeSpad         ] = { COL_SPAD_BORDER_ACTIVE
                            , COL_SPAD_BORDER_NORMAL
                            , COL_NIL
                            },
    [SchemeFloatLt      ] = { COL_FLOATLT_BORDER_ACTIVE
                            , COL_FLOATLT_BORDER_NORMAL
                            , COL_NIL
                            },
    [SchemeFloatWin     ] = { COL_FLOATWIN_BORDER_ACTIVE
                            , COL_FLOATWIN_BORDER_NORMAL
                            , COL_NIL
                            },
};
