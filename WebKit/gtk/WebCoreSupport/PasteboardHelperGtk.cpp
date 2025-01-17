/*
 *  Copyright (C) 2007 Luca Bruno <lethalman88@gmail.com>
 *  Copyright (C) 2009 Holger Hans Peter Freyther
 *  Copyright (C) 2009 Martin Robinson
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

#include "config.h"
#include "PasteboardHelperGtk.h"

#include "Frame.h"
#include "webkitwebframe.h"
#include "webkitwebview.h"
#include "webkitprivate.h"

#include <gtk/gtk.h>

using namespace WebCore;

namespace WebKit {

static GdkAtom gdkMarkupAtom = gdk_atom_intern("text/html", FALSE);
static GdkAtom netscapeURLAtom = gdk_atom_intern("_NETSCAPE_URL", FALSE);
static GdkAtom uriListAtom = gdk_atom_intern("text/uri-list", FALSE);

GtkClipboard* PasteboardHelperGtk::defaultClipboard()
{
    return gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
}

GtkClipboard* PasteboardHelperGtk::defaultClipboardForFrame(Frame* frame)
{
    WebKitWebView* webView = webkit_web_frame_get_web_view(kit(frame));
    return gtk_widget_get_clipboard(GTK_WIDGET(webView),
                                    GDK_SELECTION_CLIPBOARD);
}

GtkClipboard* PasteboardHelperGtk::primaryClipboard()
{
    return gtk_clipboard_get(GDK_SELECTION_PRIMARY);
}

GtkClipboard* PasteboardHelperGtk::primaryClipboardForFrame(Frame* frame)
{
    WebKitWebView* webView = webkit_web_frame_get_web_view(kit(frame));
    return gtk_widget_get_clipboard(GTK_WIDGET(webView),
                                    GDK_SELECTION_PRIMARY);
}

static Vector<KURL> urisToKURLVector(gchar** uris)
{
    ASSERT(uris);

    Vector<KURL> uriList;
    gchar** currentURI = uris;
    while (*currentURI) {
        uriList.append(KURL(KURL(), *currentURI));
        currentURI++;
    }

    return uriList;
}

void PasteboardHelperGtk::getClipboardContents(GtkClipboard* clipboard)
{
    DataObjectGtk* dataObject = DataObjectGtk::forClipboard(clipboard);

    String text;
    if (gtk_clipboard_wait_is_text_available(clipboard)) {
        gchar* textData = gtk_clipboard_wait_for_text(clipboard);
        if (textData) {
            text = textData;
            g_free(textData);
        }
    }
    dataObject->setText(text);

    String markup;
    if (gtk_clipboard_wait_is_target_available(clipboard, gdkMarkupAtom)) {

        GtkSelectionData* data = gtk_clipboard_wait_for_contents(clipboard, gdkMarkupAtom);
        if (data && data->length > 0 && data->type == gdkMarkupAtom) {
            gchar **list;
            gint count = gdk_text_property_to_utf8_list_for_display(
                data->display, data->type, data->format, data->data, data->length, &list);

            if (count > 0)
                markup = list[0];

            for (int i = 1; i < count; i++)
                g_free(list[i]);
            g_free(list);
       }

       if (data)
           gtk_selection_data_free(data);
    }
    dataObject->setMarkup(markup);

    Vector<KURL> uriList;
    if (gtk_clipboard_wait_is_target_available(clipboard, uriListAtom)) {
        GtkSelectionData* data = gtk_clipboard_wait_for_contents(clipboard, uriListAtom);
        if (data) {
            gchar** uris = gtk_selection_data_get_uris(data);
            if (uris) {
                uriList = urisToKURLVector(uris);
                g_strfreev(uris);
            }
            gtk_selection_data_free(data);
        }
    }
    dataObject->setURIList(uriList);

    // TODO: Eventually WebKit may need to support for reading image
    // data directly from the clipboard, but for now, don't read image data.
}

static bool settingClipboard = false;
static void getClipboardContentsCallback(GtkClipboard* clipboard, GtkSelectionData *selectionData, guint info, gpointer data)
{
    DataObjectGtk* dataObject = reinterpret_cast<DataObjectGtk*>(data);
    ASSERT(dataObject);
    PasteboardHelper::helper()->fillSelectionData(selectionData, info, dataObject);
}

static void clearClipboardContentsCallback(GtkClipboard* clipboard, gpointer data)
{
    DataObjectGtk* dataObject = reinterpret_cast<DataObjectGtk*>(data);
    ASSERT(dataObject);

    // GTK will call the clear clipboard callback during
    // gtk_clipboard_set_with_data. We don't actually want to clear
    // the data object during that time.
    if (!settingClipboard)
        dataObject->clear();
}

void PasteboardHelperGtk::writeClipboardContents(GtkClipboard* clipboard)
{
    DataObjectGtk* dataObject = DataObjectGtk::forClipboard(clipboard);
    GtkTargetList* list = targetListForDataObject(dataObject);

    int numberOfTargets;
    GtkTargetEntry* table = gtk_target_table_new_from_list(list, &numberOfTargets);

    if (numberOfTargets > 0 && table) {
        settingClipboard = true;
        gtk_clipboard_set_with_data(clipboard, table, numberOfTargets,
                                    getClipboardContentsCallback,
                                    clearClipboardContentsCallback, dataObject);
        settingClipboard = false;

    } else
        gtk_clipboard_clear(clipboard);

    if (table)
        gtk_target_table_free(table, numberOfTargets);
    gtk_target_list_unref(list);
}


void PasteboardHelperGtk::fillSelectionData(GtkSelectionData* selectionData, guint info, DataObjectGtk* dataObject)
{
    if (info == WEBKIT_WEB_VIEW_TARGET_INFO_TEXT)
        gtk_selection_data_set_text(selectionData, dataObject->text().utf8().data(), -1);

    else if (info == WEBKIT_WEB_VIEW_TARGET_INFO_HTML) {
        gchar* markup = g_strdup(dataObject->markup().utf8().data());
        gtk_selection_data_set(selectionData, selectionData->target, 8,
                               reinterpret_cast<const guchar*>(markup),
                               strlen(markup));
        g_free(markup);

    } else if (info == WEBKIT_WEB_VIEW_TARGET_INFO_URI_LIST) {
        Vector<KURL> uriList(dataObject->uriList());
        gchar** uris = g_new0(gchar*, uriList.size() + 1);
        for (size_t i = 0; i < uriList.size(); i++)
            uris[i] = g_strdup(uriList[i].string().utf8().data());

        gtk_selection_data_set_uris(selectionData, uris);
        g_strfreev(uris);

    } else if (info == WEBKIT_WEB_VIEW_TARGET_INFO_NETSCAPE_URL && dataObject->hasURL()) {
        String url(dataObject->url());
        String result(url);
        result.append("\n");

        if (dataObject->hasText())
            result.append(dataObject->text());
        else
            result.append(url);

        gchar* resultData = g_strdup(result.utf8().data());
        gtk_selection_data_set(selectionData, selectionData->target, 8,
                               reinterpret_cast<const guchar*>(resultData),
                               strlen(resultData));
        g_free(resultData);

    } else if (info == WEBKIT_WEB_VIEW_TARGET_INFO_IMAGE)
        gtk_selection_data_set_pixbuf(selectionData, dataObject->image());
}

void PasteboardHelperGtk::fillDataObject(GtkSelectionData* selectionData, guint info, DataObjectGtk* dataObject)
{
    if (info == WEBKIT_WEB_VIEW_TARGET_INFO_TEXT) {
        gchar* text = reinterpret_cast<gchar*>(gtk_selection_data_get_text(selectionData));
        if (!text)
            return;

        dataObject->setText(text);
        g_free(text);

    } else if (info == WEBKIT_WEB_VIEW_TARGET_INFO_HTML) {
        const gchar* data = reinterpret_cast<const gchar*>(selectionData->data);
        if (!data)
            return;

        gchar* markup = g_strndup(data, selectionData->length);
        dataObject->setMarkup(markup);
        g_free(markup);

    } else if (info == WEBKIT_WEB_VIEW_TARGET_INFO_URI_LIST) {
        gchar** uris = gtk_selection_data_get_uris(selectionData);
        if (!uris)
            return;

        Vector<KURL> uriList(urisToKURLVector(uris));
        dataObject->setURIList(uriList);
        g_strfreev(uris);

    } else if (info == WEBKIT_WEB_VIEW_TARGET_INFO_NETSCAPE_URL) {
        const gchar* data = reinterpret_cast<const gchar*>(selectionData->data);
        if (!data)
            return;

        gchar* urlWithLabelChars = g_strndup(data, selectionData->length);
        String urlWithLabel(urlWithLabelChars);

        Vector<String> pieces;
        urlWithLabel.split("\n", pieces);

        Vector<KURL> uriList;
        uriList.append(KURL(KURL(), pieces[0]));
        dataObject->setURIList(uriList);

        if (pieces.size() > 1)
            dataObject->setText(pieces[1]);

        g_free(urlWithLabelChars);

    } else if (info == WEBKIT_WEB_VIEW_TARGET_INFO_IMAGE) {
        GdkPixbuf* image = gtk_selection_data_get_pixbuf(selectionData);
        ASSERT(image);

        dataObject->setImage(image);
        g_object_unref(image);
    }
}

GtkTargetList* PasteboardHelperGtk::fullTargetList()
{
    static GtkTargetList* list = 0;
    if (!list) {
        list = gtk_target_list_new(0, 0);
        gtk_target_list_add_text_targets(list, WEBKIT_WEB_VIEW_TARGET_INFO_TEXT);
        gtk_target_list_add(list, gdkMarkupAtom, 0, WEBKIT_WEB_VIEW_TARGET_INFO_HTML);
        gtk_target_list_add_uri_targets(list, WEBKIT_WEB_VIEW_TARGET_INFO_URI_LIST);
        gtk_target_list_add(list, netscapeURLAtom, 0,
                            WEBKIT_WEB_VIEW_TARGET_INFO_NETSCAPE_URL);
    }
    return list;
}

GtkTargetList* PasteboardHelperGtk::targetListForDataObject(DataObjectGtk* dataObject)
{
    GtkTargetList* list = gtk_target_list_new(NULL, 0);

    if (dataObject->hasText())
        gtk_target_list_add_text_targets(list, WEBKIT_WEB_VIEW_TARGET_INFO_TEXT);

    if (dataObject->hasMarkup())
        gtk_target_list_add(list, gdkMarkupAtom, 0, WEBKIT_WEB_VIEW_TARGET_INFO_HTML);

    if (dataObject->hasURIList()) {
        gtk_target_list_add_uri_targets(list, WEBKIT_WEB_VIEW_TARGET_INFO_URI_LIST);
        gtk_target_list_add(list, netscapeURLAtom, 0, WEBKIT_WEB_VIEW_TARGET_INFO_NETSCAPE_URL);
    }

    if (dataObject->hasImage())
        gtk_target_list_add_image_targets(list, WEBKIT_WEB_VIEW_TARGET_INFO_IMAGE, TRUE);

    return list;
}

GtkTargetList* PasteboardHelperGtk::targetListForDragContext(GdkDragContext* context)
{
    // We want to avoid unecessary asynchronous data conversions, so we'll only
    // choose the first applicable target for each data type. The one exception
    // to this rule is for _NETSCAPE_URL -- it can only carry one URL, so a
    // text/uri-list with multiple URLs should be preferred.
    // TODO: Should we prefer some image targets over others?
    GtkTargetList* fullList = fullTargetList();
    GtkTargetList* resultList = gtk_target_list_new(0, 0);
    bool targets[WEBKIT_WEB_VIEW_TARGET_INFO_NETSCAPE_URL + 1];
    targets[WEBKIT_WEB_VIEW_TARGET_INFO_TEXT] = false;
    targets[WEBKIT_WEB_VIEW_TARGET_INFO_HTML] = false;
    targets[WEBKIT_WEB_VIEW_TARGET_INFO_URI_LIST] = false;
    targets[WEBKIT_WEB_VIEW_TARGET_INFO_IMAGE] = false;
    targets[WEBKIT_WEB_VIEW_TARGET_INFO_NETSCAPE_URL] =  false;

    GList* contextTargets = context->targets;
    while (contextTargets) {
        GdkAtom target = reinterpret_cast<GdkAtom>(contextTargets->data);
        guint info = 0;
        if (gtk_target_list_find(fullList, target, &info)) {

            if (info != WEBKIT_WEB_VIEW_TARGET_INFO_NETSCAPE_URL && !targets[info])
                gtk_target_list_add(resultList, target, 0, info);
            targets[info] = true;
        }

        contextTargets = contextTargets->next;
    }

    if (!targets[WEBKIT_WEB_VIEW_TARGET_INFO_URI_LIST] &&
        targets[WEBKIT_WEB_VIEW_TARGET_INFO_NETSCAPE_URL])
        gtk_target_list_add(resultList, netscapeURLAtom, 0,
                            WEBKIT_WEB_VIEW_TARGET_INFO_NETSCAPE_URL);

    return resultList;
}

}
