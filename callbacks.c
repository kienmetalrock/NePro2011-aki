#include "support.h"
 
G_MODULE_EXPORT gboolean
cb_expose_chart( GtkWidget      *widget,
                 GdkEventExpose *event,
                 ChData         *data )
{
    cairo_t *cr;
 
    /* Create cairo context from GdkWindow */
    cr = gdk_cairo_create( event->window );
 
    /* Paint whole area in green color */
    cairo_set_source_rgb( cr, 0, 1, 0 );
    cairo_paint( cr );
 
    /* Destroy cairo context */
    cairo_destroy( cr );
 
    /* Return TRUE, since we handled this event */
    return( TRUE );
}
