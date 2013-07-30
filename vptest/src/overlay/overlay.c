#include <gst/gst.h>
#include <gtk/gtk.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

// depends
// sudo apt-get install libgtk-3-dev
// sudo apt-get install libgstreamer0.10-dev libgstreamer-plugins-base0.10-dev
// make to buildubuntu


#define ARM_BUILD

static gint testBarWidth = 640;
static gint testBarHeight = 480;

//----------------------------------------------------------------------------//

static void clear(gint w, gint h)
{
	Display                 *dpy;
	Window                  root;

	dpy = XOpenDisplay (NULL);
	root = RootWindow (dpy, 0);
	XClearWindow(dpy, root);
	XFlush(dpy);
	XCloseDisplay(dpy);

}

//----------------------------------------------------------------------------//

static gboolean setResolution(gint w, gint h)
{
	Display                 *dpy;
	Window                  root;
	static XRRScreenResources  *res;
	gint i = 0;
	gboolean retval = FALSE;

	dpy = XOpenDisplay (NULL);
	root = RootWindow (dpy, 0);
	res = XRRGetScreenResources (dpy, root);

	for (i = 0; i < res->noutput; i++) {
		XRROutputInfo *output_info = XRRGetOutputInfo(dpy, res,res->outputs[i]);
		XRRCrtcInfo *crtc_info = XRRGetCrtcInfo(dpy, res, output_info->crtc);

		int j = 0;
		for (j = 0; j < output_info->nmode; j++) {

			if (g_strncasecmp(output_info->name,"HDMI",4) == 0 ) {
				if (w == res->modes[j].width && res->modes[j].height == h) {
					XRROutputInfo *output_first_info = XRRGetOutputInfo(dpy, res, res->outputs[i]);
					XRRSetCrtcConfig(dpy, res, output_first_info->crtc, CurrentTime, crtc_info->x, crtc_info->y, res->modes[j].id, RR_Rotate_0, &res->outputs[i], 1);
					// notes setting to first Hz, which will be the fastest
					retval = TRUE;
					XRRFreeOutputInfo(output_first_info);
					break;
				}
			}
		}
		XRRFreeOutputInfo(output_info);
		XRRFreeCrtcInfo(crtc_info);
	}

	XRRFreeScreenResources(res);
	XCloseDisplay(dpy);

	return retval;

}

//----------------------------------------------------------------------------//

static void getResolution(gint *w, gint *h)
{
	Display                 *dpy;
	Window                  root;
	static XRRScreenResources  *res;
	gint i = 0;

	dpy = XOpenDisplay (NULL);
	root = RootWindow (dpy, 0);
	res = XRRGetScreenResources (dpy, root);

	for (i = 0; i < res->noutput; i++) {
		XRROutputInfo *output_info = XRRGetOutputInfo(dpy, res,res->outputs[i]);
		XRRCrtcInfo *crtc_info = XRRGetCrtcInfo(dpy, res, output_info->crtc);

		if (g_strncasecmp(output_info->name,"HDMI",4) == 0 ) {
			*w = (gint)crtc_info->width;
			*h = (gint)crtc_info->height;
		}
		XRRFreeOutputInfo(output_info);
		XRRFreeCrtcInfo(crtc_info);
	}

	XRRFreeScreenResources(res);
	XCloseDisplay(dpy);

}

//----------------------------------------------------------------------------//

static gboolean time_handler(GtkWidget *widget)
{
	gtk_main_quit ();
	return TRUE;

}

//----------------------------------------------------------------------------//

static void createOverlayImageWindow(gint time, gint w, gint h)
{
	GtkWidget *window;
	GtkWidget *image;

	window = gtk_window_new (GTK_WINDOW_POPUP);

	GError *error = NULL;
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file ("test.png", &error);
	if (pixbuf == NULL) {
		g_printerr ("Error loading file: #%d %s\n", error->code, error->message);
		g_error_free (error);
		exit (1);
	}
	image = gtk_image_new_from_pixbuf (pixbuf);
	gint width = gdk_pixbuf_get_width (pixbuf);
	gint height = gdk_pixbuf_get_height (pixbuf);

	gtk_container_add (GTK_CONTAINER (window), image);
	gtk_widget_show (image);

    gtk_window_move (GTK_WINDOW(window), (w-width)/2,(h-height)/2);
	gtk_widget_show (window);

	// timer to exit after time
	g_timeout_add(time, (GSourceFunc) time_handler, (gpointer) window);
}

//----------------------------------------------------------------------------//

static void drawBar(gint x, gint y, gint w, gint h,
		            cairo_t* cr, double red, double green, double blue)
{
	cairo_set_source_rgb(cr,red,green,blue);
	cairo_rectangle(cr, x, y, w, h);
	cairo_fill(cr);
}

//----------------------------------------------------------------------------//

