/*       __ __               
 *      / //_/__  __  _______
 *     / ,< / _ \/ / / / ___/
 *    / /| /  __/ /_/ (__  ) 
 *   /_/ |_\___/\__, /____/  
 *             /____/        
 */

#define MOD   Mod4Mask
#define META  Mod1Mask
#define CTRL  ControlMask
#define SHIFT ShiftMask

#define CLK_LEFT   Button1
#define CLK_MIDDLE Button2
#define CLK_RIGHT  Button3

#define UI(x) { .ui = (x) }
#define I(x)  { .i  = (x) }
#define F(x)  { .f  = (x) }
#define V(x)  { .v  = (x) }
#define NIL   {0}

#define CMD(...) { .v = (const char *[]){ __VA_ARGS__, NULL } }

#define TAGKEYS(KEY,TAG)                               \
	{ MOD,            KEY, view,       UI(1 << TAG) }, \
	{ MOD|CTRL,       KEY, toggleview, UI(1 << TAG) }, \
	{ MOD|SHIFT,      KEY, tag,        UI(1 << TAG) }, \
	{ MOD|CTRL|SHIFT, KEY, toggletag,  UI(1 << TAG) },

static char        dmenumon[2] = "0";
static const char *dmenucmd[]  = { "dmenu_run", "-m", dmenumon, NULL };

static const Key keys[] =
{
    /* General */
	{ MOD,       XK_q, killclient,     NIL  },
    { MOD,       XK_b, togglebar,      NIL  },
	{ MOD,       XK_z, togglefloating, NIL  },
	{ MOD|SHIFT, XK_c, togglefullscr,  NIL  },
	{ MOD|SHIFT, XK_q, quit,           I(0) },
	{ MOD|SHIFT, XK_r, quit,           I(1) },

    /* Applications */
    { MOD, XK_w,      spawn, CMD("firefox-bin") },
    { MOD, XK_d,      spawn, V(dmenucmd)        },
    { MOD, XK_a,      spawn, CMD("i3lock")      },
    { MOD, XK_Return, spawn, CMD("alacritty")   },

    /* Multimon */
    { MOD,       XK_grave, swapmon,  NIL   },
	{ MOD,       XK_n,     focusmon, I(-1) },
	{ MOD,       XK_m,     focusmon, I(+1) },
	{ MOD|SHIFT, XK_n,     tagmon,   I(-1) },
	{ MOD|SHIFT, XK_m,     tagmon,   I(+1) },

    /* Layouts */
	{ MOD,       XK_t,   setlayout,    V(&layouts[LtTile   ]) },
	{ MOD|SHIFT, XK_z,   setlayout,    V(&layouts[LtFloat  ]) },
	{ MOD,       XK_g,   setlayout,    V(&layouts[LtMonocle]) },
    { MOD,       XK_c,   cyclebetween, V(layoutring0)         },
	{ MOD|SHIFT, XK_Tab, setlayout,    NIL                    },

    /* Changing Focus */
    { MOD, XK_o, cyclefocus, I(CyclePrev | CycleFloating) },
    { MOD, XK_p, cyclefocus, I(CycleNext | CycleFloating) },
    { MOD, XK_h, tilefocus,  I('W')                       },
    { MOD, XK_j, tilefocus,  I('S')                       },
    { MOD, XK_k, tilefocus,  I('N')                       },
    { MOD, XK_l, tilefocus,  I('E')                       },
    { MOD, XK_v, sp_focus,   NIL                          },

    /* Moving Windows */
    { MOD|SHIFT, XK_h, tilemove, I('W') },
    { MOD|SHIFT, XK_j, tilemove, I('S') },
    { MOD|SHIFT, XK_k, tilemove, I('N') },
    { MOD|SHIFT, XK_l, tilemove, I('E') },

    /* Tile Layout */
    { MOD,       XK_x, exstack,   NIL             },
	{ MOD|SHIFT, XK_v, tilezoom,  NIL             },
	{ MOD,       XK_i, tilenmctl, I(TileNmCtlInc) },
	{ MOD,       XK_u, tilenmctl, I(TileNmCtlDec) },
	{ MOD|CTRL,  XK_h, setmfact,  F(-0.05)        },
	{ MOD|CTRL,  XK_l, setmfact,  F(+0.05)        },

    /* Tags */
	TAGKEYS(XK_1, 0)
	TAGKEYS(XK_2, 1)
	TAGKEYS(XK_3, 2)
	TAGKEYS(XK_4, 3)
	TAGKEYS(XK_5, 4)
	TAGKEYS(XK_6, 5)
	TAGKEYS(XK_7, 6)
	TAGKEYS(XK_8, 7)
	TAGKEYS(XK_9, 8)

	{ MOD,       XK_Tab, view, NIL    },
	{ MOD,       XK_0,   view, UI(~0) },
	{ MOD|SHIFT, XK_0,   tag,  UI(~0) },

    /* Scratchpads */
    { META, XK_1, sp_toggle, V("term1") },
    { META, XK_2, sp_toggle, V("term2") },
    { META, XK_3, sp_toggle, V("term3") },
    { META, XK_4, sp_toggle, V("py1")   },
    { META, XK_5, sp_toggle, V("py2")   },
    { META, XK_6, sp_toggle, V("py3")   },
    { META, XK_7, sp_toggle, V("snd")   },
    { META, XK_8, sp_toggle, V("sys")   },
    { META, XK_9, sp_toggle, V("tor")   },
};

static const Button buttons[] =
{
	{ ClkTagBar, 0,   CLK_LEFT,  view,       NIL },
	{ ClkTagBar, 0,   CLK_RIGHT, toggleview, NIL },
	{ ClkTagBar, MOD, CLK_LEFT,  tag,        NIL },
	{ ClkTagBar, MOD, CLK_RIGHT, toggletag,  NIL },

	{ ClkLtSymbol, 0, CLK_LEFT,  cyclebetween, V(layoutring0) },
	{ ClkLtSymbol, 0, CLK_RIGHT, setlayout,    NIL            },

	{ ClkClientWin, MOD, CLK_LEFT,   movemouse,      NIL },
	{ ClkClientWin, MOD, CLK_MIDDLE, togglefloating, NIL },
	{ ClkClientWin, MOD, CLK_RIGHT,  resizemouse,    NIL },

	{ ClkWinTitle, 0, CLK_MIDDLE, tilezoom, NIL },

	{ ClkStatusText, 0, CLK_MIDDLE, spawn, V("alacritty") },
};
