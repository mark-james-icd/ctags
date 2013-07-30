
//------------------------------------------------------------------------------------------//
// DBus test app
//
// Requires dbus glib
// On board just had todo : sudo apt-get -y install libdbus-glib-1-dev
// sudo apt-get install dbus-*dev
// sudo apt-get -y install dbus libdbus-1-dev libdbus-glib-1-2 libdbus-glib-1-dev
//
// Commandline build
// gcc -o main main.c `pkg-config --cflags --libs dbus-1 dbus-glib-1 gio-2.0'
//
//
// Reference urls
//
// https://developer.gnome.org/gio/2.29/GDBusConnection.html
// https://developer.gnome.org/gio/2.35/ch32s02.html
// http://blog.damienradtke.org/dbus-daemon/
// http://dbus.freedesktop.org/doc/dbus-send.1.html
// http://www.dsource.org/projects/dbus-d/wiki/Introspection
//
// To test run the app then in a console enter
// dbus-send --print-reply --type=method_call --dest=org.guvcview.camera /org/guvcview/camera org.guvcview.camera.ExampleMessage string:'a string' int32:1
// dbus-send --print-reply --type=method_call --dest=org.guvcview.camera /org/guvcview/camera org.guvcview.camera.EnterTestMode
// dbus-send --print-reply --type=method_call --dest=org.guvcview.camera /org/guvcview/camera org.guvcview.camera.ExitTestMode
// dbus-send --print-reply --type=method_call --dest=org.guvcview.camera /org/guvcview/camera org.guvcview.camera.DisplayTestPattern  int32:1
// dbus-send --print-reply --type=method_call --dest=org.guvcview.camera /org/guvcview/camera org.guvcview.camera.VpCallout
// dbus-send --print-reply --type=method_call --dest=org.guvcview.camera /org/guvcview/camera org.guvcview.camera.VpCallin
//------------------------------------------------------------------------------------------//

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gio/gio.h>

static GMainLoop *loop;
static GDBusNodeInfo *introspection_data = NULL;
static const char* dbus_name_dot = "org.guvcview.camera";
static const char* dbus_name_slash ="/org/guvcview/camera";

//------------------------------------------------------------------------------------------//

static const gchar introspection_xml[] =
    "<node>"
    "  <interface name='org.guvcview.camera'>"
    "    <method name='ExampleMessage'>"
    "      <arg type='s' name='message' direction='in'/>"
    "      <arg type='i' name='value' direction='in'/>"
    "      <arg type='s' name='response' direction='out'/>"
    "    </method>"               
    "    <method name='EnterTestMode'>"
    "      <arg type='i' name='response' direction='out'/>"
    "    </method>"
    "    <method name='ExitTestMode'>"
    "      <arg type='i' name='response' direction='out'/>"
    "    </method>"
    "    <method name='DisplayTestPattern'>"
    "      <arg type='i' name='pattern' direction='in'/>"
    "      <arg type='i' name='response' direction='out'/>"
    "    </method>"
    "    <method name='VpCallout'>"
    "      <arg type='i' name='response' direction='out'/>"
    "    </method>"
    "    <method name='VpCallin'>"
    "      <arg type='i' name='response' direction='out'/>"
    "    </method>"
    "  </interface>"
    "</node>";

//------------------------------------------------------------------------------------------//
// Handle method calls
static void handle_method_call(GDBusConnection *conn,
                               const gchar *sender,
                               const gchar *object_path,
                               const gchar *interface_name,
                               const gchar *method_name,
                               GVariant *parameters,
                               GDBusMethodInvocation *invocation,
                               gpointer user_data)
{
    (void)conn;
    (void)sender;
    (void)object_path;
    (void)interface_name;
    (void)user_data;

    if (!g_strcmp0(method_name, "ExampleMessage")) {
        gchar *message;
        int value;
        gchar *response;
        g_variant_get(parameters, "(si)", &message, &value);
        response = g_strdup_printf("Received message: %s %d", message, value);
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(s)", response));
        g_free(message);
        g_free(response);
    }
    else if (!g_strcmp0(method_name, "EnterTestMode")) {
        // Enter a restrictive test mode to prevent normal user operation while the test driver is active.
        // By default “self view” is displayed without UI.
        //
        // todo
        //
        // return 1 for success
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(i)", 1));
    }
    else if (!g_strcmp0(method_name, "ExitTestMode")) {
        // Exit restrictive test mode to normal normal user operation on test driver exit.
        //
        // todo
        //
        // return 1 for success
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(i)", 1));
    }
    else if (!g_strcmp0(method_name, "DisplayTestPattern")) {
        // Changes input from camera to a saved pattern.
        int pattern;
        g_variant_get(parameters, "(i)", &pattern);
        //
        // todo
        //
        // return 1 for success
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(i)", 1));
    }
    else if (!g_strcmp0(method_name, "VpCallout")) {
        // Implements call out scenario.
        //
        // todo
        //
        // return 1 for success
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(i)", 1));
    }
    else if (!g_strcmp0(method_name, "VpCallin")) {
        // Implements call in scenario.
        //
        // todo
        //
        // return 1 for success
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(i)", 1));
    }
}

