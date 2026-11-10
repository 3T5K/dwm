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

static const Rule rules[] =
{
	{
        .class        = "Alacritty",
        .instance     = NULL,
        .title        = NULL,
        .tags         = 0,
        .isfloating   = 0,
        .isfullscreen = 0,
        .isterminal   = 1,
        .noswallow    = 0,
        .monitor      = -1
    },
};
