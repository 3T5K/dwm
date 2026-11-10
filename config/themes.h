/*     ________                            
 *    /_  __/ /_  ___  ____ ___  ___  _____
 *     / / / __ \/ _ \/ __ `__ \/ _ \/ ___/
 *    / / / / / /  __/ / / / / /  __(__  ) 
 *   /_/ /_/ /_/\___/_/ /_/ /_/\___/____/  
 */

static const char *colors[][3] =
{
	/*                    fg         bg         border  */
	[SchemeNorm]     = { "#cba6f7", "#1e1e2e", "#595959" },
	[SchemeSel]      = { "#1e1e2e", "#cba6f7", "#cba6f7" },
	[SchemeStatus]   = { "#cba6f7", "#1e1e2e", "#000000" }, // Statusbar right {text,background,not used but cannot be empty}
	[SchemeTagsSel]  = { "#1e1e2e", "#cba6f7", "#000000" }, // Tagbar left selected {text,background,not used but cannot be empty}
	[SchemeTagsNorm] = { "#cba6f7", "#1e1e2e", "#000000" }, // Tagbar left unselected {text,background,not used but cannot be empty}
	[SchemeInfoSel]  = { "#1e1e2e", "#cba6f7", "#000000" }, // infobar middle  selected {text,background,not used but cannot be empty}
	[SchemeInfoNorm] = { "#cba6f7", "#1e1e2e", "#000000" }, // infobar middle  unselected {text,background,not used but cannot be empty}
    [SchemeSpad]     = { "#cba6f7", "#595959", "#000000" },
};
