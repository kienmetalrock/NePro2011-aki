/*
 * Compile me with:
 *   gcc -o test test.c $(pkg-config --cflags --libs gtk+-2.0 gmodule-2.0)
 */
 
#include <gtk/gtk.h>
 
int
main( int    argc,
      char **argv )
{
    GtkBuilder *builder;
    GtkWidget  *main_window,*host_window, *playing, *ready, *hostbutt;
    GError     *error = NULL;
 
    /* Init GTK+ */
    gtk_init( &argc, &argv );
 
    /* Create new GtkBuilder object */
    builder = gtk_builder_new();
    /* Load UI from file. If error occurs, report it and quit application.
     * Replace "tut.glade" with your saved project. */
    if( ! gtk_builder_add_from_file( builder, "main.glade", &error ) )
    {
        g_warning( "%s", error->message );
        g_free( error );
        return( 1 );
    } 
    /* Get main window pointer from UI */
    main_window = GTK_WIDGET( gtk_builder_get_object( builder, "main_window" ) );
    host_window = GTK_WIDGET( gtk_builder_get_object( builder, "host_window" ) );
		playing = GTK_WIDGET (gtk_builder_get_object (builder, "playing"));   	
		
		
		hostbutt  = GTK_WIDGET(gtk_builder_get_object(builder, "hostgame"));
		g_signal_connect_swapped (G_OBJECT (hostbutt), "clicked",
			      G_CALLBACK (gtk_widget_show),
			      G_OBJECT (host_window));

    /* Connect signals */
    gtk_builder_connect_signals( builder, NULL );
 
    /* Destroy builder, since we don't need it anymore */
    g_object_unref( G_OBJECT( builder ) );
 
    /* Show window. All other widgets are automatically shown by GtkBuilder */
    gtk_widget_show( main_window);
 
    /* Start main loop */
    gtk_main();
 
    return( 0 );
}
