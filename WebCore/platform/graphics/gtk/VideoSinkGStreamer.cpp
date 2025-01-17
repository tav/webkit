/*
 *  Copyright (C) 2007 OpenedHand
 *  Copyright (C) 2007 Alp Toker <alp@atoker.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * SECTION:webkit-video-sink
 * @short_description: GStreamer video sink
 *
 * #WebKitVideoSink is a GStreamer sink element that triggers
 * repaints in the WebKit GStreamer media player for the
 * current video buffer.
 */

#include "config.h"
#include "VideoSinkGStreamer.h"

#include <glib.h>
#include <gst/gst.h>
#include <gst/video/video.h>

static GstStaticPadTemplate sinktemplate = GST_STATIC_PAD_TEMPLATE("sink",
                                                                   GST_PAD_SINK, GST_PAD_ALWAYS,
// CAIRO_FORMAT_RGB24 used to render the video buffers is little/big endian dependant.
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
                                                                   GST_STATIC_CAPS(GST_VIDEO_CAPS_BGRx)
#else
                                                                   GST_STATIC_CAPS(GST_VIDEO_CAPS_xRGB)
#endif
);

GST_DEBUG_CATEGORY_STATIC(webkit_video_sink_debug);
#define GST_CAT_DEFAULT webkit_video_sink_debug

static GstElementDetails webkit_video_sink_details =
    GST_ELEMENT_DETAILS((gchar*) "WebKit video sink",
                        (gchar*) "Sink/Video",
                        (gchar*) "Sends video data from a GStreamer pipeline to a Cairo surface",
                        (gchar*) "Alp Toker <alp@atoker.com>");

enum {
    REPAINT_REQUESTED,
    LAST_SIGNAL
};

enum {
    PROP_0
};

static guint webkit_video_sink_signals[LAST_SIGNAL] = { 0, };

struct _WebKitVideoSinkPrivate {
    GstBuffer* buffer;
    guint timeout_id;
    GMutex* buffer_mutex;
    GCond* data_cond;

    // If this is TRUE all processing should finish ASAP
    // This is necessary because there could be a race between
    // unlock() and render(), where unlock() wins, signals the
    // GCond, then render() tries to render a frame although
    // everything else isn't running anymore. This will lead
    // to deadlocks because render() holds the stream lock.
    //
    // Protected by the buffer mutex
    gboolean unlocked;
};

#define _do_init(bla) \
    GST_DEBUG_CATEGORY_INIT(webkit_video_sink_debug, \
                            "webkitsink", \
                            0, \
                            "webkit video sink")

GST_BOILERPLATE_FULL(WebKitVideoSink,
                     webkit_video_sink,
                     GstVideoSink,
                     GST_TYPE_VIDEO_SINK,
                     _do_init);

static void
webkit_video_sink_base_init(gpointer g_class)
{
    GstElementClass* element_class = GST_ELEMENT_CLASS(g_class);

    gst_element_class_add_pad_template(element_class, gst_static_pad_template_get(&sinktemplate));
    gst_element_class_set_details(element_class, &webkit_video_sink_details);
}

static void
webkit_video_sink_init(WebKitVideoSink* sink, WebKitVideoSinkClass* klass)
{
    WebKitVideoSinkPrivate* priv;

    sink->priv = priv = G_TYPE_INSTANCE_GET_PRIVATE(sink, WEBKIT_TYPE_VIDEO_SINK, WebKitVideoSinkPrivate);
    priv->data_cond = g_cond_new();
    priv->buffer_mutex = g_mutex_new();
}

static gboolean
webkit_video_sink_timeout_func(gpointer data)
{
    WebKitVideoSink* sink = reinterpret_cast<WebKitVideoSink*>(data);
    WebKitVideoSinkPrivate* priv = sink->priv;
    GstBuffer* buffer;

    g_mutex_lock(priv->buffer_mutex);
    buffer = priv->buffer;
    priv->buffer = 0;
    priv->timeout_id = 0;

    if (!buffer || priv->unlocked || G_UNLIKELY(!GST_IS_BUFFER(buffer))) {
        g_cond_signal(priv->data_cond);
        g_mutex_unlock(priv->buffer_mutex);
        return FALSE;
    }

    if (G_UNLIKELY(!GST_BUFFER_CAPS(buffer))) {
        buffer = gst_buffer_make_metadata_writable(buffer);
        gst_buffer_set_caps(buffer, GST_PAD_CAPS(GST_BASE_SINK_PAD(sink)));
    }

    g_signal_emit(sink, webkit_video_sink_signals[REPAINT_REQUESTED], 0, buffer);
    gst_buffer_unref(buffer);
    g_cond_signal(priv->data_cond);
    g_mutex_unlock(priv->buffer_mutex);

    return FALSE;
}

static GstFlowReturn
webkit_video_sink_render(GstBaseSink* bsink, GstBuffer* buffer)
{
    WebKitVideoSink* sink = WEBKIT_VIDEO_SINK(bsink);
    WebKitVideoSinkPrivate* priv = sink->priv;

    g_mutex_lock(priv->buffer_mutex);

    if (priv->unlocked) {
        g_mutex_unlock(priv->buffer_mutex);
        return GST_FLOW_OK;
    }

    priv->buffer = gst_buffer_ref(buffer);

    // Use HIGH_IDLE+20 priority, like Gtk+ for redrawing operations.
    priv->timeout_id = g_timeout_add_full(G_PRIORITY_HIGH_IDLE + 20, 0,
                                          webkit_video_sink_timeout_func,
                                          gst_object_ref(sink),
                                          (GDestroyNotify)gst_object_unref);

    g_cond_wait(priv->data_cond, priv->buffer_mutex);
    g_mutex_unlock(priv->buffer_mutex);
    return GST_FLOW_OK;
}

