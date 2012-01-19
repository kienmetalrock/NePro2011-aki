/*
 * Compile me with:
 *   gcc -o test test.c $(pkg-config --cflags --libs gtk+-2.0 gmodule-2.0)
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <gtk/gtk.h>
#include "support.h"

#define UI_FILE "main.glade"

int board[5][5];
void gen_board(){
    int i, j, k;
    int status[26];

    status[0] = 0;
    k = 1;
    for (i=0; i<5; i++) {
        for (j=0; j<5; j++) {
            board[i][j] = 0;
            status[k] = 0;
            k++;
        }
    }

    srand(time(NULL));
    for (i=0; i<5; i++) {
        for (j=0; j<5; j++) {
            k = rand()%25 + 1;
            while (status[k] == 1) k = rand()%25 + 1;
            board[i][j] = k;
            status[k] = 1;
        }
    }
    for (i=0; i<5; i++) {
        for (j=0; j<5; j++) {
            fprintf(stdout, "%2d ", board[i][j]);
        }
        fprintf(stdout, "\n");
    }
}

   int
main( int    argc,
      char **argv )
{
		ChData *data;	
    GtkBuilder *builder;
		GError     *error = NULL;

		GtkWidget *host_b[5][5],
							*ready_b[5][5],
							*ready2_b[5][5],
							*playing_b[5][5];

		char num[20];

		int i,j;
		gen_board();

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

				GW (host_board);
				GW (ready_board);
				GW (ready2_board);
				GW (playing_board);

			
#undef GW
		for (i=0;i<5;i++){
				for (j=0;j<5;j++){
						sprintf(num,"%d",board[i][j]);
						host_b[i][j] = gtk_button_new_with_label(num);
						ready_b[i][j]= gtk_button_new_with_label(num);
						ready2_b[i][j]= gtk_button_new_with_label(num);
						playing_b[i][j] = gtk_button_new_with_label(num);
						gtk_table_attach_defaults (GTK_TABLE(data->host_board), host_b[i][j], j, j+1, i, i+1);
						gtk_table_attach_defaults (GTK_TABLE(data->ready_board), ready_b[i][j], j, j+1, i, i+1);
						gtk_table_attach_defaults (GTK_TABLE(data->ready2_board), ready2_b[i][j], j, j+1, i, i+1);
						gtk_table_attach_defaults (GTK_TABLE(data->playing_board), playing_b[i][j], j, j+1, i, i+1);
						gtk_widget_show (host_b[i][j]);
						gtk_widget_show (ready_b[i][j]);
						gtk_widget_show (ready2_b[i][j]);
						gtk_widget_show (playing_b[i][j]);
					}
				}

	
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
