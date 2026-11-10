/*      _____                 __       __                    __    
 *     / ___/______________ _/ /______/ /_  ____  ____ _____/ /____
 *     \__ \/ ___/ ___/ __ `/ __/ ___/ __ \/ __ \/ __ `/ __  / ___/
 *    ___/ / /__/ /  / /_/ / /_/ /__/ / / / /_/ / /_/ / /_/ (__  ) 
 *   /____/\___/_/   \__,_/\__/\___/_/ /_/ .___/\__,_/\__,_/____/  
 *                                      /_/                        
 */

static const int          sp_exclusive_across_mons = 1;
static const int          sp_permit_fullscreen     = 1;
static const unsigned int sp_borderpx              = 3;
static SpConstMfact       sp_constant_mfacts[]     =
{
    { monocle, 1.0 },
    { NULL,    1.0 },
};

static const Spad sp_defs[] =
{
    {
        "term1",
        SP_C_NONE,
        SP_R_XYWH(0.1, 0.1, 0.8, 0.8),
        SP_M_PID,
        SP_CMD("alacritty")
    },
    {
        "term2",
        SP_C_NONE,
        SP_R_XYWH(0.1, 0.1, 0.8, 0.8),
        SP_M_PID,
        SP_CMD("alacritty")
    },
    {
        "term3",
        SP_C_NONE,
        SP_R_XYWH(0.1, 0.1, 0.8, 0.8),
        SP_M_PID,
        SP_CMD("alacritty")
    },
    {
        "py1",
        SP_C_NONE,
        SP_R_XYWH(0.1, 0.1, 0.8, 0.8),
        SP_M_PID,
        SP_CMD("alacritty", "-e", "ipython")
    },
    {
        "py2",
        SP_C_NONE,
        SP_R_XYWH(0.1, 0.1, 0.8, 0.8),
        SP_M_PID,
        SP_CMD("alacritty", "-e", "ipython")
    },
    {
        "py3",
        SP_C_NONE,
        SP_R_XYWH(0.1, 0.1, 0.8, 0.8),
        SP_M_PID,
        SP_CMD("alacritty", "-e", "ipython")
    },
    {
        "snd",
        SP_C_NONE,
        SP_R_XYWH(0.1, 0.1, 0.8, 0.8),
        SP_M_PID,
        SP_CMD("alacritty", "-e", "wiremix")
    },
    {
        "sys",
        SP_C_NONE,
        SP_R_XYWH(0.1, 0.1, 0.8, 0.8),
        SP_M_PID,
        SP_CMD("alacritty", "-e", "btop")
    },
    {
        "tor",
        SP_C_NONE,
        SP_R_GAP(10),
        SP_M_PID,
        SP_CMD("qbittorrent")
    },
};