static void
webkit_video_sink_dispose(GObject* object)
{
    WebKitVideoSink* sink = WEBKIT_VIDEO_SINK(object);
    WebKitVideoSinkPrivate* priv = sink->priv;

    if (priv->data_cond) {
        g_cond_free(priv->data_cond);
        priv->data_cond = 0;
    }

    if (priv->buffer_mutex) {
        g_mutex_free(priv->buffer_mutex);
        priv->buffer_mutex = 0;
    }

    G_OBJECT_CLASS(parent_class)->dispose(object);
}

static void
unlock_buffer_mutex(WebKitVideoSinkPrivate* priv)
{
    g_mutex_lock(priv->buffer_mutex);

    if (priv->buffer) {
        gst_buffer_unref(priv->buffer);
        priv->buffer = 0;
    }

    priv->unlocked = TRUE;

    g_cond_signal(priv->data_cond);
    g_mutex_unlock(priv->buffer_mutex);
}

static gboolean
webkit_video_sink_unlock(GstBaseSink* object)
{
    WebKitVideoSink* sink = WEBKIT_VIDEO_SINK(object);

    unlock_buffer_mutex(sink->priv);

    return GST_CALL_PARENT_WITH_DEFAULT(GST_BASE_SINK_CLASS, unlock,
                                        (object), TRUE);
}

static gboolean
webkit_video_sink_unlock_stop(GstBaseSink* object)
{
    WebKitVideoSink* sink = WEBKIT_VIDEO_SINK(object);
    WebKitVideoSinkPrivate* priv = sink->priv;

    g_mutex_lock(priv->buffer_mutex);
    priv->unlocked = FALSE;
    g_mutex_unlock(priv->buffer_mutex);

    return GST_CALL_PARENT_WITH_DEFAULT(GST_BASE_SINK_CLASS, unlock_stop,
                                        (object), TRUE);
}

static gboolean
webkit_video_sink_stop(GstBaseSink* base_sink)
{
    WebKitVideoSinkPrivate* priv = WEBKIT_VIDEO_SINK(base_sink)->priv;

    unlock_buffer_mutex(priv);
    return TRUE;
}

static gboolean
webkit_video_sink_start(GstBaseSink* base_sink)
{
    WebKitVideoSinkPrivate* priv = WEBKIT_VIDEO_SINK(base_sink)->priv;

    g_mutex_lock(priv->buffer_mutex);
    priv->unlocked = FALSE;
    g_mutex_unlock(priv->buffer_mutex);
    return TRUE;
}

static void
marshal_VOID__MINIOBJECT(GClosure * closure, GValue * return_value,
                         guint n_param_values, const GValue * param_values,
                         gpointer invocation_hint, gpointer marshal_data)
{
  typedef void (*marshalfunc_VOID__MINIOBJECT) (gpointer obj, gpointer arg1, gpointer data2);
  marshalfunc_VOID__MINIOBJECT callback;
  GCClosure *cc = (GCClosure *) closure;
  gpointer data1, data2;

  g_return_if_fail(n_param_values == 2);

  if (G_CCLOSURE_SWAP_DATA(closure)) {
      data1 = closure->data;
      data2 = g_value_peek_pointer(param_values + 0);
  } else {
      data1 = g_value_peek_pointer(param_values + 0);
      data2 = closure->data;
  }
  callback = (marshalfunc_VOID__MINIOBJECT) (marshal_data ? marshal_data : cc->callback);

  callback(data1, gst_value_get_mini_object(param_values + 1), data2);
}

static void
webkit_video_sink_class_init(WebKitVideoSinkClass* klass)
{
    GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
    GstBaseSinkClass* gstbase_sink_class = GST_BASE_SINK_CLASS(klass);

    g_type_class_add_private(klass, sizeof(WebKitVideoSinkPrivate));

    gobject_class->dispose = webkit_video_sink_dispose;

    gstbase_sink_class->unlock = webkit_video_sink_unlock;
    gstbase_sink_class->unlock_stop = webkit_video_sink_unlock_stop;
    gstbase_sink_class->render = webkit_video_sink_render;
    gstbase_sink_class->preroll = webkit_video_sink_render;
    gstbase_sink_class->stop = webkit_video_sink_stop;
    gstbase_sink_class->start = webkit_video_sink_start;

    webkit_video_sink_signals[REPAINT_REQUESTED] = g_signal_new("repaint-requested",
            G_TYPE_FROM_CLASS(klass),
            (GSignalFlags)(G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION),
            0,
            0,
            0,
            marshal_VOID__MINIOBJECT,
            G_TYPE_NONE, 1, GST_TYPE_BUFFER);
}

/**
 * webkit_video_sink_new:
 *
 * Creates a new GStreamer video sink.
 *
 * Return value: a #GstElement for the newly created video sink
 */
GstElement*
webkit_video_sink_new(void)
{
    return (GstElement*)g_object_new(WEBKIT_TYPE_VIDEO_SINK, 0);
}