static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
	gint x, y, w, h;
	gint last=0;

	w = testBarWidth/7;
	h = testBarHeight * 0.70;
	x = y = 0;

	drawBar(x,y,w,h,cr,0.8,0.8,0.8); x+=w;
	drawBar(x,y,w,h,cr,1,1,0); x+=w;
	drawBar(x,y,w,h,cr,0,1,1); x+=w;
	drawBar(x,y,w,h,cr,0,1,0); x+=w;
	drawBar(x,y,w,h,cr,1,0,1); x+=w;
	drawBar(x,y,w,h,cr,1,0,0); x+=w;
	drawBar(x,y,w,h,cr,0,0,1); last=x;
	drawBar(x,y,testBarWidth-x,h,cr,0,0,1);

	x = 0;
	y = y + h;
	h = testBarHeight * 0.10;
	drawBar(x,y,w,h,cr,0,0,1); x+=w;
	drawBar(x,y,w,h,cr,0,0,0); x+=w;
	drawBar(x,y,w,h,cr,1,0,1); x+=w;
	drawBar(x,y,w,h,cr,0,0,0); x+=w;
	drawBar(x,y,w,h,cr,0,1,1); x+=w;
	drawBar(x,y,w,h,cr,0,0,0); x+=w;
	drawBar(x,y,testBarWidth-x,h,cr,0.8,0.8,0.8);

	x = 0;
	y = y + h;
	w = testBarWidth/5.5;
	h = testBarHeight - y;
	drawBar(x,y,w,h,cr,0,0,0.8); x+=w;
	drawBar(x,y,w,h,cr,1,1,1); x+=w;
	drawBar(x,y,w,h,cr,0,0,0.8); x+=w;
	drawBar(x,y,testBarWidth-x,h,cr,0,0,0); x+=w;

	w = w * 0.3;
	drawBar(last-w,y,w,h,cr,0.7,0.7,0.7);

	return TRUE;
}

//----------------------------------------------------------------------------//

static void createOverlayBarWindow(gint time, gint w, gint h)
{
	GtkWidget *window;
	GtkWidget *darea;

	window = gtk_window_new (GTK_WINDOW_POPUP);
	gtk_window_set_default_size(GTK_WINDOW(window), testBarWidth,testBarHeight);

	gtk_window_move (GTK_WINDOW(window), (w-testBarWidth)/2,(h-testBarHeight)/2);

	//gtk_window_set_position(GTK_WINDOW(window), GtkWindowPosition(0,0));

	darea = gtk_drawing_area_new();
	gtk_container_add(GTK_CONTAINER(window), darea);

	g_signal_connect(G_OBJECT(darea), "draw",
	      G_CALLBACK(on_draw_event), NULL);

	gtk_widget_show_all(window);

	// timer to exit after time
	g_timeout_add(time, (GSourceFunc) time_handler, (gpointer) window);
}

//----------------------------------------------------------------------------//

int main(int argc, char *argv[]) {

    GstElement *pipeline, *source, *sink, *filter, *colorspace;
    GstStateChangeReturn ret;
    GstCaps *filtercaps;

    gint w = 1280;
    gint h = 720;
    gint time = 5000;
    gfloat p = 0.6;
    gint pattern = 18;

	#ifdef ARM_BUILD
    	getResolution(&w,&h);
	#endif

    if (argc>1) {
    	p = (gfloat)(atof(argv[1]));
    }

    if (argc>2) {
    	time = (gint)(atoi(argv[2])) * 1000;
    }

    if (argc>3) {
		pattern = (gfloat)(atof(argv[3]));
	}

    testBarWidth = w*p;
    testBarHeight = h*p;

    // Initialize GStreamer
    gtk_init (0,0);
    gst_init (0,0);

    // Create the elements
    source = gst_element_factory_make ("videotestsrc", "source");
    filter = gst_element_factory_make("capsfilter", "filter");

    #ifdef ARM_BUILD
       sink = gst_element_factory_make ("nv_omx_hdmi_videosink", "sink");
       colorspace = gst_element_factory_make ("nvvidconv",  "Colorspace");
       filtercaps = gst_caps_new_simple ("video/x-raw-yuv",
               "format", GST_TYPE_FOURCC, GST_MAKE_FOURCC ('I', '4', '2', '0'),
               "width", G_TYPE_INT, w,
               "height", G_TYPE_INT, h,
               "framerate", GST_TYPE_FRACTION, 30, 1,
               NULL);
    #else
        sink = gst_element_factory_make ("autovideosink", "sink");
        colorspace = gst_element_factory_make ("ffmpegcolorspace",  "Colorspace");
        filtercaps = gst_caps_new_simple ("video/x-raw-yuv",
                "format", GST_TYPE_FOURCC, GST_MAKE_FOURCC ('Y', 'U', 'Y', '2'),
                "width", G_TYPE_INT, w,
                "height", G_TYPE_INT, h,
                "framerate", GST_TYPE_FRACTION, 30, 1,
                NULL);
    #endif

    g_object_set(G_OBJECT(filter), "caps", filtercaps, NULL);
    gst_caps_unref(filtercaps);

    // Create the empty pipeline
    pipeline = gst_pipeline_new ("test-pipeline");

    if (!pipeline || !source || !sink) {
        g_printerr ("Not all elements could be created.\n");
        return -1;
    }

    // Build the pipeline
    gst_bin_add_many (GST_BIN (pipeline), source, filter, colorspace, sink, NULL);
    if (gst_element_link_many(source, filter, sink, NULL) != TRUE) {
        g_printerr ("Elements could not be linked.\n");
        gst_object_unref (pipeline);
        return -1;
    }

    // Modify the source's properties
    g_object_set (source, "pattern", pattern, NULL);

    // overlay window
    //createOverlayImageWindow(time,w,h);
    createOverlayBarWindow(time,w,h);

    // Start playing
    ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr ("Unable to set the pipeline to the playing state.\n");
        gst_object_unref (pipeline);
        return -1;
    }

    // main loop
    gtk_main();

    // Free resources
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref (pipeline);

    return 0;
}

//----------------------------------------------------------------------------//
