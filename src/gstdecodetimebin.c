/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2016 Shota TAMURA <r3108.sh@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:element-decodetimebin
 *
 * FIXME:Describe decodetimebin here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! decodetimebin ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>

#include "gstdecodetimebin.h"

GST_DEBUG_CATEGORY_STATIC (gst_decodetime_bin_debug);
#define GST_CAT_DEFAULT gst_decodetime_bin_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

#define gst_decodetime_bin_parent_class parent_class
G_DEFINE_TYPE (GstDecodetimeBin, gst_decodetime_bin, GST_TYPE_BIN);

static void gst_decodetime_bin_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_decodetime_bin_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

/* GObject vmethod implementations */
static GstPadProbeReturn cb_have_buffer (GstPad * pad, GstPadProbeInfo * info,
    gpointer user_data);

/* initialize the decodetimebin's class */
static void
gst_decodetime_bin_class_init (GstDecodetimeBinClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  gobject_class->set_property = gst_decodetime_bin_set_property;
  gobject_class->get_property = gst_decodetime_bin_get_property;

  gst_element_class_set_details_simple (gstelement_class,
      "DecodetimeBin",
      "FIXME:Generic",
      "FIXME:Generic Template Element", "Shota TAMURA <r3108.sh@gmail.com>");

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&src_factory));
  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&sink_factory));
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static void
gst_decodetime_bin_init (GstDecodetimeBin * decodetime_bin)
{
  GstPad *pad;
  GstPad *gpad;
  GstPadTemplate *pad_tmpl;

  decodetime_bin->overlay = gst_element_factory_make ("textoverlay", "overlay");
  gst_bin_add (GST_BIN (decodetime_bin), decodetime_bin->overlay);

  decodetime_bin->decoder = gst_element_factory_make ("jpegdec", "decoder");
  gst_bin_add (GST_BIN (decodetime_bin), decodetime_bin->decoder);

  gst_element_link (decodetime_bin->decoder, decodetime_bin->overlay);

  pad = gst_element_get_static_pad (decodetime_bin->decoder, "sink");
  gst_pad_add_probe (pad, GST_PAD_PROBE_TYPE_BUFFER,
      (GstPadProbeCallback) cb_have_buffer, decodetime_bin, NULL);
  pad_tmpl = gst_static_pad_template_get (&sink_factory);
  gpad = gst_ghost_pad_new_from_template ("sink", pad, pad_tmpl);
  gst_element_add_pad (GST_ELEMENT (decodetime_bin), gpad);
  gst_object_unref (pad_tmpl);
  gst_object_unref (pad);

  pad = gst_element_get_static_pad (decodetime_bin->decoder, "src");
  gst_pad_add_probe (pad, GST_PAD_PROBE_TYPE_BUFFER,
      (GstPadProbeCallback) cb_have_buffer, decodetime_bin, NULL);
  gst_object_unref (pad);

  pad = gst_element_get_static_pad (decodetime_bin->overlay, "src");
  pad_tmpl = gst_static_pad_template_get (&src_factory);
  gpad = gst_ghost_pad_new_from_template ("src", pad, pad_tmpl);
  gst_element_add_pad (GST_ELEMENT (decodetime_bin), gpad);
  gst_object_unref (pad_tmpl);
  gst_object_unref (pad);

  decodetime_bin->clock = NULL;
}

static void
gst_decodetime_bin_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
}

static void
gst_decodetime_bin_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
}

/* GstElement vmethod implementations */

static GstPadProbeReturn
cb_have_buffer (GstPad * pad, GstPadProbeInfo * info, gpointer user_data)
{
  static GstClockTime base_time;
  static GstClockTime abs_time;
  GstPadDirection direction;
  GstDecodetimeBin *bin = (GstDecodetimeBin *) user_data;

  if (!bin->clock) {
    GstElement *parent = GST_ELEMENT_PARENT (bin);
    bin->clock = gst_element_get_clock (parent);
  } else {
    direction = gst_pad_get_direction (pad);
    switch (direction) {
      case GST_PAD_SINK:
      {
        base_time = gst_clock_get_time (bin->clock);
      }
        break;
      case GST_PAD_SRC:
      {

        GstElement *overlay = ((GstDecodetimeBin *) user_data)->overlay;
        GString *string = g_string_new (NULL);

        abs_time = gst_clock_get_time (bin->clock);

        g_string_printf (string, "decode time: %" GST_TIME_FORMAT,
            GST_TIME_ARGS (abs_time - base_time));
        g_object_set (G_OBJECT (overlay), "text", string->str, NULL);
        g_string_free (string, TRUE);
      }
        break;
      default:
        break;
    }
  }

  return GST_PAD_PROBE_OK;
}

/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
decodetimebin_init (GstPlugin * decodetimebin)
{
  /* debug category for fltering log messages
   *
   * exchange the string 'Template decodetimebin' with your description
   */
  GST_DEBUG_CATEGORY_INIT (gst_decodetime_bin_debug, "decodetimebin",
      0, "Template decodetimebin");

  return gst_element_register (decodetimebin, "decodetimebin", GST_RANK_NONE,
      GST_TYPE_DECODETIMEBIN);
}

/* PACKAGE: this is usually set by autotools depending on some _INIT macro
 * in configure.ac and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use autotools to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "myfirstdecodetimebin"
#endif

/* gstreamer looks for this structure to register decodetimebins
 *
 * exchange the string 'Template decodetimebin' with your decodetimebin description
 */
GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    decodetimebin,
    "Template decodetimebin",
    decodetimebin_init, VERSION, "LGPL", "GStreamer", "http://gstreamer.net/")
