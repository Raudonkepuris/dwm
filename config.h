#include <X11/XF86keysym.h>

/* See LICENSE file for copyright and license details. */

/* appearance */
static unsigned int borderpx  = 4;        /* border pixel of windows */
static unsigned int gappx     = 6;        /* gaps between windows */
static unsigned int snap      = 32;       /* snap pixel */
static unsigned int systraypinning = 0;   /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static unsigned int systrayonleft  = 0;   /* 0: systray in the right corner, >0: systray on left of status text */
static unsigned int systrayspacing = 2;   /* systray spacing */
static int systraypinningfailfirst = 1;   /* 1: if pinning fails, display systray on the first monitor, False: display systray on the last monitor*/
static int showsystray        = 1;        /* 0 means no systray */
static int vertpad            = 10;       /* vertical padding of bar */
static int sidepad            = 10;       /* horizontal padding of bar */
static int showbar            = 1;        /* 0 means no bar */
static int topbar             = 1;        /* 0 means bottom bar */
static int barheight          = 6;        /* 2 is the default spacing around the bar's font */
static char font[]            = "monospace:size=11";
static char dmenufont[]            = "monospace:size=11";
static const char *fonts[]          = { font, "NotoColorEmoji:pixelsize=11:antialias=true:autohint=true" };
static char normbgcolor[]           = "#222222";
static char normbordercolor[]       = "#444444";
static char normfgcolor[]           = "#bbbbbb";
static char selfgcolor[]            = "#eeeeee";
static char selbordercolor[]        = "#005577";
static char selbgcolor[]            = "#005577";
static char *colors[][3] = {
       /*               fg           bg           border   */
       [SchemeNorm] = { normfgcolor, normbgcolor, normbordercolor },
       [SchemeSel]  = { selfgcolor,  selbgcolor,  selbordercolor  },
 };

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };
static const char *tagsalt[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };
static const int momentaryalttags = 0; /* 1 means alttags will show only when key is held down*/

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class           instance    title       tags mask     iscentered   isfloating   monitor */
	{ "Gimp",          NULL,       NULL,       0,            0,           1,           -1 },
};

/* layout(s) */
static float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static int nmaster     = 1;    /* number of clients in master area */
static int resizehints = 1;    /* 1 means respect size hints in tiled resizals */
static int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */
static int refreshrate = 120;  /* refresh rate (per second) for client move/resize */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

#define STATUSBAR "dwmblocks"

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", normbgcolor, "-nf", normfgcolor, "-sb", selbordercolor, "-sf", selfgcolor, NULL };
static const char *termcmd[]  = { "alacritty", NULL };
static const char *browsercmd[]  = { "firefox", NULL };

static const char *scratchpadnames[] = {
    "scratchpad",
    "bluetui scratchpad",
	"ranger scratchpad",
	"impala scratchpad",
	"mixer scratchpad",
};

static const char *scratchpadcmds[][10] = {
    { "alacritty", 
		"-o", "window.dimensions.columns=120",
		"-o", "window.dimensions.lines=34", 
		"-t", "scratchpad", NULL },
    { "alacritty",
		"-o", "window.dimensions.columns=120",
		"-o", "window.dimensions.lines=34", 
		"-t", "bluetui scratchpad",
		"-e", "bluetui", NULL },
	{ "alacritty",
		"-o", "window.dimensions.columns=120",
		"-o", "window.dimensions.lines=34", 
		"-t", "ranger scratchpad",
		"-e", "ranger", NULL },
	{ "alacritty",
	    "-o", "window.dimensions.columns=120",
		"-o", "window.dimensions.lines=34", 
		"-t", "impala scratchpad",
		"-e", "impala", NULL },
	{ "alacritty",
	    "-o", "window.dimensions.columns=120",
		"-o", "window.dimensions.lines=34", 
		"-t", "mixer scratchpad",
		"-e", "ncpamixer", NULL },
	};

