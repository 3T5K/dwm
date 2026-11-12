/* See LICENSE file for copyright and license details.
 *
 * dynamic window manager is designed like any other X client as well. It is
 * driven through handling X events. In contrast to other X clients, a window
 * manager selects for SubstructureRedirectMask on the root window, to receive
 * events about window (dis-)appearance. Only one X connection at a time is
 * allowed to select for this event mask.
 *
 * The event handlers of dwm are organized in an array which is accessed
 * whenever a new event has been fetched. This allows event dispatching
 * in O(1) time.
 *
 * Each child of the root window is called a client, except windows which have
 * set the override_redirect flag. Clients are organized in a linked client
 * list on each monitor, the focus history is remembered through a stack list
 * on each monitor. Each client contains a bit array to indicate the tags of a
 * client.
 *
 * Keys and tagging rules are organized as arrays and defined in config.h.
 *
 * To understand everything else, start reading main().
 */
#include <errno.h>
#include <locale.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif /* XINERAMA */
#include <X11/Xft/Xft.h>
#include <X11/Xlib-xcb.h>
#include <xcb/res.h>
#ifdef __OpenBSD__
#include <sys/sysctl.h>
#include <kvm.h>
#endif /* __OpenBSD */

#include "drw.h"
#include "util.h"

/* macros */
#define BUTTONMASK              (ButtonPressMask|ButtonReleaseMask)
#define CLEANMASK(mask)         (mask & ~(numlockmask|LockMask) & (ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask))
#define INTERSECT(x,y,w,h,m)    (MAX(0, MIN((x)+(w),(m)->wx+(m)->ww) - MAX((x),(m)->wx)) \
                               * MAX(0, MIN((y)+(h),(m)->wy+(m)->wh) - MAX((y),(m)->wy)))
#define ISVISIBLE(C)            ((C->tags & C->mon->tagset[C->mon->seltags]))
#define MOUSEMASK               (BUTTONMASK|PointerMotionMask)
#define WIDTH(X)                ((X)->w + 2 * (X)->bw)
#define HEIGHT(X)               ((X)->h + 2 * (X)->bw)
#define TAGMASK                 ((1 << LENGTH(tags)) - 1)
#define TEXTW(X)                (drw_fontset_getwidth(drw, (X)) + lrpad)

#define SP_CMD(...)                 (const char *[]){ __VA_ARGS__, NULL }
#define SP_ISSPAD(C)                (C->tags & (sp_tag_vis | sp_tag_hid))
#define SP_R_TILE                   { SP_R_TILE_TAG, {{0}}, 0, 0 }
#define SP_R_GAP(g)                 { SP_R_GAPS_TAG, {.gaps = {(g), (g), (g), (g)}}, 1, 1 }
#define SP_R_GAPS(t, b, l, r, w, h) { SP_R_GAPS_TAG, {.gaps = {(t), (b), (l), (r)}}, (w), (h) }
#define SP_R_MFACT(o, i, d)         { SP_R_MFACT_TAG, {.mfact = {(o), (i), (d)}}, 0, 1 }
#define SP_R_XYWH(x, y, w, h)       { SP_R_XY_TAG, {.xy = {(x), (y)}}, (w), (h) }
#define SP_M_PID                    { SP_M_PID_TAG, NULL }
#define SP_M_CLASS(class)           { SP_M_CLASS_TAG, (class) }
#define SP_M_INSTANCE(instance)     { SP_M_INSTANCE_TAG, (instance) }
#define SP_M_TITLE(title)           { SP_M_TITLE_TAG, (title) }

/* enums */
enum { CurNormal, CurResize, CurMove, CurLast }; /* cursor */
enum { SchemeNorm, SchemeSel, SchemeSpad, SchemeStatus, SchemeTagsSel,
    SchemeTagsNorm, SchemeInfoSel, SchemeInfoNorm }; /* color schemes */
enum { NetSupported, NetWMName, NetWMState, NetWMCheck,
       NetWMFullscreen, NetActiveWindow, NetWMWindowType,
       NetWMWindowTypeDialog, NetClientList, NetWMPid, NetLast }; /* EWMH atoms */
enum { WMProtocols, WMDelete, WMState, WMTakeFocus, WMLast }; /* default atoms */
enum { ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle,
       ClkClientWin, ClkRootWin, ClkLast }; /* clicks */
enum { XrayIn, XrayAc };
enum { XrayGrpNone, XrayGrpMonocle, XrayGrpTileMaster, XrayGrpTileStack };
enum {
    SP_R_TILE_TAG,
    SP_R_GAPS_TAG,
    SP_R_MFACT_TAG,
    SP_R_XY_TAG
};
enum {
    SP_R_MFACT_M = 0,
    SP_R_MFACT_S = 1
};
enum {
    SP_M_PID_TAG,
    SP_M_CLASS_TAG,
    SP_M_INSTANCE_TAG,
    SP_M_TITLE_TAG
};
enum {
    SP_C_NONE   = 0,
    SP_C_NEW    = 1 << 0,
    SP_C_LAYOUT = 1 << 1,
    SP_C_CLOSE  = 1 << 2,
};
enum {
    TmpNmRespectNmaster     = 1 << 0,
    TmpNmHookUnmanage       = 1 << 1,
    TmpNmHookToggleFloating = 1 << 2,
    TmpNmHookSetFullscreen  = 1 << 3,
    TmpNmPrefer             = 1 << 4,
    TmpNmDefer              = 1 << 5,
    TmpNmNone               = 1 << 6,
    TmpNmAndDeny            = 1 << 7,
    TmpNmNoMove             = 1 << 8,
};
enum {
    TileZoomMtSTop = 1 << 0,
    TileZoomMtSBtm = 1 << 1,
    TileZoomMtSFoc = 1 << 2,
    TileZoomMtMTop = 1 << 3,
    TileZoomMtMBtm = 1 << 4,
    TileZoomMtXDny = 1 << 5,
    TileZoomStMTop = 1 << 6,
    TileZoomStMBtm = 1 << 7,
    TileZoomStMFoc = 1 << 8,
    TileZoomStSTop = 1 << 9,
    TileZoomStSBtm = 1 << 10,
    TileZoomStXDny = 1 << 11,
    TileZoomMtSExp = 1 << 12,
    TileZoomStMExp = 1 << 13,
};
enum {
    TileNmCtlDec       = 1 << 0,
    TileNmCtlInc       = 1 << 1,
    TileNmCtlNoArrange = 1 << 2,
};
enum {
    TileNmCtlMasterDefer   = 1 << 0,
    TileNmCtlMasterPrefer  = 1 << 1,
    TileNmCtlMasterNone    = 1 << 2,
    TileNmCtlMasterAndDeny = 1 << 3,
    TileNmCtlMasterNoMove  = 1 << 4,
    TileNmCtlStackDefer    = 1 << 5,
    TileNmCtlStackPrefer   = 1 << 6,
    TileNmCtlStackNone     = 1 << 7,
    TileNmCtlStackAndDeny  = 1 << 8,
    TileNmCtlStackNoMove   = 1 << 9,
};
enum {
    TileMoveVertOverflow = 1 << 0,
    TileMoveHorzOverflow = 1 << 1,
    TileMoveMasterTop    = 1 << 2,
    TileMoveMasterBottom = 1 << 3,
    TileMoveStackTop     = 1 << 4,
    TileMoveStackBottom  = 1 << 5,
    TileMoveMasterXDeny  = 1 << 6,
    TileMoveStackXDeny   = 1 << 7,
};
enum {
    TileFocusVertOverflow = 1 << 0,
    TileFocusHorzOverflow = 1 << 1,
    TileFocusMasterTop    = 1 << 2,
    TileFocusMasterBottom = 1 << 3,
    TileFocusMasterLast   = 1 << 4,
    TileFocusStackTop     = 1 << 5,
    TileFocusStackBottom  = 1 << 6,
    TileFocusStackLast    = 1 << 7,
    TileFocusMasterXrayAc = 1 << 8,
    TileFocusStackXrayAc  = 1 << 9,
};

typedef union {
	int i;
	unsigned int ui;
	float f;
	const void *v;
} Arg;

typedef struct {
	unsigned int click;
	unsigned int mask;
	unsigned int button;
	void (*func)(const Arg *arg);
	const Arg arg;
} Button;

typedef struct {
    int master;
    int stack;
} ExStack;

typedef struct Monitor Monitor;
typedef struct Client Client;
struct Client {
	char name[256];
	float mina, maxa;
	int x, y, w, h;
	int oldx, oldy, oldw, oldh;
	int basew, baseh, incw, inch, maxw, maxh, minw, minh, hintsvalid;
	int bw, oldbw;
	unsigned int tags;
	int isfixed, isfloating, isurgent, neverfocus, oldstate, isfullscreen, isterminal, noswallow;
	pid_t pid;
    int xraygrp, xraystat;
	Client *next;
	Client *snext;
	Client *swallowing;
	Monitor *mon;
	Window win;
};

typedef struct {
	unsigned int mod;
	KeySym keysym;
	void (*func)(const Arg *);
	const Arg arg;
} Key;

typedef struct {
	const char *symbol;
	void (*arrange)(Monitor *);
} Layout;

typedef struct Pertag Pertag;
struct Monitor {
	char ltsymbol[16];
	float mfact;
	int nmaster;
	int num;
	int by;               /* bar geometry */
	int mx, my, mw, mh;   /* screen size */
	int wx, wy, ww, wh;   /* window area  */
	unsigned int seltags;
	unsigned int sellt;
	unsigned int tagset[2];
	int showbar;
	int topbar;
	Client *clients;
	Client *sel;
	Client *stack;
	Monitor *next;
	Window barwin;
	const Layout *lt[2];
	Pertag *pertag;
};

typedef struct {
	const char *class;
	const char *instance;
	const char *title;
	unsigned int tags;
	int isfloating;
	int isterminal;
	int noswallow;
	int monitor;
} Rule;

typedef struct {
    void (*arrange)(Monitor *);
    float mfact;
} SpConstMfact;

typedef struct {
    int tag;
    union {
        struct { int t, d, l, r; } gaps;
        struct { int o, i, d; } mfact;
        struct { float x, y; } xy;
    } val;
    float w, h;
} SpRules;

typedef struct {
    int tag;
    const char *val;
} SpMatch;

typedef struct {
    const char *name;
    unsigned int close;
    SpRules rules;
    SpMatch match;
    const char **cmd;
} Spad;

typedef struct {
    const Spad *spad;
    pid_t pid;
    Client *client;
} SpState;

typedef struct {
    Client *bs[2]; /* bs[0]: bottom master client bs[1]: bottom stack client */
    Client *ts[2]; /* ts[0]: top master client    ts[1]: top stack client    */
    Client *cs[2]; /* cs[0]: previous client      cs[1]: next client         */
    Client *xs[2]; /* xs[0]: master XrayAc        xs[1]: stack XrayAc        */
    int     nc   ; /* client count                                           */
    int     si   ; /* selected client index (from 0)                         */
    int     st   ; /* is selected client in stack                            */
} TileInfo;

typedef void(*TileZoomCallback)(const Arg *);

typedef void(*TileNmCtlCallback)(const Arg *);

typedef void (*TileMoveCallback)(const Arg *);

typedef void(*TileFocusCallback)(const Arg *);

typedef Client *(*TileFFBCallback)(Client *);

/* function declarations */
static void applyrules(Client *c);
static int applysizehints(Client *c, int *x, int *y, int *w, int *h, int interact);
static void arrange(Monitor *m);
static void arrangemon(Monitor *m);
static void attach(Client *c);
static void attachbottom(Client *c);
static void attachstack(Client *c);
static void buttonpress(XEvent *e);
static void checkotherwm(void);
static void cleanup(void);
static void cleanupmon(Monitor *mon);
static void clientmessage(XEvent *e);
static void cmove(Client *what, Client *where, int dir);
static void configure(Client *c);
static void configurenotify(XEvent *e);
static void configurerequest(XEvent *e);
static Client *cprev(Client *c);
static Client *cprevtiled(Client *c);
static Monitor *createmon(void);
static void cswap(Client *x, Client *y);
static void destroynotify(XEvent *e);
static void detach(Client *c);
static void detachstack(Client *c);
static Monitor *dirtomon(int dir);
static void drawbar(Monitor *m);
static void drawbars(void);
static int drawstatusbar(Monitor *m, int bh, char* text);
static void enternotify(XEvent *e);
static void expose(XEvent *e);
static void exstack(const Arg *arg);
static void focus(Client *c);
static void focusin(XEvent *e);
static void focusmon(const Arg *arg);
static void focusrestack(Client *c);
static void focusstack(const Arg *arg);
static Atom getatomprop(Client *c, Atom prop);
static int getrootptr(int *x, int *y);
static long getstate(Window w);
static int gettextprop(Window w, Atom atom, char *text, unsigned int size);
static void grabbuttons(Client *c, int focused);
static void grabkeys(void);
static void incnmaster(const Arg *arg);
static void keypress(XEvent *e);
static void killclient(const Arg *arg);
static void manage(Window w, XWindowAttributes *wa);
static void mappingnotify(XEvent *e);
static void maprequest(XEvent *e);
static void monocle(Monitor *m);
static void motionnotify(XEvent *e);
static void movemouse(const Arg *arg);
static Client *nexttiled(Client *c);
static void pop(Client *c);
static void propertynotify(XEvent *e);
static void quit(const Arg *arg);
static Monitor *recttomon(int x, int y, int w, int h);
static void resize(Client *c, int x, int y, int w, int h, int interact);
static void resizeclient(Client *c, int x, int y, int w, int h);
static void resizemouse(const Arg *arg);
static void restack(Monitor *m);
static void run(void);
static void scan(void);
static int sendevent(Client *c, Atom proto);
static void sendmon(Client *c, Monitor *m);
static void setclientstate(Client *c, long state);
static void setfocus(Client *c);
static void setfullscreen(Client *c, int fullscreen);
static void setlayout(const Arg *arg);
static void setmfact(const Arg *arg);
static void setup(void);
static void seturgent(Client *c, int urg);
static void showhide(Client *c);
static void sighup(int unused);
static void sigterm(int unused);
static void spawn(const Arg *arg);
static SpState *sp_active(Monitor *m);
static void sp_configure(Client *c, SpRules r);
static void sp_focus(const Arg *arg);
static void sp_hide(Monitor *m);
static SpState *sp_match(Client *c);
static SpState *sp_match_class_hint(Client *c);
static SpState *sp_match_pid(Client *c);
static SpState *sp_match_title(Client *c);
static pid_t sp_spawn(const Spad *spad);
static SpState *sp_state(Client *c);
static void sp_toggle(const Arg *arg);
static void tag(const Arg *arg);
static void tagmon(const Arg *arg);
static void tile(Monitor *m);
static Client *tileffb(Client *c);
static void tilefocus(const Arg *arg);
static void tilefocusdefaultfallback(const Arg *arg);
static TileInfo tileinfo(void);
static void tilenmctl(const Arg *arg);
static void tilenmctldefaultfallback(const Arg *arg);
static void tilemove(const Arg *arg);
static void tilezoom(const Arg *arg);
static void tmpnmdec(Client *c, int arg);
static void togglebar(const Arg *arg);
static void togglefloating(const Arg *arg);
static void togglefullscr(const Arg *arg);
static void toggletag(const Arg *arg);
static void toggleview(const Arg *arg);
static void unfocus(Client *c, int setfocus);
static void unmanage(Client *c, int destroyed);
static void unmapnotify(XEvent *e);
static void updatebarpos(Monitor *m);
static void updatebars(void);
static void updateclientlist(void);
static int updategeom(void);
static void updatenumlockmask(void);
static void updatesizehints(Client *c);
static void updatestatus(void);
static void updatetitle(Client *c);
static void updatewindowtype(Client *c);
static void updatewmhints(Client *c);
static void view(const Arg *arg);
static Client *wintoclient(Window w);
static Monitor *wintomon(Window w);
static int xerror(Display *dpy, XErrorEvent *ee);
static int xerrordummy(Display *dpy, XErrorEvent *ee);
static int xerrorstart(Display *dpy, XErrorEvent *ee);
static Client *xrayac(Monitor *mon, int grp);
static Client *xrayacs(Monitor *mon, int grp, Client *skip);
static void xrayapply(Client *c);
static void xraycfg(Client *c, int grp, int stat);
static void xraydefhandler(Client *c);
static Client *xrayfallback(Monitor *mon, int grp);
static void xrayfocus(Client *c);
static Client *xraymaybefallback(Monitor *mon, int grp);
static void xrayunfocus(Client *c);
static void zoom(const Arg *arg);
static void swapmon(const Arg *arg);

