
#ifndef __SUPPORT_H__
#define __SUPPORT_H__

#include <gtk/gtk.h>

/* Convenience macros for obtaining objects from UI file */
#define CH_GET_OBJECT( builder, name, type, data ) \
	data->name = type( gtk_builder_get_object( builder, #name ) )
#define CH_GET_WIDGET( builder, name, data ) \
	CH_GET_OBJECT( builder, name, GTK_WIDGET, data )

/* Main data structure definition */
typedef struct _ChData ChData;
struct _ChData
{
	/* Widgets */
		/* Window */
	GtkWidget *menu_window,     /* Main window */  
						*host_window,			/* Host window */
						*join_window,			/* Join window */
						*ready_window,		/* Ready window */
						*playing_window;	/* Playing window */
		/* Button */
	GtkWidget	*menu_host,				/* host button on menu window */
						*menu_join,				/* join button on menu window */
						*host_cancel,			/* cancel button on host window */
						*join_join,				/* join button on join window */
						*join_cancel,				/* back button on join window */
						*ready_start,			/* start button on ready window */	
						*ready_cancel,			/* quit button on ready window */
						*playing_quit;     /* quit button on playing window */
};

#endif /* __SUPPORT_H__ */
