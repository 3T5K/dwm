/*       ____        __         
 *      / __ \__  __/ /__  _____
 *     / /_/ / / / / / _ \/ ___/
 *    / _, _/ /_/ / /  __(__  ) 
 *   /_/ |_|\__,_/_/\___/____/  
 */

/* xprop(1):
 *	WM_CLASS(STRING) = instance, class
 *	WM_NAME(STRING) = title
 */

static const Rule rules[] = {
	/* class     instance  title           tags mask  isfloating  isfullscreen  isterminal  noswallow  monitor */
	{ "Gimp",    NULL,     NULL,           0,         1,          0,            0,           0,        -1 },
	{ "Firefox", NULL,     NULL,           1 << 8,    0,          0,            0,          -1,        -1 },
	{ "St",      NULL,     NULL,           0,         0,          0,            1,           0,        -1 },
	{ NULL,      NULL,     "Event Tester", 0,         0,          0,            0,           1,        -1 }, /* xev */
};