static pid_t getparentprocess(pid_t p);
static int isdescprocess(pid_t p, pid_t c);
static Client *swallowingclient(Window w);
static Client *termforwin(const Client *c);
static pid_t winpid(Window w);

/* variables */
static Client *lastfocused = NULL;
static const char broken[] = "broken";
static char stext[1024];
static int screen;
static int sw, sh;           /* X display screen geometry width, height */
static int bh;               /* bar height */
static int lrpad;            /* sum of left and right padding for text */
static int vp;               /* vertical padding for bar */
static int sp;               /* side padding for bar */
static int (*xerrorxlib)(Display *, XErrorEvent *);
static unsigned int numlockmask = 0;
static void (*handler[LASTEvent]) (XEvent *) = {
	[ButtonPress] = buttonpress,
	[ClientMessage] = clientmessage,
	[ConfigureRequest] = configurerequest,
	[ConfigureNotify] = configurenotify,
	[DestroyNotify] = destroynotify,
	[EnterNotify] = enternotify,
	[Expose] = expose,
	[FocusIn] = focusin,
	[KeyPress] = keypress,
	[MappingNotify] = mappingnotify,
	[MapRequest] = maprequest,
	[MotionNotify] = motionnotify,
	[PropertyNotify] = propertynotify,
	[UnmapNotify] = unmapnotify
};
static Atom wmatom[WMLast], netatom[NetLast];
static int restart = 0;
static int running = 1;
static Cur *cursor[CurLast];
static Clr **scheme;
static Display *dpy;
static Drw *drw;
static Monitor *mons, *selmon;
static Window root, wmcheckwin;
static void (*xrayhandlers[])(Client *) = {
    [XrayGrpNone]       = NULL,
    [XrayGrpMonocle]    = xraydefhandler,
    [XrayGrpTileMaster] = xraydefhandler,
    [XrayGrpTileStack]  = xraydefhandler,
};

static xcb_connection_t *xcon;

/* configuration, allows nested code to access above variables */
#include "config.h"

struct Pertag {
	unsigned int curtag, prevtag; /* current and previous tag */
	int nmasters[LENGTH(tags) + 1]; /* number of windows in master area */
	float mfacts[LENGTH(tags) + 1]; /* mfacts per tag */
	unsigned int sellts[LENGTH(tags) + 1]; /* selected layouts */
	const Layout *ltidxs[LENGTH(tags) + 1][2]; /* matrix of tags and layouts indexes  */
	int showbars[LENGTH(tags) + 1]; /* display bar for the current tag */
    ExStack exstacks[LENGTH(tags) + 1]; /* states of stacks in a tag with the tile layout */
};

/* compile-time check if all tags fit into an unsigned int bit array. */
struct NumTags { char limitexceeded[LENGTH(tags) > 31 - 2 ? -1 : 1]; };

static const unsigned int sp_tag_vis = 1 << LENGTH(tags);
static const unsigned int sp_tag_hid = sp_tag_vis << 1;
static SpState sp_states[LENGTH(sp_defs)] = {0};
static int sp_toggled = 0;
static int sp_repaint = 1;

/* function implementations */
void
applyrules(Client *c)
{
	const char *class, *instance;
	unsigned int i;
	const Rule *r;
	Monitor *m;
	XClassHint ch = { NULL, NULL };

	/* rule matching */
	c->isfloating = 0;
	c->tags = 0;
	XGetClassHint(dpy, c->win, &ch);
	class    = ch.res_class ? ch.res_class : broken;
	instance = ch.res_name  ? ch.res_name  : broken;

	for (i = 0; i < LENGTH(rules); i++) {
		r = &rules[i];
		if ((!r->title || strstr(c->name, r->title))
		&& (!r->class || strstr(class, r->class))
		&& (!r->instance || strstr(instance, r->instance)))
		{
			c->isterminal = r->isterminal;
			c->noswallow  = r->noswallow;
			c->isfloating = r->isfloating;
			c->tags |= r->tags;
			for (m = mons; m && m->num != r->monitor; m = m->next);
			if (m)
				c->mon = m;
		}
	}
	if (ch.res_class)
		XFree(ch.res_class);
	if (ch.res_name)
		XFree(ch.res_name);
	c->tags = c->tags & TAGMASK ? c->tags & TAGMASK : c->mon->tagset[c->mon->seltags] & ~sp_tag_vis;
}

int
applysizehints(Client *c, int *x, int *y, int *w, int *h, int interact)
{
	int baseismin;
	Monitor *m = c->mon;

	/* set minimum possible */
	*w = MAX(1, *w);
	*h = MAX(1, *h);
	if (interact) {
		if (*x > sw)
			*x = sw - WIDTH(c);
		if (*y > sh)
			*y = sh - HEIGHT(c);
		if (*x + *w + 2 * c->bw < 0)
			*x = 0;
		if (*y + *h + 2 * c->bw < 0)
			*y = 0;
	} else {
		if (*x >= m->wx + m->ww)
			*x = m->wx + m->ww - WIDTH(c);
		if (*y >= m->wy + m->wh)
			*y = m->wy + m->wh - HEIGHT(c);
		if (*x + *w + 2 * c->bw <= m->wx)
			*x = m->wx;
		if (*y + *h + 2 * c->bw <= m->wy)
			*y = m->wy;
	}
	if (*h < bh)
		*h = bh;
	if (*w < bh)
		*w = bh;
	if (resizehints || c->isfloating || !c->mon->lt[c->mon->sellt]->arrange) {
		if (!c->hintsvalid)
			updatesizehints(c);
		/* see last two sentences in ICCCM 4.1.2.3 */
		baseismin = c->basew == c->minw && c->baseh == c->minh;
		if (!baseismin) { /* temporarily remove base dimensions */
			*w -= c->basew;
			*h -= c->baseh;
		}
		/* adjust for aspect limits */
		if (c->mina > 0 && c->maxa > 0) {
			if (c->maxa < (float)*w / *h)
				*w = *h * c->maxa + 0.5;
			else if (c->mina < (float)*h / *w)
				*h = *w * c->mina + 0.5;
		}
		if (baseismin) { /* increment calculation requires this */
			*w -= c->basew;
			*h -= c->baseh;
		}
		/* adjust for increment value */
		if (c->incw)
			*w -= *w % c->incw;
		if (c->inch)
			*h -= *h % c->inch;
		/* restore base dimensions */
		*w = MAX(*w + c->basew, c->minw);
		*h = MAX(*h + c->baseh, c->minh);
		if (c->maxw)
			*w = MIN(*w, c->maxw);
		if (c->maxh)
			*h = MIN(*h, c->maxh);
	}
	return *x != c->x || *y != c->y || *w != c->w || *h != c->h;
}

void
arrange(Monitor *m)
{
	if (m)
		showhide(m->stack);
	else for (m = mons; m; m = m->next)
		showhide(m->stack);
	if (m) {
		arrangemon(m);
		restack(m);
	} else for (m = mons; m; m = m->next)
		arrangemon(m);
}

void
arrangemon(Monitor *m)
{
    Client *c;

	strncpy(m->ltsymbol, m->lt[m->sellt]->symbol, sizeof m->ltsymbol);
    if (m->lt[m->sellt]->arrange)
        m->lt[m->sellt]->arrange(m);
    else
        for (c = nexttiled(m->clients); c; c = nexttiled(c->next))
            xraycfg(c, XrayGrpNone, -1);
}

void
attach(Client *c)
{
	c->next = c->mon->clients;
	c->mon->clients = c;
}

void
attachbottom(Client *c)
{
	Client **tc;
	c->next = NULL;
	for (tc = &c->mon->clients; *tc; tc = &(*tc)->next);
	*tc = c;
}

void
attachstack(Client *c)
{
	c->snext = c->mon->stack;
	c->mon->stack = c;
}

void
swallow(Client *p, Client *c)
{

	if (c->noswallow || c->isterminal)
		return;
	if (c->noswallow && !swallowfloating && c->isfloating)
		return;

	detach(c);
	detachstack(c);

	setclientstate(c, WithdrawnState);
	XUnmapWindow(dpy, p->win);

	p->swallowing = c;
	c->mon = p->mon;

	Window w = p->win;
	p->win = c->win;
	c->win = w;
	updatetitle(p);
	XMoveResizeWindow(dpy, p->win, p->x, p->y, p->w, p->h);
	arrange(p->mon);
	configure(p);
	updateclientlist();
}

void
unswallow(Client *c)
{
	c->win = c->swallowing->win;

	free(c->swallowing);
	c->swallowing = NULL;

	/* unfullscreen the client */
	setfullscreen(c, 0);
	updatetitle(c);
	arrange(c->mon);
	XMapWindow(dpy, c->win);
	XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
	setclientstate(c, NormalState);
	focus(NULL);
	arrange(c->mon);
}

void
buttonpress(XEvent *e)
{
	unsigned int i, x, click;
	Arg arg = {0};
	Client *c;
	Monitor *m;
	XButtonPressedEvent *ev = &e->xbutton;

	click = ClkRootWin;
	/* focus monitor if necessary */
	if ((m = wintomon(ev->window)) && m != selmon) {
		unfocus(selmon->sel, 1);
		selmon = m;
		focus(NULL);
	}
	if (ev->window == selmon->barwin) {
		i = x = 0;
		do
			x += TEXTW(tags[i]);
		while (ev->x >= x && ++i < LENGTH(tags));
		if (i < LENGTH(tags)) {
			click = ClkTagBar;
			arg.ui = 1 << i;
		} else if (ev->x < x + TEXTW(selmon->ltsymbol))
			click = ClkLtSymbol;
		else if (ev->x > selmon->ww - (int)TEXTW(stext))
			click = ClkStatusText;
		else
			click = ClkWinTitle;
	} else if ((c = wintoclient(ev->window))) {
		focus(c);
		restack(selmon);
		XAllowEvents(dpy, ReplayPointer, CurrentTime);
		click = ClkClientWin;
	}
	for (i = 0; i < LENGTH(buttons); i++)
		if (click == buttons[i].click && buttons[i].func && buttons[i].button == ev->button
		&& CLEANMASK(buttons[i].mask) == CLEANMASK(ev->state))
			buttons[i].func(click == ClkTagBar && buttons[i].arg.i == 0 ? &arg : &buttons[i].arg);
}

void
checkotherwm(void)
{
	xerrorxlib = XSetErrorHandler(xerrorstart);
	/* this causes an error if some other window manager is running */
	XSelectInput(dpy, DefaultRootWindow(dpy), SubstructureRedirectMask);
	XSync(dpy, False);
	XSetErrorHandler(xerror);
	XSync(dpy, False);
}

void
cleanup(void)
{
	Arg a = {.ui = ~0};
	Layout foo = { "", NULL };
	Monitor *m;
	size_t i;

	view(&a);
	selmon->lt[selmon->sellt] = &foo;
	for (m = mons; m; m = m->next)
		while (m->stack)
			unmanage(m->stack, 0);
	XUngrabKey(dpy, AnyKey, AnyModifier, root);
	while (mons)
		cleanupmon(mons);
	for (i = 0; i < CurLast; i++)
		drw_cur_free(drw, cursor[i]);
	for (i = 0; i < LENGTH(colors) + 1; i++)
		drw_scm_free(drw, scheme[i], 3);
	free(scheme);
	XDestroyWindow(dpy, wmcheckwin);
	drw_free(drw);
	XSync(dpy, False);
	XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
	XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
}

void
cleanupmon(Monitor *mon)
{
	Monitor *m;

	if (mon == mons)
		mons = mons->next;
	else {
		for (m = mons; m && m->next != mon; m = m->next);
		m->next = mon->next;
	}
	XUnmapWindow(dpy, mon->barwin);
	XDestroyWindow(dpy, mon->barwin);
	free(mon);
}

void
clientmessage(XEvent *e)
{
	XClientMessageEvent *cme = &e->xclient;
	Client *c = wintoclient(cme->window);

	if (!c)
		return;
	if (cme->message_type == netatom[NetWMState]) {
		if (cme->data.l[1] == netatom[NetWMFullscreen]
		|| cme->data.l[2] == netatom[NetWMFullscreen])
			setfullscreen(c, (cme->data.l[0] == 1 /* _NET_WM_STATE_ADD    */
				|| (cme->data.l[0] == 2 /* _NET_WM_STATE_TOGGLE */ && !c->isfullscreen)));
	} else if (cme->message_type == netatom[NetActiveWindow]) {
		if (c != selmon->sel && !c->isurgent)
			seturgent(c, 1);
	}
}

void
cmove(Client *what, Client *where, int dir)
{
    Client *pwhat, *pwhere, *nwhat;

    if ( !what || !where || what == where
         || (!dir && what->next  == where )
         || ( dir && where->next == what  ) )
        return;

    pwhat = cprev(what);
    nwhat = where;
    if (!dir)
        pwhere = cprev(where);

    if (pwhat)
        pwhat->next = what->next;
    else
        selmon->clients = what->next;

    if (dir) {
        nwhat = where->next;
        where->next = what;
    } else {
        if (pwhere)
            pwhere->next = what;
        else
            selmon->clients = what;
    }

    what->next = nwhat;
}

void
configure(Client *c)
{
	XConfigureEvent ce;

	ce.type = ConfigureNotify;
	ce.display = dpy;
	ce.event = c->win;
	ce.window = c->win;
	ce.x = c->x;
	ce.y = c->y;
	ce.width = c->w;
	ce.height = c->h;
	ce.border_width = c->bw;
	ce.above = None;
	ce.override_redirect = False;
	XSendEvent(dpy, c->win, False, StructureNotifyMask, (XEvent *)&ce);
}

void
configurenotify(XEvent *e)
{
	Monitor *m;
	Client *c;
	XConfigureEvent *ev = &e->xconfigure;
	int dirty;

	/* TODO: updategeom handling sucks, needs to be simplified */
	if (ev->window == root) {
		dirty = (sw != ev->width || sh != ev->height);
		sw = ev->width;
		sh = ev->height;
		if (updategeom() || dirty) {
			drw_resize(drw, sw, bh);
			updatebars();
			for (m = mons; m; m = m->next) {
				for (c = m->clients; c; c = c->next)
					if (c->isfullscreen)
						resizeclient(c, m->mx, m->my, m->mw, m->mh);
				XMoveResizeWindow(dpy, m->barwin, m->wx + sp, m->by + vp, m->ww -  2 * sp, bh);
			}
			focus(NULL);
			arrange(NULL);
		}
	}
}

void
configurerequest(XEvent *e)
{
	Client *c;
	Monitor *m;
	XConfigureRequestEvent *ev = &e->xconfigurerequest;
	XWindowChanges wc;

	if ((c = wintoclient(ev->window))) {
		if (ev->value_mask & CWBorderWidth)
			c->bw = ev->border_width;
		else if (c->isfloating || !selmon->lt[selmon->sellt]->arrange) {
			m = c->mon;
			if (ev->value_mask & CWX) {
				c->oldx = c->x;
				c->x = m->mx + ev->x;
			}
			if (ev->value_mask & CWY) {
				c->oldy = c->y;
				c->y = m->my + ev->y;
			}
			if (ev->value_mask & CWWidth) {
				c->oldw = c->w;
				c->w = ev->width;
			}
			if (ev->value_mask & CWHeight) {
				c->oldh = c->h;
				c->h = ev->height;
			}
			if ((c->x + c->w) > m->mx + m->mw && c->isfloating)
				c->x = m->mx + (m->mw / 2 - WIDTH(c) / 2); /* center in x direction */
			if ((c->y + c->h) > m->my + m->mh && c->isfloating)
				c->y = m->my + (m->mh / 2 - HEIGHT(c) / 2); /* center in y direction */
			if ((ev->value_mask & (CWX|CWY)) && !(ev->value_mask & (CWWidth|CWHeight)))
				configure(c);
			if (ISVISIBLE(c))
				XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
		} else
			configure(c);
	} else {
		wc.x = ev->x;
		wc.y = ev->y;
		wc.width = ev->width;
		wc.height = ev->height;
		wc.border_width = ev->border_width;
		wc.sibling = ev->above;
		wc.stack_mode = ev->detail;
		XConfigureWindow(dpy, ev->window, ev->value_mask, &wc);
	}
	XSync(dpy, False);
}