/*
 * Xresources preferences to load at startup
 */
ResourcePref resources[] = {
		{ "font",                    STRING,  &font },
		{ "dmenufont",               STRING,  &dmenufont },
		{ "normbgcolor",             STRING,  &normbgcolor },
		{ "normbordercolor",         STRING,  &normbordercolor },
		{ "normfgcolor",             STRING,  &normfgcolor },
		{ "selbgcolor",              STRING,  &selbgcolor },
		{ "selbordercolor",          STRING,  &selbordercolor },
		{ "selfgcolor",              STRING,  &selfgcolor },
		{ "borderpx",          	     INTEGER, &borderpx },
		{ "gappx",                   INTEGER, &gappx },
		{ "snap",          		     INTEGER, &snap },
		{ "systraypinning",          INTEGER, &systraypinning },
		{ "systrayonleft",           INTEGER, &systrayonleft },
		{ "systrayspacing",          INTEGER, &systrayspacing },
		{ "systraypinningfailfirst", INTEGER, &systraypinningfailfirst },
		{ "showsystray",             INTEGER, &showsystray },
		{ "vertpad",                 INTEGER, &vertpad },
		{ "sidepad",                 INTEGER, &sidepad },
		{ "showbar",          	     INTEGER, &showbar },
		{ "topbar",          	     INTEGER, &topbar },
		{ "nmaster",          	     INTEGER, &nmaster },
		{ "resizehints",       	     INTEGER, &resizehints },
		{ "mfact",      	 	     FLOAT,   &mfact },
		{ "lockfullscreen",          INTEGER, &lockfullscreen },
		{ "refreshrate",             INTEGER, &refreshrate },
		{ "barheight",               INTEGER, &barheight },
};

static const Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_d,      spawn,          {.v = dmenucmd } },
	{ MODKEY,                       XK_s,      spawn,          {.v = termcmd } },
	{ MODKEY,                       XK_a,      spawn,          {.v = browsercmd } },
	{ MODKEY|ShiftMask,             XK_s,      togglescratch,  {.ui = 0 } },
	{ MODKEY,                       XK_b,      togglescratch,  {.ui = 1 } },
	{ MODKEY,                       XK_r,      togglescratch,  {.ui = 2 } },
	{ MODKEY,                       XK_w,      togglescratch,  {.ui = 3 } },
	{ MODKEY,                       XK_v,      togglescratch,  {.ui = 4 } },
	{ MODKEY|ShiftMask,             XK_b,      togglebar,      {0} },
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_o,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY,                       XK_space,  zoom,           {0} },
	{ MODKEY,                       XK_f,      togglefullscr,  {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY,                       XK_q,      killclient,     {0} },
	{ MODKEY|ShiftMask,             XK_space,  setlayout,      {0} },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },

	{ 0,                            XF86XK_AudioMute,         spawn, SHCMD("volume 0") },
	{ 0,                            XF86XK_AudioLowerVolume,  spawn, SHCMD("volume 1 5") },
	{ 0,                            XF86XK_AudioRaiseVolume,  spawn, SHCMD("volume 2 5") },
	{ ShiftMask,                    XF86XK_AudioLowerVolume,  spawn, SHCMD("volume 1 2") },
	{ ShiftMask,                    XF86XK_AudioRaiseVolume,  spawn, SHCMD("volume 2 2") },
	{ 0,                            XF86XK_MonBrightnessDown, spawn, SHCMD("brightness 1") },
	{ 0,                            XF86XK_MonBrightnessUp,   spawn, SHCMD("brightness 2") },

	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	{ MODKEY|ShiftMask,             XK_q,      quit,           {0} },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button1,        sigstatusbar,   {.i = 1} },
	{ ClkStatusText,        0,              Button2,        sigstatusbar,   {.i = 2} },
	{ ClkStatusText,        0,              Button3,        sigstatusbar,   {.i = 3} },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