//------------------------------------------------------------------------------------------//
// Handle property queries
static GVariant *handle_get_property(GDBusConnection *conn,
                                     const gchar *sender,
                                     const gchar *object_path,
                                     const gchar *interface_name,
                                     const gchar *property_name,
                                     GError **error,
                                     gpointer user_data)
{
    (void)conn;
    (void)sender;
    (void)object_path;
    (void)interface_name;
    (void)property_name;
    (void)error;
    (void)user_data;

    return NULL;
}

//------------------------------------------------------------------------------------------//
// Handle set property
static gboolean handle_set_property(GDBusConnection *conn,
                                    const gchar *sender,
                                    const gchar *object_path,
                                    const gchar *interface_name,
                                    const gchar *property_name,
                                    GVariant *value,
                                    GError **error,
                                    gpointer user_data)
{
    (void)conn;
    (void)sender;
    (void)object_path;
    (void)interface_name;
    (void)property_name;
    (void)value;
    (void)error;
    (void)user_data;

    return FALSE;
}

//------------------------------------------------------------------------------------------//
// Function vtable for handling methods and properties
static const GDBusInterfaceVTable interface_vtable = {
    &handle_method_call,
    &handle_get_property,
    &handle_set_property
};

//------------------------------------------------------------------------------------------//
// On SIGINT, exit the main loop
static void sig_handler(int signo)
{
    if (signo == SIGINT) {
        g_main_loop_quit(loop);
    }
}

//------------------------------------------------------------------------------------------//
//
static void on_bus_acquired (GDBusConnection *connection, const gchar *name, gpointer user_data)
{
    (void)name;
    (void)user_data;

    guint registration_id;
    registration_id = g_dbus_connection_register_object (connection,
                                                       dbus_name_slash,
                                                       introspection_data->interfaces[0],
                                                       &interface_vtable,
                                                       NULL,
                                                       NULL,
                                                       NULL);
    g_assert (registration_id > 0);
}

//------------------------------------------------------------------------------------------//
static void on_name_acquired (GDBusConnection *connection, const gchar *name, gpointer user_data)
{
    (void)connection;
    (void)name;
    (void)user_data;
}

//------------------------------------------------------------------------------------------//
static void on_name_lost (GDBusConnection *connection, const gchar *name, gpointer user_data)
{
    (void)connection;
    (void)name;
    (void)user_data;

    //g_main_loop_quit(loop);
}

//------------------------------------------------------------------------------------------//
int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    printf("Camera Bus test started\n");
    printf("PID %d\n", getpid());

    // this is just for this test app so we can make it exit neatly
    // set up the SIGINT signal handler
    // kill -s SIGINT <pid>
    if (signal(SIGINT, &sig_handler) == SIG_ERR) {        
        exit(EXIT_FAILURE);
    }

    guint owner_id;    
    g_type_init ();

    // build the introspection data structures from xml
    introspection_data = g_dbus_node_info_new_for_xml (introspection_xml, NULL);
    g_assert (introspection_data != NULL);

    // get a unique nam on the bus
    owner_id = g_bus_own_name (G_BUS_TYPE_SESSION,
                               dbus_name_dot,
                               G_BUS_NAME_OWNER_FLAGS_NONE,
                               on_bus_acquired,
                               on_name_acquired,
                               on_name_lost,
                               NULL,
                               NULL);

    // omitted rror handling

    // main loop
    loop = g_main_loop_new (NULL, FALSE);
    g_main_loop_run (loop);

    // clean up
    g_bus_unown_name (owner_id);
    g_dbus_node_info_unref (introspection_data);

    printf("exiting..\n");
    return 0;
}





