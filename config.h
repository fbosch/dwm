/* See LICENSE file for copyright and license details. */
#include "selfrestart.c"
#include <X11/XF86keysym.h>

#define ICONSIZE 22   /* icon size */
#define ICONSPACING 15 /* space between icon and title */
#define STATUSBAR "dwmblocks"

/* appearance */
static const unsigned int panel[] = {20, 20, 20, 20}; //positions: 0-top panel, 1-bottom panel, 2-left panel, 3-right panel
static const unsigned int borderpx  = 3;        		 	/* border pixel of windows */
static const unsigned int gappx     = 3;        			/* gaps between windows */
static const unsigned int snap      = 0;       				/* snap pixel */
static const unsigned int systraypinning = 0;   /* 0: sloppy systray follows selected monitor, >1: pin systray to monitor X */
static const unsigned int systrayonleft = 0;   	/* 0: systray in the right corner, >0: systray on left of status text */
static const unsigned int systrayspacing = 2;   /* systray spacing */
static const int systraypinningfailfirst = 1;   /* 1: if pinning fails, display systray on the first monitor, False: display systray on the last monitor*/
static const int showsystray        = 1;     /* 0 means no systray */
static const int showbar            = 1;        			/* 0 means no bar */
static const int topbar             = 1;        			/* 0 means bottom bar */
static const int vertpad              = 0;       				/* vertical padding of bar */
static const int sidepad            = 0;       				/* horizontal padding of bar */
static const int horizpadbar        = 15;        			/* horizontal padding for statusbar */
static const int vertpadbar         = 15;        			/* vertical padding for statusbar */
static const char *fonts[]          = {  "SF Mono:size=11", "TerminessTTF Nerd Font:size=14", };
static const char dmenufont[]       = "SF Mono:size=12";
static const char col_gray1[]       = "#161616";
static const char col_gray2[]       = "#3B3A43";
static const char col_gray3[]       = "#49494A";
static const char col_gray4[]       = "#909999";
static const char col_cyan[]        = "#292929";
static const char col_white[]				= "#ffffff";
static const char *colors[][3]      = {
	/*               fg         bg         border   */
	[SchemeNorm] = { col_gray4, col_gray1, col_gray2 },
	[SchemeSel]  = { col_white, col_cyan,  col_gray3  },
};

#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* tagging */
static const char *tags[] = { "   " , " ₂  ", " ₃  ", " ₄  ", " ₅  ", " ₆  ", " ₇  ", " "};

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       tags mask     isfloating   monitor */
	{ "Gimp",     NULL,       NULL,       0,            1,           -1 },
	{ "mpv",      NULL,       NULL,       0,            1,           -1 },
	{ "vlc",      NULL,       NULL,       0,            1,           -1 },
	{ "nomacs",   NULL,       NULL,       0,            1,           -1 },
	{ "Nemo", 	  NULL,       NULL,       0,       			1,           -1 },
	{ "Thunar", 	  NULL,       NULL,       0,       			1,           -1 },
	{ "feh", 	  	NULL,       NULL,       0,       			1,           -1 },
	{ "Bitwarden", 	  	NULL,       NULL,       0,       			1,           -1 },
	{ "Lampe", 	  	NULL,       NULL,       0,       			1,           -1 },
	{ "Plexamp", 	  	NULL,       NULL,       0,       			1,           -1 },
	{ "Conky-manager", 	  	NULL,       NULL,       0,       			1,           -1 },
	{ "Steam", 	  	NULL,       NULL,     1 << 4,       			1,           -1 },
	{ "Minecraft Launcher", 	  	NULL,       NULL,     1 << 3,       			1,           0 },
	{ "Minecraft 1.18.2", 	  	NULL,       NULL,     1 << 3,       			0,           0 },
	{ "Minecraft", 	  	NULL,       NULL,     1 << 3,       			0,           0 },
	{ "Sxiv", 	  	NULL,       NULL,       0,       			1,           -1 },
	{ "inkdrop", 	  	NULL,       NULL,       0,       			1,           -1 },
	{ "Sysmontask", 	  	NULL,       NULL,       0,       			1,           -1 },
	{ "plexmediaplayer", 	  	NULL,       NULL,     1 << 6,       			1,           0 },
	{ "Brave",    NULL,       NULL,       1 << 1,       0,           -1 },
	{ "code-oss", NULL,       NULL,       1 << 2,       0,           -1 },
	{ "discord",  NULL,       NULL,       1 << 4,       1,           -1 },
	{ "Signal",   NULL,       NULL,      	1 << 4,       1,           -1 },
	{ "Caprine",   NULL,       NULL,      	1 << 4,       1,           -1 },
	{ "mail",   	NULL,       NULL,      	1 << 4,       0,           -1 },
	{ "kitty",   	NULL,       NULL,      	0, 			      0,           1 },
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 0;    /* 1 means respect size hints in tiled resizals */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "|M|",      centeredmaster }, /* first entry is default */
	{ ">M>",      centeredfloatingmaster },
	{ "[]=",      tile },
	{ "><>",      NULL }, /* no layout function means floating behavior */
	{ "[M]",      monocle },
};

/* key definitions */
#define MODKEY Mod1Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-nb", col_gray1, "-nf", col_white, "-sb", col_gray3, "-sf", col_white, NULL };
static const char *termcmd[]  = { "mullvad-exclude", "kitty", "--single-instance" }; /* run kitty in single instance and exclude it from vpn */
static const char *layoutmenu_cmd = "layoutmenu.sh";

static Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_p,      spawn,          {.v = dmenucmd } },
	{ MODKEY|ShiftMask,             XK_Return, spawn,          {.v = termcmd } },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_d,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY,                       XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY|ShiftMask,             XK_c,      killclient,     {0} },
	{ MODKEY,                       XK_u,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_o,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[3]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[4]} },
	{ MODKEY|ShiftMask,             XK_f,      fullscreen,     {0} },
	{ MODKEY,                       XK_space,  setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
	{ MODKEY|ShiftMask, 						XK_d, 		 spawn, 				SHCMD("pamixer -t; pkill -RTMIN+22 dwmblocks") },
	{ MODKEY|ShiftMask, 						XK_a, 		 spawn, 				SHCMD("pamixer -d 5; pkill -RTMIN+22 dwmblocks") },
	{ MODKEY|ShiftMask, 						XK_s, 		 spawn, 				SHCMD("pamixer -i 5; pkill -RTMIN+22 dwmblocks") },
	{ ControlMask|ShiftMask,				XK_space,	 spawn, 				SHCMD("setxkbmap -query | grep \"us\" && setxkbmap dk || setxkbmap us; pkill -RTMIN+23 dwmblocks") },
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
	{ MODKEY|ShiftMask,             XK_r,      self_restart,   {0} },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	/* { ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} }, */
	{ ClkLtSymbol,          0,              Button3,        layoutmenu,     {0} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	// { ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
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