Client *
cprev(Client *c)
{
    Client *x, *y;

    if (!c)
        return NULL;

    for ( x = NULL, y = selmon->clients
        ; y && y != c
        ; x = y, y = y->next
        );
    return y ? x : y;
}

Client *
cprevtiled(Client *c)
{
    Client *x, *y;

    if (!c)
        return NULL;

    for ( x = NULL, y = nexttiled(selmon->clients)
        ; y && y != c
        ; x = y, y = nexttiled(y->next)
        );
    return y ? x : y;
}

Monitor *
createmon(void)
{
	Monitor *m;
	unsigned int i;

	m = ecalloc(1, sizeof(Monitor));
	m->tagset[0] = m->tagset[1] = defaulttag;
	m->mfact = mfact;
	m->nmaster = nmaster;
	m->showbar = showbar;
	m->topbar = topbar;
	m->lt[0] = &layouts[0];
	m->lt[1] = &layouts[1 % LENGTH(layouts)];
	strncpy(m->ltsymbol, layouts[0].symbol, sizeof m->ltsymbol);
	m->pertag = ecalloc(1, sizeof(Pertag));
	m->pertag->curtag = m->pertag->prevtag = 1;

	for (i = 0; i <= LENGTH(tags); i++) {
		m->pertag->nmasters[i] = m->nmaster;
		m->pertag->mfacts[i] = m->mfact;

		m->pertag->ltidxs[i][0] = m->lt[0];
		m->pertag->ltidxs[i][1] = m->lt[1];
		m->pertag->sellts[i] = m->sellt;

		m->pertag->showbars[i] = m->showbar;

        m->pertag->exstacks[i].master = 0;
        m->pertag->exstacks[i].stack  = 0;
	}

	return m;
}

void
cswap(Client *x, Client *y)
{
    Client *i, *px, *py;

    if (!x || !y || x == y)
        return;

    px = cprev(x);
    py = cprev(y);

    if (px)
        px->next = y;
    if (py)
        py->next = x;

    i = x->next;
    x->next = y->next;
    y->next = i;

    if (selmon->clients == x)
        selmon->clients = y;
    else if (selmon->clients == y)
        selmon->clients = x;
}

void
destroynotify(XEvent *e)
{
	Client *c;
	XDestroyWindowEvent *ev = &e->xdestroywindow;

	if ((c = wintoclient(ev->window)))
		unmanage(c, 1);

	else if ((c = swallowingclient(ev->window)))
		unmanage(c->swallowing, 1);
}

void
detach(Client *c)
{
	Client **tc;

	for (tc = &c->mon->clients; *tc && *tc != c; tc = &(*tc)->next);
	*tc = c->next;
}

void
detachstack(Client *c)
{
	Client **tc, *t;

	for (tc = &c->mon->stack; *tc && *tc != c; tc = &(*tc)->snext);
	*tc = c->snext;

	if (c == c->mon->sel) {
		for (t = c->mon->stack; t && !ISVISIBLE(t); t = t->snext);
		c->mon->sel = t;
	}
}

Monitor *
dirtomon(int dir)
{
	Monitor *m = NULL;

	if (dir > 0) {
		if (!(m = selmon->next))
			m = mons;
	} else if (selmon == mons)
		for (m = mons; m->next; m = m->next);
	else
		for (m = mons; m->next != selmon; m = m->next);
	return m;
}

int
drawstatusbar(Monitor *m, int bh, char* stext) {
	int ret, i, w, x, len;
	short isCode = 0;
	char *text;
	char *p;

	len = strlen(stext) + 1 ;
	if (!(text = (char*) malloc(sizeof(char)*len)))
		die("malloc");
	p = text;
	memcpy(text, stext, len);

	/* compute width of the status text */
	w = 0;
	i = -1;
	while (text[++i]) {
		if (text[i] == '^') {
			if (!isCode) {
				isCode = 1;
				text[i] = '\0';
				w += TEXTW(text) - lrpad;
				text[i] = '^';
				if (text[++i] == 'f')
					w += atoi(text + ++i);
			} else {
				isCode = 0;
				text = text + i + 1;
				i = -1;
			}
		}
	}
	if (!isCode)
		w += TEXTW(text) - lrpad;
	else
		isCode = 0;
	text = p;

	w += 2; /* 1px padding on both sides */
	ret = x = m->ww - w;

	drw_setscheme(drw, scheme[LENGTH(colors)]);
	drw->scheme[ColFg] = scheme[SchemeStatus][ColFg];
	drw->scheme[ColBg] = scheme[SchemeStatus][ColBg];
	drw_rect(drw, x - 2 * sp, 0, w, bh, 1, 1);
	x++;

	/* process status text */
	i = -1;
	while (text[++i]) {
		if (text[i] == '^' && !isCode) {
			isCode = 1;

			text[i] = '\0';
			w = TEXTW(text) - lrpad;
			drw_text(drw, x - 2 * sp, 0, w, bh, 0, text, 0);

			x += w;

			/* process code */
			while (text[++i] != '^') {
				if (text[i] == 'c') {
					char buf[8];
					memcpy(buf, (char*)text+i+1, 7);
					buf[7] = '\0';
					drw_clr_create(drw, &drw->scheme[ColFg], buf);
					i += 7;
				} else if (text[i] == 'b') {
					char buf[8];
					memcpy(buf, (char*)text+i+1, 7);
					buf[7] = '\0';
					drw_clr_create(drw, &drw->scheme[ColBg], buf);
					i += 7;
				} else if (text[i] == 'd') {
					drw->scheme[ColFg] = scheme[SchemeStatus][ColFg];
					drw->scheme[ColBg] = scheme[SchemeStatus][ColBg];
				} else if (text[i] == 'r') {
					int rx = atoi(text + ++i);
					while (text[++i] != ',');
					int ry = atoi(text + ++i);
					while (text[++i] != ',');
					int rw = atoi(text + ++i);
					while (text[++i] != ',');
					int rh = atoi(text + ++i);

					drw_rect(drw, rx + x - 2 * sp, ry, rw, rh, 1, 0);
				} else if (text[i] == 'f') {
					x += atoi(text + ++i);
				}
			}

			text = text + i + 1;
			i=-1;
			isCode = 0;
		}
	}

	if (!isCode) {
		w = TEXTW(text) - lrpad;
		drw_text(drw, x - 2 * sp, 0, w, bh, 0, text, 0);
	}

	drw_setscheme(drw, scheme[SchemeStatus]);
	free(p);

	return ret;
}

void
drawbar(Monitor *m)
{
	int x, w, tw = 0;
	int boxs = drw->fonts->h / 9;
	int boxw = drw->fonts->h / 6 + 2;
	unsigned int i, occ = 0, urg = 0;
	Client *c;

	if (!m->showbar)
		return;

	/* draw status first so it can be overdrawn by tags later */
	if (m == selmon || 1) { /* status is only drawn on selected monitor */
		tw = m->ww - drawstatusbar(m, bh, stext);
	}

	for (c = m->clients; c; c = c->next) {
		occ |= c->tags;
		if (c->isurgent)
			urg |= c->tags;
	}
	x = 0;
	for (i = 0; i < LENGTH(tags); i++) {
		w = TEXTW(tags[i]);
		drw_setscheme(drw, scheme[m->tagset[m->seltags] & 1 << i ? SchemeTagsSel : SchemeTagsNorm]);
		drw_text(drw, x, 0, w, bh, lrpad / 2, tags[i], urg & 1 << i);
		if (occ & 1 << i)
			drw_rect(drw, x + boxs, boxs, boxw, boxw,
				m == selmon && selmon->sel && selmon->sel->tags & 1 << i,
				urg & 1 << i);
		x += w;
	}
	w = TEXTW(m->ltsymbol);
	drw_setscheme(drw, scheme[SchemeTagsNorm]);
	x = drw_text(drw, x, 0, w, bh, lrpad / 2, m->ltsymbol, 0);

	if ((w = m->ww - tw - x) > bh) {
		drw_setscheme(drw, scheme[m == selmon ? SchemeInfoSel : SchemeInfoNorm]);
		if (m->sel) {
			drw_text(drw, x, 0, w - 2 * sp, bh, lrpad / 2, m->sel->name, 0);
			if (m->sel->isfloating)
				drw_rect(drw, x + boxs, boxs, boxw, boxw, m->sel->isfixed, 0);
		} else {
			drw_rect(drw, x, 0, w - 2 * sp, bh, 1, 1);
		}
	}
	drw_map(drw, m->barwin, 0, 0, m->ww, bh);
}

void
drawbars(void)
{
	Monitor *m;

	for (m = mons; m; m = m->next)
		drawbar(m);
}

void
enternotify(XEvent *e)
{
	Client *c;
	Monitor *m;
	XCrossingEvent *ev = &e->xcrossing;

	if ((ev->mode != NotifyNormal || ev->detail == NotifyInferior) && ev->window != root)
		return;
	c = wintoclient(ev->window);
	m = c ? c->mon : wintomon(ev->window);
	if (m != selmon) {
		unfocus(selmon->sel, 1);
		selmon = m;
	} else if (!c || c == selmon->sel)
		return;
	focus(c);
}

void
expose(XEvent *e)
{
	Monitor *m;
	XExposeEvent *ev = &e->xexpose;

	if (ev->count == 0 && (m = wintomon(ev->window)))
		drawbar(m);
}

void
exstack(const Arg *arg)
{
    Client *c;
    int n, ct = selmon->pertag->curtag;

    if (selmon->lt[selmon->sellt]->arrange != &tile
            || !selmon->sel || selmon->sel->isfloating)
        return;

    for (n = 0, c = nexttiled(selmon->clients); c && selmon->sel != c; c = nexttiled(c->next), ++n);

    if (n < selmon->nmaster)
        selmon->pertag->exstacks[ct].master = !selmon->pertag->exstacks[ct].master;
    else
        selmon->pertag->exstacks[ct].stack = !selmon->pertag->exstacks[ct].stack;

    arrange(selmon);
}

void
focus(Client *c)
{
	if (!c || !ISVISIBLE(c))
		for (c = selmon->stack; c && !ISVISIBLE(c); c = c->snext);
    xrayfocus(c);
	if (selmon->sel && selmon->sel != c)
		unfocus(selmon->sel, 0);
	if (c) {
		if (c->mon != selmon)
			selmon = c->mon;
		if (c->isurgent)
			seturgent(c, 0);
		detachstack(c);
		attachstack(c);
		grabbuttons(c, 1);
		/* set new focused border first to avoid flickering */
		if (SP_ISSPAD(c))
			XSetWindowBorder(dpy, c->win, scheme[SchemeSpad][ColFg].pixel);
		else
			XSetWindowBorder(dpy, c->win, scheme[SchemeSel][ColBorder].pixel);
		/* lastfocused may be us if another window was unmanaged */
		if (lastfocused && lastfocused != c) {
			if (SP_ISSPAD(lastfocused)) {
				if (sp_repaint)
					XSetWindowBorder(dpy, lastfocused->win, scheme[SchemeSpad][ColBg].pixel);
			} else
				XSetWindowBorder(dpy, lastfocused->win, scheme[SchemeNorm][ColBorder].pixel);
		}
		setfocus(c);
	} else {
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
		XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
	}
	selmon->sel = c;
	drawbars();
}

/* there are some broken focus acquiring clients needing extra handling */
void
focusin(XEvent *e)
{
	XFocusChangeEvent *ev = &e->xfocus;

	if (selmon->sel && ev->window != selmon->sel->win)
		setfocus(selmon->sel);
}

void
focusmon(const Arg *arg)
{
	Monitor *m;

	if (!mons->next)
		return;
	if ((m = dirtomon(arg->i)) == selmon)
		return;
	unfocus(selmon->sel, 0);
	XWarpPointer(dpy, None, m->barwin, 0, 0, 0, 0, m->mw / 2, m->mh / 2);
	selmon = m;
	focus(NULL);
}

void
focusrestack(Client *c)
{
    if (c) {
        focus(c);
        restack(selmon);
    }
}

void
focusstack(const Arg *arg)
{
	Client *c = NULL, *i;

	if (!selmon->sel || (selmon->sel->isfullscreen && lockfullscreen))
		return;
	if (arg->i > 0) {
		for (c = selmon->sel->next; c && !ISVISIBLE(c); c = c->next);
		if (!c)
			for (c = selmon->clients; c && !ISVISIBLE(c); c = c->next);
	} else {
		for (i = selmon->clients; i != selmon->sel; i = i->next)
			if (ISVISIBLE(i))
				c = i;
		if (!c)
			for (; i; i = i->next)
				if (ISVISIBLE(i))
					c = i;
	}
	if (c) {
		focus(c);
		restack(selmon);
	}
}

Atom
getatomprop(Client *c, Atom prop)
{
	int di;
	unsigned long nitems, dl;
	unsigned char *p = NULL;
	Atom da, atom = None;

	if (XGetWindowProperty(dpy, c->win, prop, 0L, sizeof atom, False, XA_ATOM,
		&da, &di, &nitems, &dl, &p) == Success && p) {
		if (nitems > 0)
			atom = *(Atom *)p;
		XFree(p);
	}
	return atom;
}

int
getrootptr(int *x, int *y)
{
	int di;
	unsigned int dui;
	Window dummy;

	return XQueryPointer(dpy, root, &dummy, &dummy, x, y, &di, &di, &dui);
}

long
getstate(Window w)
{
	int format;
	long result = -1;
	unsigned char *p = NULL;
	unsigned long n, extra;
	Atom real;

	if (XGetWindowProperty(dpy, w, wmatom[WMState], 0L, 2L, False, wmatom[WMState],
		&real, &format, &n, &extra, (unsigned char **)&p) != Success)
		return -1;
	if (n != 0)
		result = *p;
	XFree(p);
	return result;
}

int
gettextprop(Window w, Atom atom, char *text, unsigned int size)
{
	char **list = NULL;
	int n;
	XTextProperty name;

	if (!text || size == 0)
		return 0;
	text[0] = '\0';
	if (!XGetTextProperty(dpy, w, &name, atom) || !name.nitems)
		return 0;
	if (name.encoding == XA_STRING) {
		strncpy(text, (char *)name.value, size - 1);
	} else if (XmbTextPropertyToTextList(dpy, &name, &list, &n) >= Success && n > 0 && *list) {
		strncpy(text, *list, size - 1);
		XFreeStringList(list);
	}
	text[size - 1] = '\0';
	XFree(name.value);
	return 1;
}

void
grabbuttons(Client *c, int focused)
{
	updatenumlockmask();
	{
		unsigned int i, j;
		unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
		XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
		if (!focused)
			XGrabButton(dpy, AnyButton, AnyModifier, c->win, False,
				BUTTONMASK, GrabModeSync, GrabModeSync, None, None);
		for (i = 0; i < LENGTH(buttons); i++)
			if (buttons[i].click == ClkClientWin)
				for (j = 0; j < LENGTH(modifiers); j++)
					XGrabButton(dpy, buttons[i].button,
						buttons[i].mask | modifiers[j],
						c->win, False, BUTTONMASK,
						GrabModeAsync, GrabModeSync, None, None);
	}
}

void
grabkeys(void)
{
	updatenumlockmask();
	{
		unsigned int i, j, k;
		unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
		int start, end, skip;
		KeySym *syms;

		XUngrabKey(dpy, AnyKey, AnyModifier, root);
		XDisplayKeycodes(dpy, &start, &end);
		syms = XGetKeyboardMapping(dpy, start, end - start + 1, &skip);
		if (!syms)
			return;
		for (k = start; k <= end; k++)
			for (i = 0; i < LENGTH(keys); i++)
				/* skip modifier codes, we do that ourselves */
				if (keys[i].keysym == syms[(k - start) * skip])
					for (j = 0; j < LENGTH(modifiers); j++)
						XGrabKey(dpy, k,
							 keys[i].mod | modifiers[j],
							 root, True,
							 GrabModeAsync, GrabModeAsync);
		XFree(syms);
	}
}

void
incnmaster(const Arg *arg)
{
	selmon->nmaster = selmon->pertag->nmasters[selmon->pertag->curtag] = MAX(selmon->nmaster + arg->i, 0);
	arrange(selmon);
}

