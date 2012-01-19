/*
 * Compile me with:
 *   gcc -o test test.c $(pkg-config --cflags --libs gtk+-2.0 gmodule-2.0)
 */
 
#include <gtk/gtk.h>
#include "support.h"

#define UI_FILE "main.glade"

int
main( int    argc,
      char **argv )
{
		ChData *data;	
    GtkBuilder *builder;
		GError     *error = NULL;
 
    /* Init GTK+ */
    gtk_init( &argc, &argv );
 
    /* Create new GtkBuilder object */
    builder = gtk_builder_new();
    /* Load UI from file. If error occurs, report it and quit application.*/

    if( ! gtk_builder_add_from_file( builder, UI_FILE, &error ) )
    {
        g_warning( "%s", error->message );
        g_free( error );
        return( 1 );
    }

/* Allocate data structure */
		 data = g_slice_new( ChData );


/* Get objects from UI */
#define GW( name ) CH_GET_WIDGET( builder, name, data )
		    GW (menu_window);
				GW (host_window);
				GW (join_window);
				GW (ready_window);
				GW (playing_window);

				GW (menu_host);
				GW (menu_join);
				GW (host_cancel);
				GW (join_join);
				GW (join_cancel);
				GW (ready_cancel);
				GW (ready_start);
				GW (playing_quit);
#undef GW

	 	/* Connect signals */
    gtk_builder_connect_signals( builder, data );
 		
/*callbacks for buttons*/
			//Callback for menu_host button
		g_signal_connect_swapped(	G_OBJECT (data->menu_host), "clicked",
			      									G_CALLBACK (gtk_widget_show),
			      									G_OBJECT (data->host_window));

		g_signal_connect_swapped(	G_OBJECT (data->menu_host), "clicked",
				            					G_CALLBACK (gtk_widget_hide),
										       		G_OBJECT(data->menu_window));
		

			//Callback for menu_join button
		g_signal_connect_swapped(	G_OBJECT (data->menu_join), "clicked",
															G_CALLBACK(gtk_widget_show),
															G_OBJECT (data->join_window));
   	g_signal_connect_swapped( G_OBJECT (data->menu_join), "clicked",
			                        G_CALLBACK (gtk_widget_hide),
															G_OBJECT(data->menu_window));

			//Callback for host_cancel button
		g_signal_connect_swapped( G_OBJECT (data->host_cancel), "clicked",
						                  G_CALLBACK(gtk_widget_show),
															G_OBJECT (data->menu_window));
		g_signal_connect_swapped( G_OBJECT (data->host_cancel), "clicked",
								              G_CALLBACK(gtk_widget_hide),
															G_OBJECT (data->host_window));



			//Callback for join_cancel button
		g_signal_connect_swapped(	G_OBJECT (data->join_cancel), "clicked",
				                     	G_CALLBACK(gtk_widget_show),
															G_OBJECT (data->menu_window));
		g_signal_connect_swapped( G_OBJECT (data->join_cancel), "clicked",
			                        G_CALLBACK(gtk_widget_hide),
															G_OBJECT (data->join_window));
			
			//Callback for join_join  button
		g_signal_connect_swapped( G_OBJECT (data->join_join), "clicked",
						                  G_CALLBACK(gtk_widget_show),
															G_OBJECT (data->ready_window));
		g_signal_connect_swapped( G_OBJECT (data->join_join), "clicked",
								              G_CALLBACK(gtk_widget_hide),
															G_OBJECT (data->join_window));


			//Callback for ready_cancel  button
		g_signal_connect_swapped( G_OBJECT (data->ready_cancel), "clicked",
						                  G_CALLBACK(gtk_widget_show),
															G_OBJECT (data->menu_window));
	 	g_signal_connect_swapped( G_OBJECT (data->ready_cancel), "clicked",
								              G_CALLBACK(gtk_widget_hide),
															G_OBJECT (data->ready_window));
		

		//Callback for ready_start  button
		g_signal_connect_swapped( G_OBJECT (data->ready_start), "clicked",
						                  G_CALLBACK(gtk_widget_show),
															G_OBJECT (data->playing_window));
		g_signal_connect_swapped( G_OBJECT (data->ready_start), "clicked",
								              G_CALLBACK(gtk_widget_hide),
															G_OBJECT (data->ready_window));


		//Callback for playing_quit  button
		g_signal_connect_swapped( G_OBJECT (data->playing_quit), "clicked",
						                  G_CALLBACK(gtk_widget_show),
															G_OBJECT (data->menu_window));
		g_signal_connect_swapped( G_OBJECT (data->playing_quit), "clicked",
								              G_CALLBACK(gtk_widget_hide),
															G_OBJECT (data->playing_window));


		/* Destroy builder, since we don't need it anymore */
    g_object_unref( G_OBJECT( builder ) );
 
    /* Show window. All other widgets are automatically shown by GtkBuilder */
    gtk_widget_show(data->menu_window);
 
    /* Start main loop */
    gtk_main();
 
    return( 0 );
}
