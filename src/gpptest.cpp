#include <gst/gst.h>

#include <iostream>
#include <thread>
#include <string>
#include <ctime>

#include "v4l2IOctrl.h"
#include "timestamp.h"
#include "log.h"

using std::ref;
using std::string;
using std::thread;

using namespace gpptest;

static timestamp CurrentTime;
static log log_info;

// typedef struct _deviceinfo
// {
//     char*m_devname;
//     int m_framewidth;
//     int m_frameheight;
//     int m_framerate;
//     unsigned int frame_bps;
//                      brightness 0x00980900 (int)    : min=-64 max=64 step=1 default=0 value=0
//                        contrast 0x00980901 (int)    : min=0 max=64 step=1 default=32 value=32
//                      saturation 0x00980902 (int)    : min=0 max=128 step=1 default=64 value=64
//                             hue 0x00980903 (int)    : min=-40 max=40 step=1 default=0 value=0
//  white_balance_temperature_auto 0x0098090c (bool)   : default=1 value=1
//                           gamma 0x00980910 (int)    : min=72 max=500 step=1 default=100 value=100
//                            gain 0x00980913 (int)    : min=0 max=100 step=1 default=0 value=0
//            power_line_frequency 0x00980918 (menu)   : min=0 max=2 default=1 value=1
//       white_balance_temperature 0x0098091a (int)    : min=2800 max=6500 step=1 default=4600 value=4600 flags=inactive
//                       sharpness 0x0098091b (int)    : min=0 max=6 step=1 default=2 value=2
//          backlight_compensation 0x0098091c (int)    : min=0 max=2 step=1 default=1 value=1
//                   exposure_auto 0x009a0901 (menu)   : min=0 max=3 default=3 value=3
//               exposure_absolute 0x009a0902 (int)    : min=1 max=5000 step=1 default=157 value=157 flags=inactive
//          exposure_auto_priority 0x009a0903 (bool)   : default=0 value=1
// }dev;

typedef struct _GstDataStruct
{
    string devname;
    V4l2DeviceList testlist;

    GstElement *pipeline;
    GstElement *srcbin;
    GstElement *v4l2src;
    GstElement *errsrc;
    GstElement *capsfilter; 
    // GstElement *queue;
    // GstElement *h264parse;
    // GstElement *videoscale;
    // GstElement *videoconverter;
    // GstElement *videorate;
    GstElement *videosink;
    GstBus *bus;
    GstMessage *msg;
    guint bus_watch_id;
    guint sourceid;  /* To control the GSource */
    GMainLoop *loop; /* GLib's Main Loop */
} GstDataStruct;

static GstDataStruct GstData;

static gboolean
timeout_handler(gpointer data)
{

    GstDataStruct *gdata = (GstDataStruct *)data;
    gdata->testlist.updateDeviceList();

    CurrentTime.time_update();
    if (gdata->testlist.isListEmpty())
    {
        gst_bus_post(gdata->bus,
                     gst_message_new_application(GST_OBJECT(gdata->v4l2src),
                                                 gst_structure_new_empty("device_lost_connection")));
        // printf("DEVICE_LOST_CONNECTION\r");
        printf("%s\tDEVICE_LOST_CONNECTION\r", CurrentTime.timestamp_to_string().c_str());
        fflush(stdout);
        //send message "DEVICE_LOST_CONNECTION"
    }
    else
    {
        gst_bus_post(gdata->bus,
                     gst_message_new_application(GST_OBJECT(gdata->v4l2src),
                                                 gst_structure_new_empty("device_in_connection")));
        // printf("DEVICE_IN_CONNECTION\r");
        printf("%s\tDEVICE_IN_CONNECTION\r", CurrentTime.timestamp_to_string().c_str());
        fflush(stdout);
        //send message "DEVICE_IN_CONNECTION"
    }
    return true;
}