#ifdef XINERAMA
static int
isuniquegeom(XineramaScreenInfo *unique, size_t n, XineramaScreenInfo *info)
{
	while (n--)
		if (unique[n].x_org == info->x_org && unique[n].y_org == info->y_org
		&& unique[n].width == info->width && unique[n].height == info->height)
			return 0;
	return 1;
}
#endif /* XINERAMA */

void
keypress(XEvent *e)
{
	unsigned int i;
	KeySym keysym;
	XKeyEvent *ev;

	ev = &e->xkey;
	keysym = XKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0);
	for (i = 0; i < LENGTH(keys); i++)
		if (keysym == keys[i].keysym
		&& CLEANMASK(keys[i].mod) == CLEANMASK(ev->state)
		&& keys[i].func)
			keys[i].func(&(keys[i].arg));
}

void
killclient(const Arg *arg)
{
	if (!selmon->sel)
		return;
	if (!sendevent(selmon->sel, wmatom[WMDelete])) {
		XGrabServer(dpy);
		XSetErrorHandler(xerrordummy);
		XSetCloseDownMode(dpy, DestroyAll);
		XKillClient(dpy, selmon->sel->win);
		XSync(dpy, False);
		XSetErrorHandler(xerror);
		XUngrabServer(dpy);
	}
}

void
manage(Window w, XWindowAttributes *wa)
{
	Client *c, *t = NULL, *term = NULL;
	Window trans = None;
	XWindowChanges wc;
    SpState *sps = NULL;

	c = ecalloc(1, sizeof(Client));
	c->win = w;
	c->pid = winpid(w);
    xraycfg(c, XrayGrpNone, -1);
	/* geometry */
	c->x = c->oldx = wa->x;
	c->y = c->oldy = wa->y;
	c->w = c->oldw = wa->width;
	c->h = c->oldh = wa->height;
	c->oldbw = wa->border_width;

	updatetitle(c);
	if (XGetTransientForHint(dpy, w, &trans) && (t = wintoclient(trans))) {
		c->mon = t->mon;
		c->tags = t->tags;
	} else {
		c->mon = selmon;
        if (sp_toggled && (sps = sp_match(c))) {
            sps->client = c;
            c->bw = sp_borderpx;
            c->tags = sp_tag_vis;
            sp_configure(c, sps->spad->rules);
            sp_toggled = 0;
        } else {
            if ((sps = sp_active(selmon)) && (sps->spad->close & SP_C_NEW)) {
                sp_hide(selmon);
                sp_repaint = 0;
            }
            sps = NULL;
            applyrules(c);
    		term = termforwin(c);
        }
	}

	if (c->x + WIDTH(c) > c->mon->wx + c->mon->ww)
		c->x = c->mon->wx + c->mon->ww - WIDTH(c);
	if (c->y + HEIGHT(c) > c->mon->wy + c->mon->wh)
		c->y = c->mon->wy + c->mon->wh - HEIGHT(c);
	c->x = MAX(c->x, c->mon->wx);
	c->y = MAX(c->y, c->mon->wy);
	if (!sps)
		c->bw = borderpx;

	wc.border_width = c->bw;
	XConfigureWindow(dpy, w, CWBorderWidth, &wc);
	if (sps)
		XSetWindowBorder(dpy, w, scheme[SchemeSpad][ColBg].pixel);
	else
		XSetWindowBorder(dpy, w, scheme[SchemeNorm][ColBorder].pixel);
	configure(c); /* propagates border_width, if size doesn't change */
	updatewindowtype(c);
	updatesizehints(c);
	updatewmhints(c);
	XSelectInput(dpy, w, EnterWindowMask|FocusChangeMask|PropertyChangeMask|StructureNotifyMask);
	grabbuttons(c, 0);
	if (!c->isfloating)
		c->isfloating = c->oldstate = trans != None || c->isfixed;
	if (c->isfloating)
		XRaiseWindow(dpy, c->win);
	attachbottom(c);
	attachstack(c);
	XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32, PropModeAppend,
		(unsigned char *) &(c->win), 1);
	XMoveResizeWindow(dpy, c->win, c->x + 2 * sw, c->y, c->w, c->h); /* some windows require this */
	setclientstate(c, NormalState);
	if (c->mon == selmon)
		unfocus(selmon->sel, 0);
	c->mon->sel = c;
	arrange(c->mon);
	XMapWindow(dpy, c->win);
	if (term)
		swallow(term, c);
	focus(NULL);
    sp_repaint = 1;
}

void
mappingnotify(XEvent *e)
{
	XMappingEvent *ev = &e->xmapping;

	XRefreshKeyboardMapping(ev);
	if (ev->request == MappingKeyboard)
		grabkeys();
}

void
maprequest(XEvent *e)
{
	static XWindowAttributes wa;
	XMapRequestEvent *ev = &e->xmaprequest;

	if (!XGetWindowAttributes(dpy, ev->window, &wa) || wa.override_redirect)
		return;
	if (!wintoclient(ev->window))
		manage(ev->window, &wa);
}

void
monocle(Monitor *m)
{
	unsigned int n = 0;
	Client *c;
    int ac;

	for (c = m->clients; c; c = c->next)
		if (ISVISIBLE(c))
			n++;
	if (n > 0) /* override layout symbol */
		snprintf(m->ltsymbol, sizeof m->ltsymbol, "[%d]", n);
	for (ac = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next)) {
        xraycfg(c, XrayGrpMonocle, m->sel == c ? (ac = 1, XrayAc) : XrayIn);
		resize(c, m->wx + mgap, m->wy + mgap, m->ww - 2 * (c->bw + mgap), m->wh - 2 * (c->bw + mgap), 0);
        xrayapply(c);
    }

    if (!ac)
        xrayfallback(m, XrayGrpMonocle);
}

void
motionnotify(XEvent *e)
{
	static Monitor *mon = NULL;
	Monitor *m;
	XMotionEvent *ev = &e->xmotion;

	if (ev->window != root)
		return;
	if ((m = recttomon(ev->x_root, ev->y_root, 1, 1)) != mon && mon) {
		unfocus(selmon->sel, 1);
		selmon = m;
		focus(NULL);
	}
	mon = m;
}

void
movemouse(const Arg *arg)
{
	int x, y, ocx, ocy, nx, ny;
	Client *c;
	Monitor *m;
	XEvent ev;
	Time lasttime = 0;

	if (!(c = selmon->sel) || SP_ISSPAD(selmon->sel))
		return;
	if (c->isfullscreen) /* no support moving fullscreen windows by mouse */
		return;
	restack(selmon);
	ocx = c->x;
	ocy = c->y;
	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, cursor[CurMove]->cursor, CurrentTime) != GrabSuccess)
		return;
	if (!getrootptr(&x, &y))
		return;
	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch(ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / refreshrate))
				continue;
			lasttime = ev.xmotion.time;

			nx = ocx + (ev.xmotion.x - x);
			ny = ocy + (ev.xmotion.y - y);
			if (abs(selmon->wx - nx) < snap + sgap)
				nx = selmon->wx + sgap;
			else if (abs((selmon->wx + selmon->ww) - (nx + WIDTH(c))) < snap + sgap)
				nx = selmon->wx + selmon->ww - WIDTH(c) - sgap;
			if (abs(selmon->wy - ny) < snap + sgap)
				ny = selmon->wy + sgap;
			else if (abs((selmon->wy + selmon->wh) - (ny + HEIGHT(c))) < snap + sgap)
				ny = selmon->wy + selmon->wh - HEIGHT(c) - sgap;
			if (!selmon->lt[selmon->sellt]->arrange || c->isfloating)
				resize(c, nx, ny, c->w, c->h, 1);
			else if (selmon->lt[selmon->sellt]->arrange || !c->isfloating) {
				if ((m = recttomon(ev.xmotion.x_root, ev.xmotion.y_root, 1, 1)) != selmon) {
					sendmon(c, m);
					selmon = m;
					focus(NULL);
				}

				Client *cc = c->mon->clients;
				while (1) {
					if (cc == 0) break;
					if(
					 cc != c && !cc->isfloating && ISVISIBLE(cc) &&
					 ev.xmotion.x_root > cc->x &&
					 ev.xmotion.x_root < cc->x + cc->w &&
					 ev.xmotion.y_root > cc->y &&
					 ev.xmotion.y_root < cc->y + cc->h ) {
						break;
					}

					cc = cc->next;
				}

				if (cc) {
					Client *cl1, *cl2, ocl1;
					
					if (!selmon->lt[selmon->sellt]->arrange) return;

					cl1 = c;
					cl2 = cc;
					ocl1 = *cl1;
					strcpy(cl1->name, cl2->name);
					cl1->win = cl2->win;
					cl1->x = cl2->x;
					cl1->y = cl2->y;
					cl1->w = cl2->w;
					cl1->h = cl2->h;
					
					cl2->win = ocl1.win;
					strcpy(cl2->name, ocl1.name);
					cl2->x = ocl1.x;
					cl2->y = ocl1.y;
					cl2->w = ocl1.w;
					cl2->h = ocl1.h;
					
					selmon->sel = cl2;

					c = cc;
					focus(c);
					
					arrange(cl1->mon);
				}
			}
			break;
		}
	} while (ev.type != ButtonRelease);
	XUngrabPointer(dpy, CurrentTime);
	if ((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
		sendmon(c, m);
		selmon = m;
		focus(NULL);
	}
}

Client *
nexttiled(Client *c)
{
	for (; c && (c->isfloating || !ISVISIBLE(c)); c = c->next);
	return c;
}

void
pop(Client *c)
{
	detach(c);
	attach(c);
	focus(c);
	arrange(c->mon);
}

void
propertynotify(XEvent *e)
{
	Client *c;
	Window trans;
	XPropertyEvent *ev = &e->xproperty;

	if ((ev->window == root) && (ev->atom == XA_WM_NAME))
		updatestatus();
	else if (ev->state == PropertyDelete)
		return; /* ignore */
	else if ((c = wintoclient(ev->window))) {
		switch(ev->atom) {
		default: break;
		case XA_WM_TRANSIENT_FOR:
			if (!c->isfloating && (XGetTransientForHint(dpy, c->win, &trans)) &&
				(c->isfloating = (wintoclient(trans)) != NULL))
				arrange(c->mon);
			break;
		case XA_WM_NORMAL_HINTS:
			c->hintsvalid = 0;
			break;
		case XA_WM_HINTS:
			updatewmhints(c);
			drawbars();
			break;
		}
		if (ev->atom == XA_WM_NAME || ev->atom == netatom[NetWMName]) {
			updatetitle(c);
			if (c == c->mon->sel)
				drawbar(c->mon);
		}
		if (ev->atom == netatom[NetWMWindowType])
			updatewindowtype(c);
	}
}

void
quit(const Arg *arg)
{
	if(arg->i) restart = 1;
	running = 0;
}

Monitor *
recttomon(int x, int y, int w, int h)
{
	Monitor *m, *r = selmon;
	int a, area = 0;

	for (m = mons; m; m = m->next)
		if ((a = INTERSECT(x, y, w, h, m)) > area) {
			area = a;
			r = m;
		}
	return r;
}

void
resize(Client *c, int x, int y, int w, int h, int interact)
{
	if (applysizehints(c, &x, &y, &w, &h, interact))
		resizeclient(c, x, y, w, h);
}

void
resizeclient(Client *c, int x, int y, int w, int h)
{
	XWindowChanges wc;

	c->oldx = c->x; c->x = wc.x = x;
	c->oldy = c->y; c->y = wc.y = y;
	c->oldw = c->w; c->w = wc.width = w;
	c->oldh = c->h; c->h = wc.height = h;
	wc.border_width = c->bw;
	XConfigureWindow(dpy, c->win, CWX|CWY|CWWidth|CWHeight|CWBorderWidth, &wc);
	configure(c);
	XSync(dpy, False);
}

void
resizemouse(const Arg *arg)
{
	int ocx, ocy, nw, nh;
	int ocx2, ocy2, nx, ny;
	Client *c;
	Monitor *m;
	XEvent ev;
	int horizcorner, vertcorner;
	int di;
	unsigned int dui;
	Window dummy;
	Time lasttime = 0;

	if (!(c = selmon->sel) || SP_ISSPAD(selmon->sel))
		return;
	if (c->isfullscreen) /* no support resizing fullscreen windows by mouse */
		return;
	restack(selmon);
	ocx = c->x;
	ocy = c->y;
	ocx2 = c->x + c->w;
	ocy2 = c->y + c->h;
	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, cursor[CurResize]->cursor, CurrentTime) != GrabSuccess)
		return;
	if (!XQueryPointer (dpy, c->win, &dummy, &dummy, &di, &di, &nx, &ny, &dui))
		return;
	horizcorner = nx < c->w / 2;
	vertcorner  = ny < c->h / 2;
	XWarpPointer (dpy, None, c->win, 0, 0, 0, 0,
			horizcorner ? (-c->bw) : (c->w + c->bw -1),
			vertcorner  ? (-c->bw) : (c->h + c->bw -1));
	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch(ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / refreshrate))
				continue;
			lasttime = ev.xmotion.time;

			nx = horizcorner && ocx2 - ev.xmotion.x >= c->minw ? ev.xmotion.x : c->x;
			ny = vertcorner && ocy2 - ev.xmotion.y >= c->minh ? ev.xmotion.y : c->y;
			nw = MAX(horizcorner ? (ocx2 - nx) : (ev.xmotion.x - ocx - 2 * c->bw + 1), 1);
			nh = MAX(vertcorner ? (ocy2 - ny) : (ev.xmotion.y - ocy - 2 * c->bw + 1), 1);

			if (horizcorner && ev.xmotion.x > ocx2)
				nx = ocx2 - (nw = c->minw);
			if (vertcorner && ev.xmotion.y > ocy2)
				ny = ocy2 - (nh = c->minh);

			if (c->mon->wx + nw >= selmon->wx && c->mon->wx + nw <= selmon->wx + selmon->ww
			&& c->mon->wy + nh >= selmon->wy && c->mon->wy + nh <= selmon->wy + selmon->wh)
			{
				if (!c->isfloating && selmon->lt[selmon->sellt]->arrange
				&& (abs(nw - c->w) > snap || abs(nh - c->h) > snap))
					togglefloating(NULL);
			}
			if (!selmon->lt[selmon->sellt]->arrange || c->isfloating)
				resize(c, nx, ny, nw, nh, 1);
			break;
		}
	} while (ev.type != ButtonRelease);
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0,
		      horizcorner ? (-c->bw) : (c->w + c->bw - 1),
		      vertcorner ? (-c->bw) : (c->h + c->bw - 1));
	XUngrabPointer(dpy, CurrentTime);
	while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));
	if ((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
		sendmon(c, m);
		selmon = m;
		focus(NULL);
	}
}

