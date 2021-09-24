#include <gst/gst.h>

static gboolean
my_bus_func(GstBus *bus, GstMessage *message, gpointer user_data)
{
    GstDevice *device;
    gchar *name;

    switch (GST_MESSAGE_TYPE(message))
    {
    case GST_MESSAGE_DEVICE_ADDED:
        gst_message_parse_device_added(message, &device);
        name = gst_device_get_display_name(device);
        g_print("Device added: %s\n", name);
        g_free(name);
        gst_object_unref(device);
        break;
    case GST_MESSAGE_DEVICE_REMOVED:
        gst_message_parse_device_removed(message, &device);
        name = gst_device_get_display_name(device);
        g_print("Device removed: %s\n", name);
        g_free(name);
        gst_object_unref(device);
        break;
    default:
        break;
    }
    return G_SOURCE_CONTINUE;
}

GstDeviceMonitor *
setup_raw_video_source_device_monitor(void)
{
    GstDeviceMonitor *monitor;
    GstBus *bus;
    GstCaps *caps;

    monitor = gst_device_monitor_new();

    bus = gst_device_monitor_get_bus(monitor);
    gst_bus_add_watch(bus, my_bus_func, NULL);
    gst_object_unref(bus);

    caps = gst_caps_new_empty_simple("video/x-raw");
    gst_device_monitor_add_filter(monitor, "Video/Source", caps);
    gst_caps_unref(caps);

    gst_device_monitor_start(monitor);

    return monitor;
}

int main(int argc, char *argv[])
{
    gst_init(&argc, &argv);

    GstDeviceMonitor *monitor = setup_raw_video_source_device_monitor();

    GMainLoop *loop = g_main_loop_new(NULL, FALSE);

    g_main_loop_run(loop);

    gst_device_monitor_stop(monitor);
    gst_object_unref(monitor);
    return 0;
}