static gboolean
bus_call(GstBus *bus, GstMessage *msg, gpointer data)
{
    GstDataStruct *gdata = (GstDataStruct *)data;

    CurrentTime.time_update(); //This is the log time

    // if (log_info.isthisfile(CurrentTime.date_to_string))
    // {
    //     if (!log_info.isopen())
    //     {
    //         log_info.open(CurrentTime.date_to_string);
    //     }
    // }
    // else
    // {
    //     log_info.close();
    //     log_info.open(CurrentTime.date_to_string);
    // }

    // g_print("\n%" GST_TIME_FORMAT ":Message from %s \t --%s message\n", GST_TIME_ARGS(GST_MESSAGE_TIMESTAMP(msg)), GST_MESSAGE_SRC_NAME(msg), GST_MESSAGE_TYPE_NAME(msg));

    switch (GST_MESSAGE_TYPE(msg))
    {
    case GST_MESSAGE_ERROR:

        GError *err;
        gchar *debug;

        gst_message_parse_error(msg, &err, &debug);
        g_printerr("\nError received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
        g_printerr("Debugging information: %s\n", debug ? debug : "none");

        g_error_free(err);
        g_free(debug);
        if (GST_MESSAGE_SRC(msg) == (GstObject *)(gdata->v4l2src))
        {
            g_print("Dealing with message-\"message::error\"\n");
            g_print("Turn to lost mode...\n");

            gst_element_set_state(gdata->pipeline, GST_STATE_NULL);

            gst_element_set_state(gdata->errsrc, GST_STATE_NULL);

            gst_bin_remove(GST_BIN(gdata->srcbin), GST_ELEMENT(gst_object_ref(gdata->v4l2src)));

            gst_bin_add(GST_BIN(gdata->srcbin), gdata->errsrc);

            gst_element_link_many(gdata->errsrc, gdata->capsfilter, NULL);

            gst_element_set_state(gdata->pipeline, GST_STATE_PLAYING);
        }
        else
        {
            g_print("Unexpected error happened!Quit!\n");
            g_main_loop_quit(gdata->loop);
        }
        break;
    case GST_MESSAGE_NEW_CLOCK:
        g_print("Clock provider:%s\n", GST_OBJECT_NAME(msg->src));
        g_print("Time is now:%" GST_TIME_FORMAT "\n", GST_TIME_ARGS(gst_clock_get_time(gdata->pipeline->clock)));
        break;
    case GST_MESSAGE_STATE_CHANGED:
        /* We are only interested in state-changed messages from the pipeline */
        // if (GST_MESSAGE_SRC(msg))
        // {
        //     GstState old_state, new_state, pending_state;
        //     gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
        //     g_print("%s changed from %s to %s with pending state %s\n", GST_MESSAGE_SRC_NAME(msg),
        //             gst_element_state_get_name(old_state), gst_element_state_get_name(new_state), gst_element_state_get_name(pending_state));
        // }
        break;
    case GST_MESSAGE_EOS:
        /* end-of-stream */
        // g_main_loop_quit(gdata->loop);
        break;
    case GST_MESSAGE_APPLICATION:
        if (gst_message_has_name(msg, "device_lost_connection"))
        {
            if (gst_bin_get_by_name(GST_BIN(gdata->srcbin), "v4l2src"))
            {
                g_print("\nDealing with message-\"device lost connection\"\n");
                g_print("Turn to lost mode...\n");

                gst_element_set_state(gdata->pipeline, GST_STATE_NULL);

                gst_element_set_state(gdata->errsrc, GST_STATE_NULL);

                gst_bin_remove(GST_BIN(gdata->srcbin), GST_ELEMENT(gst_object_ref(gdata->v4l2src)));

                gst_bin_add(GST_BIN(gdata->srcbin), gdata->errsrc);

                gst_element_link_many(gdata->errsrc, gdata->capsfilter, NULL);

                gst_element_set_state(gdata->pipeline, GST_STATE_PLAYING);

                // GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(GST_BIN(gdata->pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline_lost");
            }
        }
        else if (gst_message_has_name(msg, "device_in_connection"))
        {
            if (gst_bin_get_by_name(GST_BIN(gdata->srcbin), "errsrc"))
            {
                g_print("\nDealing with message-\"device in connection\"\n");
                g_print("Start streaming\n");

                gst_element_set_state(gdata->pipeline, GST_STATE_NULL);

                gst_element_set_state(gdata->v4l2src, GST_STATE_NULL);

                gst_bin_remove(GST_BIN(gdata->srcbin), GST_ELEMENT(gst_object_ref(gdata->errsrc)));

                gst_bin_add(GST_BIN(gdata->srcbin), gdata->v4l2src);

                gst_element_link_many(gdata->v4l2src, gdata->capsfilter, NULL);

                gst_element_set_state(gdata->pipeline, GST_STATE_PLAYING);

                // GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(GST_BIN(gdata->pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline_normal");
            }
        }
        break;
    default:
        /* unhandled message */
        break;
    }
    return true;
}

//for checking thread
// void checking(bool &flag, V4l2DeviceList &dlist)
// {
//     while (true)
//     {
//         dlist.updateDeviceList();
//         flag = !dlist.isListEmpty();
//         sleep(1);
//     }
// }

int main(int argc, char *argv[])
{
    //sprintf(devname, "%s", "/dev/video0");

    // bool workStatus = !testlist.isListEmpty(); //true for working,false for sleeping

    //thread2 checking
    // thread threadchecking(checking, ref(workStatus), ref(testlist));
    // threadchecking.detach();

    /* pipeline diagram file setting*/
    g_setenv("GST_DEBUG_DUMP_DOT_DIR", "./", TRUE);

    gst_init(&argc, &argv);

    GstData.pipeline = gst_pipeline_new("v4l2play");
    GstData.srcbin=gst_bin_new("srcbin");
    GstData.v4l2src = gst_element_factory_make("v4l2src", "v4l2src");
    GstData.errsrc = gst_element_factory_make("videotestsrc", "errsrc");
    GstData.capsfilter=gst_element_factory_make("capsfilter","caps");
    GstCaps *caps = gst_caps_new_simple("video/x-raw",
                                          "width", G_TYPE_INT, 1280,
                                          "height", G_TYPE_INT, 720,
                                          NULL);
    g_object_set(G_OBJECT(GstData.capsfilter),"caps",caps,NULL);
    // GstData.videoconverter=gst_element_factory_make("videoconvert","videoconverter");
    GstData.videosink = gst_element_factory_make("xvimagesink", "videosink");

    gst_bin_add_many(GST_BIN(GstData.srcbin),GstData.v4l2src,GstData.capsfilter,NULL);

    gst_element_link(GstData.v4l2src,GstData.capsfilter);

    GstPad *gstpad=gst_element_get_static_pad(GstData.capsfilter,"src");

    gst_element_add_pad(GstData.srcbin,gst_ghost_pad_new("src",gstpad));
    // g_object_set(G_OBJECT(GstData.v4l2src), "device", video0.m_devname, NULL);

    gst_bin_add_many(GST_BIN(GstData.pipeline), GstData.srcbin,GstData.videosink, NULL);

    gst_element_link(GstData.srcbin,GstData.videosink);

    // if (!gst_element_link(GstData.videoconverter,GstData.videosink))
    // {
    //     g_printerr("GstData.videoconverter cannot link with GstData.videosink!\n");
    //     gst_object_unref(GstData.pipeline);
    //     return -1;
    // }

    gst_element_set_state(GstData.pipeline, GST_STATE_PLAYING);

    GstData.bus = gst_element_get_bus(GstData.pipeline);
    GstData.bus_watch_id = gst_bus_add_watch(GstData.bus, bus_call, &GstData);
    gst_object_unref(GstData.bus);

    GstData.loop = g_main_loop_new(NULL, FALSE);

    g_timeout_add_seconds(1, timeout_handler, &GstData);

    g_main_loop_run(GstData.loop);

    gst_element_set_state(GstData.pipeline, GST_STATE_NULL);
    gst_object_unref(GstData.bus);
    gst_object_unref(GstData.pipeline);
    return 0;
}