void
restack(Monitor *m)
{
	Client *c;
	XEvent ev;
	XWindowChanges wc;

	drawbar(m);
	if (!m->sel)
		return;
	if (m->sel->isfloating || !m->lt[m->sellt]->arrange)
		XRaiseWindow(dpy, m->sel->win);
	if (m->lt[m->sellt]->arrange) {
		wc.stack_mode = Below;
		wc.sibling = m->barwin;
		for (c = m->stack; c; c = c->snext)
			if (!c->isfloating && ISVISIBLE(c)) {
				XConfigureWindow(dpy, c->win, CWSibling|CWStackMode, &wc);
				wc.sibling = c->win;
			}
	}
	XSync(dpy, False);
	while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void
run(void)
{
	XEvent ev;
	/* main event loop */
	XSync(dpy, False);
	while (running && !XNextEvent(dpy, &ev))
		if (handler[ev.type])
			handler[ev.type](&ev); /* call handler */
}

void
scan(void)
{
	unsigned int i, num;
	Window d1, d2, *wins = NULL;
	XWindowAttributes wa;

	if (XQueryTree(dpy, root, &d1, &d2, &wins, &num)) {
		for (i = 0; i < num; i++) {
			if (!XGetWindowAttributes(dpy, wins[i], &wa)
			|| wa.override_redirect || XGetTransientForHint(dpy, wins[i], &d1))
				continue;
			if (wa.map_state == IsViewable || getstate(wins[i]) == IconicState)
				manage(wins[i], &wa);
		}
		for (i = 0; i < num; i++) { /* now the transients */
			if (!XGetWindowAttributes(dpy, wins[i], &wa))
				continue;
			if (XGetTransientForHint(dpy, wins[i], &d1)
			&& (wa.map_state == IsViewable || getstate(wins[i]) == IconicState))
				manage(wins[i], &wa);
		}
		if (wins)
			XFree(wins);
	}
}

void
sendmon(Client *c, Monitor *m)
{
	if (c->mon == m || SP_ISSPAD(c))
		return;
	unfocus(c, 1);
	detach(c);
	detachstack(c);
	c->mon = m;
	c->tags = m->tagset[m->seltags]; /* assign tags of target monitor */
	attachbottom(c);
	attachstack(c);
	focus(NULL);
	arrange(NULL);
}

void
setclientstate(Client *c, long state)
{
	long data[] = { state, None };

	XChangeProperty(dpy, c->win, wmatom[WMState], wmatom[WMState], 32,
		PropModeReplace, (unsigned char *)data, 2);
}

int
sendevent(Client *c, Atom proto)
{
	int n;
	Atom *protocols;
	int exists = 0;
	XEvent ev;

	if (XGetWMProtocols(dpy, c->win, &protocols, &n)) {
		while (!exists && n--)
			exists = protocols[n] == proto;
		XFree(protocols);
	}
	if (exists) {
		ev.type = ClientMessage;
		ev.xclient.window = c->win;
		ev.xclient.message_type = wmatom[WMProtocols];
		ev.xclient.format = 32;
		ev.xclient.data.l[0] = proto;
		ev.xclient.data.l[1] = CurrentTime;
		XSendEvent(dpy, c->win, False, NoEventMask, &ev);
	}
	return exists;
}

void
setfocus(Client *c)
{
	if (!c->neverfocus) {
		XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
		XChangeProperty(dpy, root, netatom[NetActiveWindow],
			XA_WINDOW, 32, PropModeReplace,
			(unsigned char *) &(c->win), 1);
	}
	sendevent(c, wmatom[WMTakeFocus]);
}

void
setfullscreen(Client *c, int fullscreen)
{
	if (fullscreen && !c->isfullscreen) {
        if (SP_ISSPAD(c) && !sp_permit_fullscreen)
            return;
		XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
			PropModeReplace, (unsigned char*)&netatom[NetWMFullscreen], 1);
        tmpnmdec(c, TmpNmHookSetFullscreen);
		c->isfullscreen = 1;
		c->oldstate = c->isfloating;
		c->oldbw = c->bw;
		c->bw = 0;
		c->isfloating = 1;
        xraycfg(c, XrayGrpNone, -1);
		resizeclient(c, c->mon->mx, c->mon->my, c->mon->mw, c->mon->mh);
		XRaiseWindow(dpy, c->win);
	} else if (!fullscreen && c->isfullscreen){
		XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
			PropModeReplace, (unsigned char*)0, 0);
		c->isfullscreen = 0;
		c->isfloating = c->oldstate;
		c->bw = c->oldbw;
		c->x = c->oldx;
		c->y = c->oldy;
		c->w = c->oldw;
		c->h = c->oldh;
		resizeclient(c, c->x, c->y, c->w, c->h);
		arrange(c->mon);
	}
}

void
setlayout(const Arg *arg)
{
    SpState *sps;
	if (!arg || !arg->v || arg->v != selmon->lt[selmon->sellt])
		selmon->sellt = selmon->pertag->sellts[selmon->pertag->curtag] ^= 1;
	if (arg && arg->v)
		selmon->lt[selmon->sellt] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt] = (Layout *)arg->v;
	strncpy(selmon->ltsymbol, selmon->lt[selmon->sellt]->symbol, sizeof selmon->ltsymbol);
    sps = sp_active(selmon);
    if (sps && (sps->spad->close & SP_C_LAYOUT)) {
        sp_hide(selmon);
        sp_repaint = 0;
        focus(NULL);
        sp_repaint = 1;
    }
	if (selmon->sel)
		arrange(selmon);
	else
		drawbar(selmon);
}

/* arg > 1.0 will set mfact absolutely */
void
setmfact(const Arg *arg)
{
	float f;

	if (!arg || !selmon->lt[selmon->sellt]->arrange)
		return;
	f = arg->f < 1.0 ? arg->f + selmon->mfact : arg->f - 1.0;
	if (f < 0.05 || f > 0.95)
		return;
	selmon->mfact = selmon->pertag->mfacts[selmon->pertag->curtag] = f;
	arrange(selmon);
}

void
setup(void)
{
	int i;
	XSetWindowAttributes wa;
	Atom utf8string;
	struct sigaction sa;

	/* do not transform children into zombies when they terminate */
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_NOCLDSTOP | SA_NOCLDWAIT | SA_RESTART;
	sa.sa_handler = SIG_IGN;
	sigaction(SIGCHLD, &sa, NULL);

	/* clean up any zombies (inherited from .xinitrc etc) immediately */
	while (waitpid(-1, NULL, WNOHANG) > 0);

	signal(SIGHUP, sighup);
	signal(SIGTERM, sigterm);

	/* init screen */
	screen = DefaultScreen(dpy);
	sw = DisplayWidth(dpy, screen);
	sh = DisplayHeight(dpy, screen);
	root = RootWindow(dpy, screen);
	drw = drw_create(dpy, screen, root, sw, sh);
	if (!drw_fontset_create(drw, fonts, LENGTH(fonts)))
		die("no fonts could be loaded.");
	lrpad = drw->fonts->h;
	bh = drw->fonts->h + 2;
	sp = sidepad;
	vp = (topbar == 1) ? vertpad : - vertpad;
	updategeom();

	/* init atoms */
	utf8string = XInternAtom(dpy, "UTF8_STRING", False);
	wmatom[WMProtocols] = XInternAtom(dpy, "WM_PROTOCOLS", False);
	wmatom[WMDelete] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	wmatom[WMState] = XInternAtom(dpy, "WM_STATE", False);
	wmatom[WMTakeFocus] = XInternAtom(dpy, "WM_TAKE_FOCUS", False);
	netatom[NetActiveWindow] = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
	netatom[NetSupported] = XInternAtom(dpy, "_NET_SUPPORTED", False);
	netatom[NetWMName] = XInternAtom(dpy, "_NET_WM_NAME", False);
	netatom[NetWMState] = XInternAtom(dpy, "_NET_WM_STATE", False);
	netatom[NetWMCheck] = XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", False);
	netatom[NetWMFullscreen] = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
	netatom[NetWMWindowType] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
	netatom[NetWMWindowTypeDialog] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", False);
	netatom[NetClientList] = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
    netatom[NetWMPid] = XInternAtom(dpy, "_NET_WM_PID", False);
	/* init cursors */
	cursor[CurNormal] = drw_cur_create(drw, XC_left_ptr);
	cursor[CurResize] = drw_cur_create(drw, XC_sizing);
	cursor[CurMove] = drw_cur_create(drw, XC_fleur);
	/* init scratchpad state */
	for (i = 0; i < LENGTH(sp_defs); ++i)
		sp_states[i] = (SpState){
			.spad = sp_defs + i,
			.pid = -1,
			.client = NULL,
		};
	/* init appearance */
	scheme = ecalloc(LENGTH(colors) + 1, sizeof(Clr *));
	scheme[LENGTH(colors)] = drw_scm_create(drw, colors[0], 3);
	for (i = 0; i < LENGTH(colors); i++)
		scheme[i] = drw_scm_create(drw, colors[i], 3);
	/* init bars */
	updatebars();
	updatestatus();
	/* supporting window for NetWMCheck */
	wmcheckwin = XCreateSimpleWindow(dpy, root, 0, 0, 1, 1, 0, 0, 0);
	XChangeProperty(dpy, wmcheckwin, netatom[NetWMCheck], XA_WINDOW, 32,
		PropModeReplace, (unsigned char *) &wmcheckwin, 1);
	XChangeProperty(dpy, wmcheckwin, netatom[NetWMName], utf8string, 8,
		PropModeReplace, (unsigned char *) "dwm", 3);
	XChangeProperty(dpy, root, netatom[NetWMCheck], XA_WINDOW, 32,
		PropModeReplace, (unsigned char *) &wmcheckwin, 1);
	/* EWMH support per view */
	XChangeProperty(dpy, root, netatom[NetSupported], XA_ATOM, 32,
		PropModeReplace, (unsigned char *) netatom, NetLast);
	XDeleteProperty(dpy, root, netatom[NetClientList]);
	/* select events */
	wa.cursor = cursor[CurNormal]->cursor;
	wa.event_mask = SubstructureRedirectMask|SubstructureNotifyMask
		|ButtonPressMask|PointerMotionMask|EnterWindowMask
		|LeaveWindowMask|StructureNotifyMask|PropertyChangeMask;
	XChangeWindowAttributes(dpy, root, CWEventMask|CWCursor, &wa);
	XSelectInput(dpy, root, wa.event_mask);
	grabkeys();
	focus(NULL);
}

void
seturgent(Client *c, int urg)
{
	XWMHints *wmh;

	c->isurgent = urg;
	if (!(wmh = XGetWMHints(dpy, c->win)))
		return;
	wmh->flags = urg ? (wmh->flags | XUrgencyHint) : (wmh->flags & ~XUrgencyHint);
	XSetWMHints(dpy, c->win, wmh);
	XFree(wmh);
}

void
showhide(Client *c)
{
    SpState *sps;
	if (!c)
		return;
	if (ISVISIBLE(c)) {
		/* show clients top down */
        if (SP_ISSPAD(c) && (sps = sp_state(c))) {
            sp_configure(c, sps->spad->rules);
            if (sps->spad->rules.tag != SP_R_TILE_TAG)
                XResizeWindow(dpy, c->win, c->w, c->h);
        }
		XMoveWindow(dpy, c->win, c->x, c->y);
		if ((!c->mon->lt[c->mon->sellt]->arrange || c->isfloating) && !c->isfullscreen)
			resize(c, c->x, c->y, c->w, c->h, 0);
		showhide(c->snext);
	} else {
		/* hide clients bottom up */
		showhide(c->snext);
		XMoveWindow(dpy, c->win, WIDTH(c) * -2, c->y);
	}
}

void
sighup(int unused)
{
	Arg a = {.i = 1};
	quit(&a);
}

void
sigterm(int unused)
{
	Arg a = {.i = 0};
	quit(&a);
}

void
spawn(const Arg *arg)
{
	struct sigaction sa;

	if (fork() == 0) {
		if (dpy)
			close(ConnectionNumber(dpy));
		setsid();

		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
		sa.sa_handler = SIG_DFL;
		sigaction(SIGCHLD, &sa, NULL);

		execvp(((char **)arg->v)[0], (char **)arg->v);
		die("dwm: execvp '%s' failed:", ((char **)arg->v)[0]);
	}
}

SpState *
sp_active(Monitor *m)
{
    int i;
    Client *c;

    if (!m || (m->tagset[m->seltags] & sp_tag_vis) == 0)
        return NULL;

    for (i = 0; i < LENGTH(sp_states); ++i) {
        c = sp_states[i].client;
        if (c && (c->tags & sp_tag_vis) && c->mon == m)
            return sp_states + i;
    }

    return NULL;
}

void
sp_configure(Client *c, SpRules r)
{
    int nc, s, st, i;
    float mfact;
    Client *x;

    if (!c)
        return;

    switch (r.tag) {
        case SP_R_MFACT_TAG:
            for ( nc = 0, x = nexttiled(c->mon->clients)
                ; x
                ; x = nexttiled(x->next), ++nc );
            mfact = c->mon->mfact;
            for (i = 0; i < LENGTH(sp_constant_mfacts); ++i)
                if (c->mon->lt[c->mon->sellt]->arrange
                        == sp_constant_mfacts[i].arrange) {
                    mfact = sp_constant_mfacts[i].mfact;
                    break;
                }
            s    = mfact == 1 || nc == 0
                              || c->mon->nmaster == 0
                              || nc <= c->mon->nmaster;
            st   = r.val.mfact.d > 0;
            r.w  = s ? 1 : st ? 1 - mfact : mfact;
            r.h  = 1;
            c->x = (c->mon->ww * mfact + r.val.mfact.i) * (st*!s)
                   + r.val.mfact.o * (!st||s);
            c->y = r.val.mfact.o;
            c->w = 0 - r.val.mfact.o * (s+1) - r.val.mfact.i * !s;
            c->h = 0 - r.val.mfact.o * 2;
            break;
        case SP_R_GAPS_TAG:
            c->x = r.val.gaps.l;
            c->y = r.val.gaps.t;
            c->w = 0 - r.val.gaps.l - r.val.gaps.r;
            c->h = 0 - r.val.gaps.t - r.val.gaps.d;
            break;
        case SP_R_XY_TAG:
            c->x = c->mon->ww * r.val.xy.x;
            c->y = c->mon->wh * r.val.xy.y;
            c->w = c->h = 0;
            break;
        case SP_R_TILE_TAG:
            c->isfloating = 0;
        default:
            return;
    }

    c->isfloating = 1;
    c->x += c->mon->wx;
    c->y += c->mon->wy;
    c->w += (int)(c->mon->ww * r.w) - c->bw * 2;
    c->h += (int)(c->mon->wh * r.h) - c->bw * 2;
}

void
sp_focus(const Arg *arg)
{
    Client *c;

    if (!selmon->sel)
        return;

    if (SP_ISSPAD(selmon->sel)) {
        for (c = selmon->stack; c; c = c->snext)
            if (ISVISIBLE(c) && c != selmon->sel) {
                focus(c);
                break;
            }
    } else
        for (c = selmon->clients; c; c = c->next)
            if (ISVISIBLE(c) && SP_ISSPAD(c)) {
                focus(c);
                break;
            }
}

void
sp_hide(Monitor *m)
{
    SpState *sps;
    int i;

    for (i = 0; i < LENGTH(sp_states); ++i) {
        sps = sp_states + i;
        if (!sps->client || (m && sps->client->mon != m))
            continue;
        sps->client->tags = sp_tag_hid;
    }

    if (m)
        m->tagset[m->seltags] &= ~sp_tag_vis;
    else
        for (m = mons; m; m = m->next)
            m->tagset[m->seltags] &= ~sp_tag_vis;
}

SpState *
sp_match(Client *c)
{
    SpState *sps;
    return (sps = sp_match_pid(c))        ? sps
         : (sps = sp_match_class_hint(c)) ? sps
         : (sps = sp_match_title(c))      ? sps
         : NULL;
}

SpState *
sp_match_class_hint(Client *c)
{
    int i;
    SpState *sps;
    const SpMatch *m;
    const char *class, *instance, *which;
    XClassHint ch = { NULL, NULL };

    if (!XGetClassHint(dpy, c->win, &ch))
        return NULL;
    class    = ch.res_class ? ch.res_class : broken;
    instance = ch.res_class ? ch.res_name  : broken;

    for (i = 0; i < LENGTH(sp_states); ++i, sps = NULL) {
        sps   = &sp_states[i];
        m     = &sps->spad->match;
        which = m->tag == SP_M_CLASS_TAG    ? class
              : m->tag == SP_M_INSTANCE_TAG ? instance
              : NULL;
        if (sps->client == c)
            break;
        if (sps->client || !m->val || !*m->val || !which)
            continue;
        if (strstr(which, m->val))
            break;
    }

    if (ch.res_name)
        XFree(ch.res_name);
    if (ch.res_class)
        XFree(ch.res_class);

    return sps;
}

SpState *
sp_match_pid(Client *c)
{
    int i;
    pid_t pid;
    XTextProperty text;

    if (!XGetTextProperty(dpy, c->win, &text, netatom[NetWMPid]))
        return NULL;

    pid = *(unsigned int *)text.value;

    for (i = 0; i < LENGTH(sp_states); ++i)
        if ( sp_states[i].spad->match.tag == SP_M_PID_TAG
          && sp_states[i].pid == pid
          && ( sp_states[i].client == NULL
            || sp_states[i].client == c ) )
            return sp_states + i;

    return NULL;
}

SpState *
sp_match_title(Client *c)
{
    int i;
    SpState *sps;
    const SpMatch *m;

    for (i = 0; i < LENGTH(sp_states); ++i) {
        sps = &sp_states[i];
        m   = &sps->spad->match;
        if (sps->client == c)
            return sps;
        if ( sps->client || m->tag != SP_M_TITLE_TAG
          || !m->val || !*m->val )
            continue;
        if (strstr(c->name, m->val))
            return sps;
    }

    return NULL;
}

