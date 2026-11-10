/*     ________                            
 *    /_  __/ /_  ___  ____ ___  ___  _____
 *     / / / __ \/ _ \/ __ `__ \/ _ \/ ___/
 *    / / / / / /  __/ / / / / /  __(__  ) 
 *   /_/ /_/ /_/\___/_/ /_/ /_/\___/____/  
 */

#ifndef DWM_THEME
#  define DWM_THEME catppuccin
#endif

#define COL_NIL "#000000"

#if DWM_THEME == catppuccin
#  define COL_BORDER_ACTIVE       "#cba6f7"
#  define COL_BORDER_NORMAL       "#1e1e2e"
#  define COL_STATUSBAR_TEXT      "#cba6f7"
#  define COL_STATUSBAR_BG        "#1e1e2e"
#  define COL_TAGBAR_ACTIVE_TEXT  "#1e1e2e"
#  define COL_TAGBAR_ACTIVE_BG    "#cba6f7"
#  define COL_TAGBAR_NORMAL_TEXT  "#cba6f7"
#  define COL_TAGBAR_NORMAL_BG    "#1e1e2e"
#  define COL_INFOBAR_ACTIVE_TEXT "#1e1e2e"
#  define COL_INFOBAR_ACTIVE_BG   "#cba6f7"
#  define COL_INFOBAR_NORMAL_TEXT "#cba6f7"
#  define COL_INFOBAR_NORMAL_BG   "#1e1e2e"
#  define COL_SPAD_BORDER_ACTIVE  "#cba6f7"
#  define COL_SPAD_BORDER_NORMAL  "#595959"
#else
#  error "bad theme"
#endif

static const char *colors[][3] =
{
	[SchemeNorm    ] = { COL_NIL
                       , COL_NIL
                       , COL_BORDER_NORMAL
                       },
	[SchemeSel     ] = { COL_NIL
                       , COL_NIL
                       , COL_BORDER_ACTIVE
                       },
	[SchemeStatus  ] = { COL_STATUSBAR_TEXT
                       , COL_STATUSBAR_BG
                       , COL_NIL
                       },
	[SchemeTagsSel ] = { COL_TAGBAR_ACTIVE_TEXT
                       , COL_TAGBAR_ACTIVE_BG
                       , COL_NIL
                       },
	[SchemeTagsNorm] = { COL_TAGBAR_NORMAL_TEXT
                       , COL_TAGBAR_NORMAL_BG
                       , COL_NIL
                       },
	[SchemeInfoSel ] = { COL_INFOBAR_ACTIVE_TEXT
                       , COL_INFOBAR_ACTIVE_BG
                       , COL_NIL
                       },
	[SchemeInfoNorm] = { COL_INFOBAR_NORMAL_TEXT
                       , COL_INFOBAR_NORMAL_BG
                       , COL_NIL
                       },
    [SchemeSpad    ] = { COL_SPAD_BORDER_ACTIVE
                       , COL_SPAD_BORDER_NORMAL
                       , COL_NIL
                       },
};