pid_t
sp_spawn(const Spad *spad)
{
    pid_t pid;
    struct sigaction sa;

    if (!spad || !spad->cmd)
        return -1;

    if ((pid = fork()) == 0) {
        if (dpy)
            close(ConnectionNumber(dpy));
        setsid();

        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sa.sa_handler = SIG_DFL;
        sigaction(SIGCHLD, &sa, NULL);

        execvp(((char **)spad->cmd)[0], (char **)spad->cmd);
        die("dwm: execvp '%s' failed:", ((char **)spad->cmd)[0]);
    }

    return pid;
}

SpState *
sp_state(Client *c)
{
    for (int i = 0; i < LENGTH(sp_states); ++i)
        if (sp_states[i].client == c)
            return sp_states + i;
    return NULL;
}

void
sp_toggle(const Arg *arg)
{
    unsigned int tagset;
    SpState *sps, *x;
    Monitor *m;
    int i;

    if (!arg || !arg->v || !*(char *)arg->v)
        return;

    for (sps = NULL, i = 0; i < LENGTH(sp_defs); ++i)
        if (strcmp(sp_defs[i].name, arg->v) == 0) {
            sps = sp_states + i;
            break;
        }
    if (!sps)
        return;

    for (i = 0; i < LENGTH(sp_states); ++i) {
        x = sp_states + i;
        if (x == sps || !x->client || !( sp_exclusive_across_mons
                                      || x->client->mon == selmon ) )
            continue;
        x->client->tags = sp_tag_hid;
    }

    tagset = selmon->tagset[selmon->seltags];
    if (sp_exclusive_across_mons)
        for (m = mons; m; m = m->next)
            m->tagset[m->seltags] &= ~sp_tag_vis;

    selmon->tagset[selmon->seltags] |= sp_tag_vis;
    if (sps->client == NULL) {
        sps->pid = sp_spawn(sps->spad);
        sp_toggled = 1;
        if (sp_exclusive_across_mons)
            for (m = mons; m; m = m->next)
                if (m != selmon)
                    arrange(m);
        return;
    }

    if (sps->client->mon != selmon) {
        detach(sps->client);
        detachstack(sps->client);
        sps->client->mon = selmon;
        sps->client->tags = sp_tag_hid;
        attach(sps->client);
        attachstack(sps->client);
    }

    sp_repaint = 0;
    if ((tagset & sp_tag_vis) && (sps->client->tags & sp_tag_vis)) {
        sps->client->tags = sp_tag_hid;
        selmon->tagset[selmon->seltags] &= ~sp_tag_vis;
        if (sps->client->isfullscreen) {
            setfullscreen(sps->client, 0);
        }
    } else
        sps->client->tags = sp_tag_vis;
    focus(sps->client);
    sp_repaint = 1;
    arrange(NULL);
    restack(selmon);
}

void
tag(const Arg *arg)
{
	if (selmon->sel && (arg->ui & TAGMASK) && !SP_ISSPAD(selmon->sel)) {
		selmon->sel->tags = arg->ui & TAGMASK;
		focus(NULL);
		arrange(selmon);
	}
}

void
tagmon(const Arg *arg)
{
	if (!selmon->sel || !mons->next)
		return;
	sendmon(selmon->sel, dirtomon(arg->i));
}

void
tile(Monitor *m)
{
    unsigned int i, n, h, mw, my, ty, si, ma, sa, ns;
    Client *c, *xr[2];
    int em = m->pertag->exstacks[m->pertag->curtag].master
      , es = m->pertag->exstacks[m->pertag->curtag].stack;

    for (n = si = 0, xr[0] = xr[1] = NULL, c = nexttiled(m->clients); c; c = nexttiled(c->next), ++n) {
        if (m->sel == c)
            si = n;

        if (c->xraystat == XrayAc) switch (c->xraygrp) {
            case XrayGrpTileMaster : xr[0] = c; break;
            case XrayGrpTileStack  : xr[1] = c; break;
        }
    }

    if (n == 0)
        return;

    if (m->sel && !m->sel->isfloating)
        xr[si >= m->nmaster] = m->sel;

    if (n > m->nmaster) {
        mw = m->nmaster ? m->ww * m->mfact : 0;
        ns = m->nmaster > 0 ? 2 : 1;
    } else {
        mw = m->ww;
        ns = 1;
    }
    for (i = ma = sa = my = ty = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++) {
        if (i < m->nmaster) {
            xraycfg( c
                   , em ? XrayGrpTileMaster : XrayGrpNone
                   , xr[0] == c ? (ma = 1, XrayAc) : XrayIn
                   );
            h = em ? m->wh - 2 * ogap : (m->wh - 2 * ogap + igap - my) / (MIN(n, m->nmaster) - i);
            resize( c
                  , m->wx + ogap
                  , m->wy + ogap + my * !em
                  , mw - (ns ^ 3) * ogap - 2 * c->bw - (ns - 1) * (igap / 2)
                  , h - 2 * c->bw - igap * !em
                  , False
                  );
            if (my + HEIGHT(c) + igap < m->wh - 2 * ogap)
                my += HEIGHT(c) + igap;
        } else {
            xraycfg( c
                   , es ? XrayGrpTileStack : XrayGrpNone
                   , xr[1] == c ? (sa = 1, XrayAc) : XrayIn
                   );
            h = es ? m->wh - 2 * ogap : (m->wh - 2 * ogap + igap - ty) / (n - i);
            resize( c
                  , m->wx + (ns & 1) * ogap + mw + (ns - 1) * (igap / 2)
                  , m->wy + ogap + ty * !es
                  , m->ww - (ns ^ 3) * ogap - mw - 2 * c->bw - (ns - 1) * (igap / 2)
                  , h - 2 * c->bw - igap * !es
                  , False
                  );
            if (ty + HEIGHT(c) + igap < m->wh - 2 * ogap)
                ty += HEIGHT(c) + igap;
        }

        xrayapply(c);
    }

    if (!ma && em)
        xrayfallback(m, XrayGrpTileMaster);

    if (!sa && es)
        xrayfallback(m, XrayGrpTileStack);
}

TileInfo
tileinfo(void)
{
    Client *x, *y;
    TileInfo ti = { 0 };

    for (ti.nc = ti.si = 0, x = ti.ts[0] = nexttiled(selmon->clients); x; x = y, ++ti.nc) {
        y = nexttiled(x->next);
        if (x->xraystat == XrayAc) switch (x->xraygrp) {
            case XrayGrpTileMaster : ti.xs[0] = x; break;
            case XrayGrpTileStack  : ti.xs[1] = x; break;
        }
        if (selmon->sel == x) {
            ti.si    = ti.nc;
            ti.cs[1] = y;
        }
        if (selmon->sel == y)
            ti.cs[0] = x;
        if (ti.nc + 1 == selmon->nmaster) {
            ti.bs[0] = x;
            ti.ts[1] = y;
        }
        if (!y)
            ti.bs[1] = x;
    }

    if (selmon->nmaster > ti.nc)
        ti.bs[0] = ti.bs[1];

    if (selmon->nmaster == ti.nc)
        ti.bs[1] = NULL;

    if (selmon->nmaster == 0) {
        ti.ts[1] = ti.ts[0];
        ti.ts[0] = NULL;
    }

    ti.st = ti.si >= selmon->nmaster;

    return ti;
}

Client *
tileffb(Client *c)
{
    TileInfo ti;
    Client *x, *y;
    int i, ms, mn, md;

    if (selmon->lt[selmon->sellt]->arrange != &tile)
        return tileffbfallback ? tileffbfallback(c) : NULL;

    if (!c || selmon->sel != c)
        return NULL;

    ms = tmpnmcfg & (TmpNmPrefer | TmpNmDefer | TmpNmNone);
    mn = (tmpnmcfg & TmpNmNoMove)  != 0;
    md = (tmpnmcfg & TmpNmAndDeny) != 0;
    if ( ( ms != TmpNmPrefer && ms != TmpNmDefer && ms != TmpNmNone )
      || ( mn && !(md && ms == TmpNmPrefer) ) )
        return NULL;

    if (c->isfloating) {
        for ( x = c->mon->stack
            ; x && (!x->isfloating || !ISVISIBLE(x) || x == c)
            ; x = x->snext
            );
        return x;
    }

    ti = tileinfo();
    if (ti.nc == 1)
        return NULL;

    if (ti.xs[ti.st]) {
        for (x = selmon->stack; x; x = x->snext)
            if (ISVISIBLE(x) && !x->isfloating && x != c) {
                for ( i = 0, y = nexttiled(selmon->clients)
                    ; y && y != x
                    ; y = nexttiled(y->next), ++i
                    );
                if ((int[2]){ i <  selmon->nmaster
                            , i >= selmon->nmaster }[ti.st]) {
                    return x;
                }
            }
    }

    if (!ti.st) {
        if (ti.xs[1]) {
            if ( selmon->nmaster > MAX(tmpnmdectil, 0)
              || (md && ms == TmpNmPrefer                         )
              || (md && ms == TmpNmDefer  && ti.xs[1] == ti.bs[1]
                                          && ti.xs[1] == ti.ts[1] )
              || (md && ms == TmpNmNone   && ti.xs[1] == ti.ts[1] ) )
                return selmon->sel == ti.bs[0]
                    ? ti.cs[0] ? ti.cs[0] : ti.xs[1]
                    : ti.cs[1];

            if (ms == TmpNmPrefer)
                return selmon->sel == ti.bs[0] ? ti.xs[1] : ti.cs[1];

            if ( ms == TmpNmDefer     && ti.ts[1] == ti.xs[1]
              && ti.bs[1] != ti.xs[1] && ti.bs[0] == selmon->sel )
                return nexttiled(ti.xs[1]->next);

        } else if (selmon->nmaster > MAX(tmpnmdectil, 0))
            return (x = ti.cs[selmon->sel != ti.bs[0]]) ? x : ti.cs[1];
    }

    return ti.cs[1] ? ti.cs[1] : ti.cs[0];
}

void
tmpnmdec(Client *c, int arg)
{
    TileInfo ti;
    int ms, mn, md, d;

    if ( !c || selmon->sel != c || !(tmpnmcfg & arg) || c->isfloating
      || !ISVISIBLE(c) || selmon->lt[selmon->sellt]->arrange != &tile )
        return;

    ms = tmpnmcfg & (TmpNmPrefer | TmpNmDefer | TmpNmNone);
    mn = (tmpnmcfg & TmpNmNoMove)  != 0;
    md = (tmpnmcfg & TmpNmAndDeny) != 0;
    if ( ( ms != TmpNmPrefer && ms != TmpNmDefer && ms != TmpNmNone )
      || ( mn && !(md && ms == TmpNmPrefer) ) )
        return;

    ti = tileinfo();

    if (!ti.st && tmpnmdectil > 0 && selmon->nmaster <= tmpnmdectil && ti.xs[1]) {
        d = ti.ts[1] == ti.xs[1];
        if (ms == TmpNmDefer && d && !(d = ti.ts[1] == ti.bs[1]))
            cswap(ti.xs[1], nexttiled(ti.xs[1]->next));
        else if (ms == TmpNmPrefer && ((d = 1), !mn))
            cswap(ti.ts[1], ti.xs[1]);
        if (md && d)
            selmon->pertag->nmasters[selmon->pertag->curtag] = --selmon->nmaster;
        return;
    }

    c->mon->nmaster = selmon->pertag->nmasters[selmon->pertag->curtag]
        = (ti.nc == 1 && (tmpnmcfg & TmpNmRespectNmaster)) ? nmaster
        : (ti.nc < selmon->nmaster) ? ti.nc - (ti.nc > MAX(tmpnmdectil, 0))
        : selmon->nmaster - (!ti.st && selmon->nmaster > MAX(tmpnmdectil, 0));
}

void
tilezoom(const Arg *arg)
{
    TileInfo ti;
    Client *x, *y, **bsts[2] = { ti.bs, ti.ts };
    int mm, ms, nc, mnt, mnb, mne, mst, msb;

    if (selmon->lt[selmon->sellt]->arrange != &tile) {
        if (tilezoomfallback)
            tilezoomfallback(arg);
        return;
    }

    mm = tilezoomcfg & ( TileZoomMtSTop | TileZoomMtSBtm | TileZoomMtSFoc
                       | TileZoomMtXDny | TileZoomMtMTop | TileZoomMtMBtm );
    ms = tilezoomcfg & ( TileZoomStMTop | TileZoomStMBtm | TileZoomStMFoc
                       | TileZoomStXDny | TileZoomStSTop | TileZoomStSBtm );
    if ( ( mm != TileZoomMtSTop && mm != TileZoomMtSBtm && mm != TileZoomMtSFoc
        && mm != TileZoomMtXDny && mm != TileZoomMtMTop && mm != TileZoomMtMBtm )
      || ( ms != TileZoomStMTop && ms != TileZoomStMBtm && ms != TileZoomStMFoc
        && ms != TileZoomStXDny && ms != TileZoomStSTop && ms != TileZoomStSBtm )
      || ( selmon->sel && selmon->sel->isfloating ) )
        return;

    if ( ( mm == TileZoomMtMTop || mm == TileZoomMtMBtm )
      && ( tilezoomcfg & TileZoomMtSExp ) )
        return;

    if ( ( ms == TileZoomStSTop || ms == TileZoomStSBtm )
      && ( tilezoomcfg & TileZoomStMExp ) )
        return;

    ti = tileinfo();
    if (tilezoomcfg & (ti.st ? TileZoomStXDny : TileZoomMtXDny))
        return;

    if (ti.nc == 0 || selmon->nmaster == 0 || selmon->nmaster >= ti.nc)
        return;

    mnt = tilezoomcfg & (ti.st ? TileZoomStMTop : TileZoomMtSTop);
    mnb = tilezoomcfg & (ti.st ? TileZoomStMBtm : TileZoomMtSBtm);
    mne = tilezoomcfg & (ti.st ? TileZoomStMExp : TileZoomMtSExp);
    mst = tilezoomcfg & (ti.st ? TileZoomStSTop : TileZoomMtMTop);
    msb = tilezoomcfg & (ti.st ? TileZoomStSBtm : TileZoomMtMBtm);
    if (ti.xs[!ti.st] && mne)
        cswap(selmon->sel, ti.xs[!ti.st]);
    else if (mnt || mnb || mst || msb)
        cswap(selmon->sel, bsts[mnt || mst][(mnt || mnb)^ti.st]);
    else
        for (x = selmon->stack; x; x = x->snext)
            if (ISVISIBLE(x) && !x->isfloating) {
                for ( nc = 0, y = nexttiled(selmon->clients)
                    ; y && y != x
                    ; y = nexttiled(y->next), ++nc );
                if ((int[2]){ nc >= selmon->nmaster
                            , nc <  selmon->nmaster }[ti.st]) {
                    cswap(selmon->sel, x);
                    break;
                }
            }
    arrange(selmon);
}

void
tilenmctl(const Arg *arg)
{
    TileInfo ti;
    Client **bsts[2] = { ti.bs, ti.ts };
    int mm, ms, ma, mn, md, dir, deny;

    if (selmon->lt[selmon->sellt]->arrange != &tile) {
        if (tilenmctlfallback)
            tilenmctlfallback(arg);
        return;
    }

    if (!arg)
        return;

    mm = tilenmctlcfg & (TileNmCtlMasterDefer | TileNmCtlMasterPrefer | TileNmCtlMasterNone);
    ms = tilenmctlcfg & (TileNmCtlStackDefer  | TileNmCtlStackPrefer  | TileNmCtlStackNone );
    ma = arg->i & (TileNmCtlInc | TileNmCtlDec);
    if ( ( mm != TileNmCtlMasterDefer && mm != TileNmCtlMasterPrefer && mm != TileNmCtlMasterNone )
      || ( ms != TileNmCtlStackDefer  && ms != TileNmCtlStackPrefer  && ms != TileNmCtlStackNone  )
      || ( ma != TileNmCtlInc         && ma != TileNmCtlDec                                       )
      || ( (tilenmctlcfg & TileNmCtlMasterNoMove)
        && !( (tilenmctlcfg & TileNmCtlMasterAndDeny) && mm == TileNmCtlMasterPrefer) )
      || ( (tilenmctlcfg & TileNmCtlStackNoMove )
        && !( (tilenmctlcfg & TileNmCtlStackAndDeny ) && ms == TileNmCtlStackPrefer ) )
       ) return;

    dir = (arg->i & TileNmCtlInc) != 0;
    if (!dir && selmon->nmaster <= 0)
        return;

    ti   = tileinfo();
    deny = 0;
    md   = tilenmctlcfg & (dir ? TileNmCtlStackAndDeny : TileNmCtlMasterAndDeny);
    if (ti.xs[dir] && (int[2]){ selmon->nmaster <= ti.nc
                              , selmon->nmaster <  ti.nc }[dir]) {
        deny = bsts[dir][dir] == ti.xs[dir];
        if (tilenmctlcfg & (dir ? TileNmCtlStackDefer : TileNmCtlMasterDefer)) {
            if (deny && !(deny = ti.bs[dir] == ti.ts[dir]))
                cswap(ti.xs[dir], dir ? nexttiled(ti.xs[1]->next) : cprevtiled(ti.xs[0]));
        } else if (tilenmctlcfg & (dir ? TileNmCtlStackPrefer : TileNmCtlMasterPrefer)) {
            mn   = tilenmctlcfg & (dir ? TileNmCtlStackNoMove : TileNmCtlMasterNoMove);
            deny = 1;
            if (bsts[dir][dir] != ti.xs[dir] && !mn)
                cswap(bsts[dir][dir], ti.xs[dir]);
        }
    }

    if (!md || !deny) {
        selmon->pertag->nmasters[selmon->pertag->curtag] = selmon->nmaster += dir ? 1 : -1;
        if (!(arg->i & TileNmCtlNoArrange))
            arrange(selmon);
    }
}

void
tilenmctldefaultfallback(const Arg *arg)
{
    Arg a;

    if (!arg)
        return;

    switch (arg->i) {
        case TileNmCtlInc: a.i = +1; break;
        case TileNmCtlDec: a.i = -1; break;
        default: return;
    }

    incnmaster(&a);
}

void
tilemove(const Arg *arg)
{
    TileInfo ti;
    Client *c, **bsts[2] = { ti.bs, ti.ts };
    int mm, mx, ms, mt, north, south, west, east, of, dir;

    if (selmon->lt[selmon->sellt]->arrange != &tile) {
        if (tilemovefallback)
            tilemovefallback(arg);
        return;
    }

    if (!arg || !selmon->sel || selmon->sel->isfloating ||
            (selmon->sel->isfullscreen && lockfullscreen))
        return;

    mm = tilemovecfg & (TileMoveMasterTop | TileMoveMasterBottom);
    ms = tilemovecfg & (TileMoveStackTop  | TileMoveStackBottom );
    if ( (mm != TileMoveMasterTop && mm != TileMoveMasterBottom)
      || (ms != TileMoveStackTop  && ms != TileMoveStackBottom ) )
        return;

    ti = tileinfo();
    if (ti.nc == 0)
        return;

    north = arg->i == 'N' || arg->i == 'n';
    south = arg->i == 'S' || arg->i == 's';
    mx    = tilemovecfg & (ti.st ? TileMoveStackXDeny : TileMoveMasterXDeny);
    if (north || south) {
        if (mx && ti.xs[ti.st])
            return;
        c = (bsts[north][ti.st] == selmon->sel)
          ? (tilemovecfg & TileMoveVertOverflow)
          ? bsts[south][ti.st] : NULL : ti.cs[south];
        if (c) {
            cswap(c, selmon->sel);
            arrange(selmon);
        }
        return;
    }

    west = arg->i == 'W' || arg->i == 'w';
    east = arg->i == 'E' || arg->i == 'e';
    of   = tilemovecfg & TileMoveHorzOverflow;
    dir  = ( ti.st && west) ? 0
         : (!ti.st && west) ? (of ? 1 : -1)
         : ( ti.st && east) ? (of ? 0 : -1)
         : (!ti.st && east) ? 1
         : -1
         ;

    if (!(west || east) || dir == -1)
        return;

    selmon->nmaster = selmon->pertag->nmasters[selmon->pertag->curtag]
       = dir ? MIN(selmon->nmaster, ti.nc) - 1 : selmon->nmaster + 1;
    mt = (tilemovecfg & (dir ? TileMoveStackTop : TileMoveMasterTop)) != 0;
    c  = bsts[mt][dir];
    cmove(selmon->sel, c ? c : bsts[!dir][!dir], c ? !mt : dir);
    arrange(selmon);
}

void
tilefocus(const Arg *arg)
{
    TileInfo ti;
    Client *x, *y, **bsts[2] = { ti.bs, ti.ts };
    int nc, mm, ms, mt, mb, north, south, east, west, dir, of;

    if (selmon->lt[selmon->sellt]->arrange != &tile) {
        if (tilefocusfallback)
            tilefocusfallback(arg);
        return;
    }

    if (!arg || !selmon->sel)
        return;

    mm = tilefocuscfg & (TileFocusMasterTop | TileFocusMasterBottom | TileFocusMasterLast);
    ms = tilefocuscfg & (TileFocusStackTop  | TileFocusStackBottom  | TileFocusStackLast );
    if ( ( mm != TileFocusMasterTop && mm != TileFocusMasterBottom && mm != TileFocusMasterLast )
      || ( ms != TileFocusStackTop  && ms != TileFocusStackBottom  && ms != TileFocusStackLast  ) )
        return;

    if (selmon->sel->isfloating) {
        if (!(selmon->sel->isfullscreen && lockfullscreen)) {
            for (x = selmon->stack; x && (!ISVISIBLE(x) || x->isfloating); x = x->snext);
            focusrestack(x);
        }
        return;
    }

    ti = tileinfo();
    if (ti.nc == 0)
        return;

    north = arg->i == 'N' || arg->i == 'n';
    south = arg->i == 'S' || arg->i == 's';
    if (north || south) {
        focusrestack((int[2]){ ti.si == (ti.st ? selmon->nmaster : 0)
                             , ti.si + 1 == selmon->nmaster || !ti.cs[1] }[south]
            ? (tilefocuscfg & TileFocusVertOverflow)
            ? bsts[south][ti.st] : NULL : ti.cs[south]);
        return;
    }

    if (selmon->nmaster == 0 || ti.nc <= selmon->nmaster)
        return;

    west = arg->i == 'W' || arg->i == 'w';
    east = arg->i == 'E' || arg->i == 'e';
    of   = tilefocuscfg & TileFocusHorzOverflow;
    dir  = ( ti.st && west) ? 0
         : (!ti.st && west) ? (of ? 1 : -1)
         : ( ti.st && east) ? (of ? 0 : -1)
         : (!ti.st && east) ? 1
         : -1
         ;

    if (!(west || east) || dir == -1)
        return;

    if (tilefocuscfg & (dir ? TileFocusStackXrayAc : TileFocusMasterXrayAc) && ti.xs[dir]) {
        focusrestack(ti.xs[dir]);
        return;
    }

    mt = tilefocuscfg & (dir ? TileFocusStackTop    : TileFocusMasterTop   );
    mb = tilefocuscfg & (dir ? TileFocusStackBottom : TileFocusMasterBottom);
    if (mt || mb) {
        focusrestack(bsts[mt != 0][dir]);
        return;
    }

    for (x = selmon->stack; x; x = x->snext)
        if (ISVISIBLE(x) && !x->isfloating) {
            for (nc = 0, y = nexttiled(selmon->clients)
                    ; y && y != x
                    ; y = nexttiled(y->next), ++nc);
            if ((int[2]){ nc <  selmon->nmaster
                        , nc >= selmon->nmaster }[dir]) {
                focusrestack(x);
                return;
            }
        }
}

void
tilefocusdefaultfallback(const Arg *arg)
{
    Arg a;

    if (!arg)
        return;

    switch (arg->i) {
        case 'N':
        case 'n':
            a.i = -1;
            break;
        case 'S':
        case 's':
            a.i = +1;
            break;
        default:
            return;
    }

    focusstack(&a);
}

void
togglebar(const Arg *arg)
{
	selmon->showbar = selmon->pertag->showbars[selmon->pertag->curtag] = !selmon->showbar;
	updatebarpos(selmon);
	XMoveResizeWindow(dpy, selmon->barwin, selmon->wx + sp, selmon->by + vp, selmon->ww - 2 * sp, bh);
	arrange(selmon);
}

void
togglefloating(const Arg *arg)
{
	if (!selmon->sel)
		return;
	if (selmon->sel->isfullscreen) /* no support for fullscreen windows */
		return;
    tmpnmdec(selmon->sel, TmpNmHookToggleFloating);
	selmon->sel->isfloating = !selmon->sel->isfloating || selmon->sel->isfixed;
	if (selmon->sel->isfloating) {
        xraycfg(selmon->sel, XrayGrpNone, -1);
		resize(selmon->sel, selmon->sel->x, selmon->sel->y,
			selmon->sel->w, selmon->sel->h, 0);
    }
	arrange(selmon);
}

void
togglefullscr(const Arg *arg)
{
  if(selmon->sel)
    setfullscreen(selmon->sel, !selmon->sel->isfullscreen);
}

void
toggletag(const Arg *arg)
{
	unsigned int newtags;

	if (!selmon->sel || SP_ISSPAD(selmon->sel))
		return;
	newtags = selmon->sel->tags ^ (arg->ui & TAGMASK);
	if (newtags) {
		selmon->sel->tags = newtags;
		focus(NULL);
		arrange(selmon);
	}
}

void
toggleview(const Arg *arg)
{
	unsigned int newtagset = selmon->tagset[selmon->seltags] ^ (arg->ui & TAGMASK);
	int i;

	if (newtagset) {
		selmon->tagset[selmon->seltags] = newtagset;

		if (newtagset == ~0) {
			selmon->pertag->prevtag = selmon->pertag->curtag;
			selmon->pertag->curtag = 0;
		}

		/* test if the user did not select the same tag */
		if (!(newtagset & 1 << (selmon->pertag->curtag - 1))) {
			selmon->pertag->prevtag = selmon->pertag->curtag;
			for (i = 0; !(newtagset & 1 << i); i++) ;
			selmon->pertag->curtag = i + 1;
		}

		/* apply settings for this view */
		selmon->nmaster = selmon->pertag->nmasters[selmon->pertag->curtag];
		selmon->mfact = selmon->pertag->mfacts[selmon->pertag->curtag];
		selmon->sellt = selmon->pertag->sellts[selmon->pertag->curtag];
		selmon->lt[selmon->sellt] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt];
		selmon->lt[selmon->sellt^1] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt^1];

		if (selmon->showbar != selmon->pertag->showbars[selmon->pertag->curtag])
			togglebar(NULL);

		focus(NULL);
		arrange(selmon);
	}
}

void
unfocus(Client *c, int setfocus)
{
	if (!c)
		return;
	grabbuttons(c, 0);
	lastfocused = c;
	if (setfocus) {
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
		XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
	}
    xrayunfocus(c);
}

void
unmanage(Client *c, int destroyed)
{
    Client *nc = NULL;
    SpState *sps;
	Monitor *m = c->mon;
	XWindowChanges wc;
    int i;

    for (i = 0; i < LENGTH(sp_states); ++i)
        if (sp_states[i].client == c) {
            sp_states[i].client = NULL;
            sp_states[i].pid    = -1;
            m->tagset[m->seltags] &= ~sp_tag_vis;
            break;
        }

    if ((sps = sp_active(m)) && (sps->spad->close & SP_C_CLOSE))
		sp_hide(m);

	if (c->swallowing) {
		unswallow(c);
		return;
	}

	Client *s = swallowingclient(c->win);
	if (s) {
		free(s->swallowing);
		s->swallowing = NULL;
		arrange(m);
		focus(NULL);
		return;
	}

    nc = tileffb(c);
    tmpnmdec(c, TmpNmHookUnmanage);
	detach(c);
	detachstack(c);
	if (!destroyed) {
		wc.border_width = c->oldbw;
		XGrabServer(dpy); /* avoid race conditions */
		XSetErrorHandler(xerrordummy);
		XSelectInput(dpy, c->win, NoEventMask);
		XConfigureWindow(dpy, c->win, CWBorderWidth, &wc); /* restore border */
		XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
		setclientstate(c, WithdrawnState);
		XSync(dpy, False);
		XSetErrorHandler(xerror);
		XUngrabServer(dpy);
	}
	if (lastfocused == c)
		lastfocused = NULL;
	free(c);

	if (!s) {
		arrange(m);
    	focus(nc);
		updateclientlist();
	}
}

void
unmapnotify(XEvent *e)
{
	Client *c;
	XUnmapEvent *ev = &e->xunmap;

	if ((c = wintoclient(ev->window))) {
		if (ev->send_event)
			setclientstate(c, WithdrawnState);
		else
			unmanage(c, 0);
	}
}

void
updatebars(void)
{
	Monitor *m;
	XSetWindowAttributes wa = {
		.override_redirect = True,
		.background_pixmap = ParentRelative,
		.event_mask = ButtonPressMask|ExposureMask
	};
	XClassHint ch = {"dwm", "dwm"};
	for (m = mons; m; m = m->next) {
		if (m->barwin)
			continue;
		m->barwin = XCreateWindow(dpy, root, m->wx + sp, m->by + vp, m->ww - 2 * sp, bh, 0, DefaultDepth(dpy, screen),
				CopyFromParent, DefaultVisual(dpy, screen),
				CWOverrideRedirect|CWBackPixmap|CWEventMask, &wa);
		XDefineCursor(dpy, m->barwin, cursor[CurNormal]->cursor);
		XMapRaised(dpy, m->barwin);
		XSetClassHint(dpy, m->barwin, &ch);
	}
}

void
updatebarpos(Monitor *m)
{
	m->wy = m->my;
	m->wh = m->mh;
	if (m->showbar) {
		m->wh = m->wh - vertpad - bh;
		m->by = m->topbar ? m->wy : m->wy + m->wh + vertpad;
		m->wy = m->topbar ? m->wy + bh + vp : m->wy;
	} else
		m->by = -bh - vp;
}

void
updateclientlist(void)
{
	Client *c;
	Monitor *m;

	XDeleteProperty(dpy, root, netatom[NetClientList]);
	for (m = mons; m; m = m->next)
		for (c = m->clients; c; c = c->next)
			XChangeProperty(dpy, root, netatom[NetClientList],
				XA_WINDOW, 32, PropModeAppend,
				(unsigned char *) &(c->win), 1);
}

int
updategeom(void)
{
	int dirty = 0;

#ifdef XINERAMA
	if (XineramaIsActive(dpy)) {
		int i, j, n, nn;
		Client *c;
		Monitor *m;
		XineramaScreenInfo *info = XineramaQueryScreens(dpy, &nn);
		XineramaScreenInfo *unique = NULL;

		for (n = 0, m = mons; m; m = m->next, n++);
		/* only consider unique geometries as separate screens */
		unique = ecalloc(nn, sizeof(XineramaScreenInfo));
		for (i = 0, j = 0; i < nn; i++)
			if (isuniquegeom(unique, j, &info[i]))
				memcpy(&unique[j++], &info[i], sizeof(XineramaScreenInfo));
		XFree(info);
		nn = j;

		/* new monitors if nn > n */
		for (i = n; i < nn; i++) {
			for (m = mons; m && m->next; m = m->next);
			if (m)
				m->next = createmon();
			else
				mons = createmon();
		}
		for (i = 0, m = mons; i < nn && m; m = m->next, i++)
			if (i >= n
			|| unique[i].x_org != m->mx || unique[i].y_org != m->my
			|| unique[i].width != m->mw || unique[i].height != m->mh)
			{
				dirty = 1;
				m->num = i;
				m->mx = m->wx = unique[i].x_org;
				m->my = m->wy = unique[i].y_org;
				m->mw = m->ww = unique[i].width;
				m->mh = m->wh = unique[i].height;
				updatebarpos(m);
			}
		/* removed monitors if n > nn */
		for (i = nn; i < n; i++) {
			for (m = mons; m && m->next; m = m->next);
			while ((c = m->clients)) {
				dirty = 1;
				m->clients = c->next;
				detachstack(c);
				c->mon = mons;
				attachbottom(c);
				attachstack(c);
			}
			if (m == selmon)
				selmon = mons;
			cleanupmon(m);
		}
		free(unique);
	} else
#endif /* XINERAMA */
	{ /* default monitor setup */
		if (!mons)
			mons = createmon();
		if (mons->mw != sw || mons->mh != sh) {
			dirty = 1;
			mons->mw = mons->ww = sw;
			mons->mh = mons->wh = sh;
			updatebarpos(mons);
		}
	}
	if (dirty) {
		selmon = mons;
		selmon = wintomon(root);
	}
	return dirty;
}

void
updatenumlockmask(void)
{
	unsigned int i, j;
	XModifierKeymap *modmap;

	numlockmask = 0;
	modmap = XGetModifierMapping(dpy);
	for (i = 0; i < 8; i++)
		for (j = 0; j < modmap->max_keypermod; j++)
			if (modmap->modifiermap[i * modmap->max_keypermod + j]
				== XKeysymToKeycode(dpy, XK_Num_Lock))
				numlockmask = (1 << i);
	XFreeModifiermap(modmap);
}

void
updatesizehints(Client *c)
{
	long msize;
	XSizeHints size;

	if (!XGetWMNormalHints(dpy, c->win, &size, &msize))
		/* size is uninitialized, ensure that size.flags aren't used */
		size.flags = PSize;
	if (size.flags & PBaseSize) {
		c->basew = size.base_width;
		c->baseh = size.base_height;
	} else if (size.flags & PMinSize) {
		c->basew = size.min_width;
		c->baseh = size.min_height;
	} else
		c->basew = c->baseh = 0;
	if (size.flags & PResizeInc) {
		c->incw = size.width_inc;
		c->inch = size.height_inc;
	} else
		c->incw = c->inch = 0;
	if (size.flags & PMaxSize) {
		c->maxw = size.max_width;
		c->maxh = size.max_height;
	} else
		c->maxw = c->maxh = 0;
	if (size.flags & PMinSize) {
		c->minw = size.min_width;
		c->minh = size.min_height;
	} else if (size.flags & PBaseSize) {
		c->minw = size.base_width;
		c->minh = size.base_height;
	} else
		c->minw = c->minh = 0;
	if (size.flags & PAspect) {
		c->mina = (float)size.min_aspect.y / size.min_aspect.x;
		c->maxa = (float)size.max_aspect.x / size.max_aspect.y;
	} else
		c->maxa = c->mina = 0.0;
	c->isfixed = (c->maxw && c->maxh && c->maxw == c->minw && c->maxh == c->minh);
	c->hintsvalid = 1;
}

void
updatestatus(void)
{
	Monitor* m;
	if (!gettextprop(root, XA_WM_NAME, stext, sizeof(stext)))
		strcpy(stext, "dwm-"VERSION);
	for(m = mons; m; m = m->next)
		drawbar(m);
}

void
updatetitle(Client *c)
{
	if (!gettextprop(c->win, netatom[NetWMName], c->name, sizeof c->name))
		gettextprop(c->win, XA_WM_NAME, c->name, sizeof c->name);
	if (c->name[0] == '\0') /* hack to mark broken clients */
		strcpy(c->name, broken);
}

void
updatewindowtype(Client *c)
{
	Atom state = getatomprop(c, netatom[NetWMState]);
	Atom wtype = getatomprop(c, netatom[NetWMWindowType]);

	if (state == netatom[NetWMFullscreen])
		setfullscreen(c, 1);
	if (wtype == netatom[NetWMWindowTypeDialog])
		c->isfloating = 1;
}

void
updatewmhints(Client *c)
{
	XWMHints *wmh;

	if ((wmh = XGetWMHints(dpy, c->win))) {
		if (c == selmon->sel && wmh->flags & XUrgencyHint) {
			wmh->flags &= ~XUrgencyHint;
			XSetWMHints(dpy, c->win, wmh);
		} else
			c->isurgent = (wmh->flags & XUrgencyHint) ? 1 : 0;
		if (wmh->flags & InputHint)
			c->neverfocus = !wmh->input;
		else
			c->neverfocus = 0;
		XFree(wmh);
	}
}

void
view(const Arg *arg)
{
	int i;
	unsigned int tmptag;

	if ((arg->ui & TAGMASK) == (selmon->tagset[selmon->seltags] & ~sp_tag_vis))
		return;
    selmon->tagset[selmon->seltags] &= ~sp_tag_vis;
	selmon->seltags ^= 1; /* toggle sel tagset */
	if (arg->ui & TAGMASK) {
		selmon->tagset[selmon->seltags] = arg->ui & TAGMASK;
		selmon->pertag->prevtag = selmon->pertag->curtag;

		if (arg->ui == ~0)
			selmon->pertag->curtag = 0;
		else {
			for (i = 0; !(arg->ui & 1 << i); i++) ;
			selmon->pertag->curtag = i + 1;
		}
	} else {
		tmptag = selmon->pertag->prevtag;
		selmon->pertag->prevtag = selmon->pertag->curtag;
		selmon->pertag->curtag = tmptag;
	}

	selmon->nmaster = selmon->pertag->nmasters[selmon->pertag->curtag];
	selmon->mfact = selmon->pertag->mfacts[selmon->pertag->curtag];
	selmon->sellt = selmon->pertag->sellts[selmon->pertag->curtag];
	selmon->lt[selmon->sellt] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt];
	selmon->lt[selmon->sellt^1] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt^1];

	if (selmon->showbar != selmon->pertag->showbars[selmon->pertag->curtag])
		togglebar(NULL);

    sp_repaint = 0;
	focus(NULL);
    sp_repaint = 1;
	arrange(selmon);
}

pid_t
winpid(Window w)
{

	pid_t result = 0;

#ifdef __linux__
	xcb_res_client_id_spec_t spec = {0};
	spec.client = w;
	spec.mask = XCB_RES_CLIENT_ID_MASK_LOCAL_CLIENT_PID;

	xcb_generic_error_t *e = NULL;
	xcb_res_query_client_ids_cookie_t c = xcb_res_query_client_ids(xcon, 1, &spec);
	xcb_res_query_client_ids_reply_t *r = xcb_res_query_client_ids_reply(xcon, c, &e);

	if (!r)
		return (pid_t)0;

	xcb_res_client_id_value_iterator_t i = xcb_res_query_client_ids_ids_iterator(r);
	for (; i.rem; xcb_res_client_id_value_next(&i)) {
		spec = i.data->spec;
		if (spec.mask & XCB_RES_CLIENT_ID_MASK_LOCAL_CLIENT_PID) {
			uint32_t *t = xcb_res_client_id_value_value(i.data);
			result = *t;
			break;
		}
	}

	free(r);

	if (result == (pid_t)-1)
		result = 0;

#endif /* __linux__ */

#ifdef __OpenBSD__
        Atom type;
        int format;
        unsigned long len, bytes;
        unsigned char *prop;
        pid_t ret;

        if (XGetWindowProperty(dpy, w, XInternAtom(dpy, "_NET_WM_PID", 0), 0, 1, False, AnyPropertyType, &type, &format, &len, &bytes, &prop) != Success || !prop)
               return 0;

        ret = *(pid_t*)prop;
        XFree(prop);
        result = ret;

#endif /* __OpenBSD__ */
	return result;
}

pid_t
getparentprocess(pid_t p)
{
	unsigned int v = 0;

#ifdef __linux__
	FILE *f;
	char buf[256];
	snprintf(buf, sizeof(buf) - 1, "/proc/%u/stat", (unsigned)p);

	if (!(f = fopen(buf, "r")))
		return 0;

	fscanf(f, "%*u %*s %*c %u", &v);
	fclose(f);
#endif /* __linux__*/

#ifdef __OpenBSD__
	int n;
	kvm_t *kd;
	struct kinfo_proc *kp;

	kd = kvm_openfiles(NULL, NULL, NULL, KVM_NO_FILES, NULL);
	if (!kd)
		return 0;

	kp = kvm_getprocs(kd, KERN_PROC_PID, p, sizeof(*kp), &n);
	v = kp->p_ppid;
#endif /* __OpenBSD__ */

	return (pid_t)v;
}

int
isdescprocess(pid_t p, pid_t c)
{
	while (p != c && c != 0)
		c = getparentprocess(c);

	return (int)c;
}

Client *
termforwin(const Client *w)
{
	Client *c;
	Monitor *m;

	if (!w->pid || w->isterminal)
		return NULL;

	for (m = mons; m; m = m->next) {
		for (c = m->clients; c; c = c->next) {
			if (c->isterminal && !c->swallowing && c->pid && isdescprocess(c->pid, w->pid))
				return c;
		}
	}

	return NULL;
}

Client *
swallowingclient(Window w)
{
	Client *c;
	Monitor *m;

	for (m = mons; m; m = m->next) {
		for (c = m->clients; c; c = c->next) {
			if (c->swallowing && c->swallowing->win == w)
				return c;
		}
	}

	return NULL;
}

Client *
wintoclient(Window w)
{
	Client *c;
	Monitor *m;

	for (m = mons; m; m = m->next)
		for (c = m->clients; c; c = c->next)
			if (c->win == w)
				return c;
	return NULL;
}

Monitor *
wintomon(Window w)
{
	int x, y;
	Client *c;
	Monitor *m;

	if (w == root && getrootptr(&x, &y))
		return recttomon(x, y, 1, 1);
	for (m = mons; m; m = m->next)
		if (w == m->barwin)
			return m;
	if ((c = wintoclient(w)))
		return c->mon;
	return selmon;
}

/* There's no way to check accesses to destroyed windows, thus those cases are
 * ignored (especially on UnmapNotify's). Other types of errors call Xlibs
 * default error handler, which may call exit. */
int
xerror(Display *dpy, XErrorEvent *ee)
{
	if (ee->error_code == BadWindow
	|| (ee->request_code == X_SetInputFocus && ee->error_code == BadMatch)
	|| (ee->request_code == X_PolyText8 && ee->error_code == BadDrawable)
	|| (ee->request_code == X_PolyFillRectangle && ee->error_code == BadDrawable)
	|| (ee->request_code == X_PolySegment && ee->error_code == BadDrawable)
	|| (ee->request_code == X_ConfigureWindow && ee->error_code == BadMatch)
	|| (ee->request_code == X_GrabButton && ee->error_code == BadAccess)
	|| (ee->request_code == X_GrabKey && ee->error_code == BadAccess)
	|| (ee->request_code == X_CopyArea && ee->error_code == BadDrawable))
		return 0;
	fprintf(stderr, "dwm: fatal error: request code=%d, error code=%d\n",
		ee->request_code, ee->error_code);
	return xerrorxlib(dpy, ee); /* may call exit */
}

int
xerrordummy(Display *dpy, XErrorEvent *ee)
{
	return 0;
}

/* Startup Error handler to check if another window manager
 * is already running. */
int
xerrorstart(Display *dpy, XErrorEvent *ee)
{
	die("dwm: another window manager is already running");
	return -1;
}

Client *
xrayac(Monitor *mon, int grp)
{
    return xrayacs(mon, grp, NULL);
}

Client *
xrayacs(Monitor *mon, int grp, Client *skip)
{
    Client *c;

    if (!mon)
        return NULL;

    for (c = mon->clients; c; c = c->next)
        if (c != skip && ISVISIBLE(c) && c->xraygrp == grp && c->xraystat == XrayAc)
            break;

    return c;
}

void
xrayapply(Client *c)
{
    if (c && xrayhandlers[c->xraygrp])
        xrayhandlers[c->xraygrp](c);
}

void
xraycfg(Client *c, int grp, int stat)
{
    if (!c)
        return;

    if (grp == XrayGrpNone) {
        c->xraygrp  = XrayGrpNone;
        c->xraystat = 0;
        return;
    }

    c->xraygrp  = grp;
    c->xraystat = stat;
}

void
xraydefhandler(Client *c)
{
    switch (c->xraystat) {
        case XrayIn:
            XMoveWindow(dpy, c->win, WIDTH(c) * -2, c->y);
            break;
        case XrayAc:
            XMoveWindow(dpy, c->win, c->x, c->y);
            break;
    }
}

Client *
xrayfallback(Monitor *mon, int grp)
{
    Client *c;

    if (grp == XrayGrpNone)
        return NULL;

    for (c = mon->stack; c; c = c->snext)
        if (ISVISIBLE(c) && c->xraygrp == grp) {
            xraycfg(c, grp, XrayAc);
            xrayapply(c);
            return c;
        }

    return NULL;
}

void
xrayfocus(Client *c)
{
    Client *i;

    if (!c || c->xraygrp == XrayGrpNone)
        return;

    xraycfg(c, c->xraygrp, XrayAc);
    xrayapply(c);
    for (i = c->mon->clients; i; i = i->next)
        if (i != c && ISVISIBLE(i) && i->xraygrp == c->xraygrp && i->xraystat == XrayAc) {
            xraycfg(i, i->xraygrp, XrayIn);
            xrayapply(i);
        }
}

Client *
xraymaybefallback(Monitor *mon, int grp)
{
    Client *c;

    if (grp == XrayGrpNone)
        return NULL;

    c = xrayac(mon, grp);
    return c ? c : xrayfallback(mon, grp);
}

void
xrayunfocus(Client *c)
{
    if (!c || c->xraygrp == XrayGrpNone)
        return;

    if (xrayacs(c->mon, c->xraygrp, c)) {
        xraycfg(c, c->xraygrp, XrayIn);
        xrayapply(c);
    }
}

void
zoom(const Arg *arg)
{
	Client *c = selmon->sel;

	if (!selmon->lt[selmon->sellt]->arrange || !c || c->isfloating)
		return;
	if (c == nexttiled(selmon->clients) && !(c = nexttiled(c->next)))
		return;
	pop(c);
}

void
swapmon(const Arg *arg)
{
    Monitor *arr[2], *m1, *m2, *i;
    int x;

    if (!arg || !mons->next)
        return;

    m1 = selmon;

    for ( m2 = NULL, i = mons
        ; i && i != m1
        ; m2 = i, i = i->next );

    if ((arg->i > 0 && m1->next) || !m2)
        m2 = m1->next;

    if (m1->sel)
        unfocus(m1->sel, 1);

    unsigned int tmp_ts[2] = { m1->tagset[0], m1->tagset[1] };
    m1->tagset[0] = m2->tagset[0];
    m1->tagset[1] = m2->tagset[1];
    m2->tagset[0] = tmp_ts[0];
    m2->tagset[1] = tmp_ts[1];

    const Layout *tmp_lt[2] = { m1->lt[0], m1->lt[1] };
    m1->lt[0] = m2->lt[0];
    m1->lt[1] = m2->lt[1];
    m2->lt[0] = tmp_lt[0];
    m2->lt[1] = tmp_lt[1];

    Client *c;
    for (c = m1->clients; c; c = c->next)
        c->mon = m2;
    for (c = m2->clients; c; c = c->next)
        c->mon = m1;

#define SWAP_(type, member) \
    type tmp_##member = m1->member; \
    m1->member = m2->member; \
    m2->member = tmp_##member;

    SWAP_(Client *, clients);
    SWAP_(Client *, stack);
    SWAP_(Pertag *, pertag);
    SWAP_(Client *, sel);
    SWAP_(int, showbar);
    SWAP_(int, topbar);
    SWAP_(float, mfact);
    SWAP_(int, nmaster);
    SWAP_(unsigned int, sellt);
    SWAP_(unsigned int, seltags);

#undef SWAP_

    arr[0] = m1; arr[1] = m2;
    for (x = 0; x < 2; ++x) {
        updatebarpos(arr[x]);
        XMoveResizeWindow(
            dpy,
            arr[x]->barwin,
            arr[x]->wx + sp,
            arr[x]->by + vp,
            arr[x]->ww - 2 * sp,
            bh
        );
        arrange(arr[x]);
    }

    focus(m1->sel);
}

int
main(int argc, char *argv[])
{
	if (argc == 2 && !strcmp("-v", argv[1]))
		die("dwm-"VERSION);
	else if (argc != 1)
		die("usage: dwm [-v]");
	if (!setlocale(LC_CTYPE, "") || !XSupportsLocale())
		fputs("warning: no locale support\n", stderr);
	if (!(dpy = XOpenDisplay(NULL)))
		die("dwm: cannot open display");
	if (!(xcon = XGetXCBConnection(dpy)))
		die("dwm: cannot get xcb connection\n");
	checkotherwm();
	setup();
#ifdef __OpenBSD__
	if (pledge("stdio rpath proc exec ps", NULL) == -1)
		die("pledge");
#endif /* __OpenBSD__ */
	scan();
	run();
	if(restart) execvp(argv[0], argv);
	cleanup();
	XCloseDisplay(dpy);
	return EXIT_SUCCESS;
}
