/*
 * Copyright (C) 2007 Kevin Ollivier <kevino@theolliviers.com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "ChromeClientWx.h"
#include "FloatRect.h"
#include "FrameLoadRequest.h"
#include "NotImplemented.h"
#include "PlatformString.h"

#include <stdio.h>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <wx/textdlg.h>

#include "WebFrame.h"
#include "WebView.h"
#include "WebViewPrivate.h"

namespace WebCore {

ChromeClientWx::~ChromeClientWx()
{
}

void ChromeClientWx::chromeDestroyed()
{
    notImplemented();
}

void ChromeClientWx::setWindowRect(const FloatRect&)
{
    notImplemented();
}

FloatRect ChromeClientWx::windowRect()
{
    notImplemented();
    return FloatRect();
}

FloatRect ChromeClientWx::pageRect()
{
    notImplemented();
    return FloatRect();
}

float ChromeClientWx::scaleFactor()
{
    notImplemented();
    return 0.0;
}

void ChromeClientWx::focus()
{
    notImplemented();
}

void ChromeClientWx::unfocus()
{
    notImplemented();
}

bool ChromeClientWx::canTakeFocus(FocusDirection)
{
    notImplemented();
    return false;
}

void ChromeClientWx::takeFocus(FocusDirection)
{
    notImplemented();
}


Page* ChromeClientWx::createWindow(Frame*, const FrameLoadRequest& request, const WindowFeatures&)
{

    // FIXME: Create a EVT_WEBKIT_NEW_WINDOW event, and only run this code
    // when that event is not handled.
    
    Page* myPage = 0;
    wxWebFrame* newFrame = new wxWebFrame(wxTheApp->GetAppName());
    
    if (newFrame->webview) {
        newFrame->webview->LoadURL(request.resourceRequest().url().url());
        newFrame->Show(true);

        WebViewPrivate* impl = newFrame->webview->m_impl;
        if (impl)
            myPage = impl->frame->page();
    }
    
    return myPage;
}

Page* ChromeClientWx::createModalDialog(Frame*, const FrameLoadRequest&)
{
    notImplemented();
    return 0;
}

void ChromeClientWx::show()
{
    notImplemented();
}

bool ChromeClientWx::canRunModal()
{
    notImplemented();
    return false;
}

void ChromeClientWx::runModal()
{
    notImplemented();
}

void ChromeClientWx::setToolbarsVisible(bool)
{
    notImplemented();
}

bool ChromeClientWx::toolbarsVisible()
{
    notImplemented();
    return false;
}

void ChromeClientWx::setStatusbarVisible(bool)
{
    notImplemented();
}

bool ChromeClientWx::statusbarVisible()
{
    notImplemented();
    return false;
}

void ChromeClientWx::setScrollbarsVisible(bool)
{
    notImplemented();
}

bool ChromeClientWx::scrollbarsVisible()
{
    notImplemented();
    return false;
}

void ChromeClientWx::setMenubarVisible(bool)
{
    notImplemented();
}

bool ChromeClientWx::menubarVisible()
{
    notImplemented();
    return false;
}

void ChromeClientWx::setResizable(bool)
{
    notImplemented();
}

void ChromeClientWx::addMessageToConsole(const String&,
                                          unsigned int,
                                          const String&)
{
    notImplemented();
}

bool ChromeClientWx::canRunBeforeUnloadConfirmPanel()
{
    notImplemented();
    return true;
}

bool ChromeClientWx::runBeforeUnloadConfirmPanel(const String& string,
                                                  Frame* frame)
{
    wxMessageDialog dialog(NULL, string, wxT("Confirm Action?"), wxYES_NO);
    return dialog.ShowModal() == wxYES;
}

void ChromeClientWx::closeWindowSoon()
{
    notImplemented();
}

/*
    Sites for testing prompts: 
    Alert - just type in a bad web address or http://www.htmlite.com/JS002.php
    Prompt - http://www.htmlite.com/JS007.php
    Confirm - http://www.htmlite.com/JS006.php
*/

void ChromeClientWx::runJavaScriptAlert(Frame* frame, const String& string)
{
    wxMessageBox(string, wxT("JavaScript Alert"), wxOK);
}

bool ChromeClientWx::runJavaScriptConfirm(Frame* frame, const String& string)
{
    wxMessageDialog dialog(NULL, string, wxT("JavaScript Confirm"), wxYES_NO);
    dialog.Centre();
    return (dialog.ShowModal() == wxID_YES);
}

bool ChromeClientWx::runJavaScriptPrompt(Frame* frame, const String& message, const String& defaultValue, String& result)
{
    wxTextEntryDialog dialog(NULL, message, wxT("JavaScript Prompt"), wxEmptyString, wxOK | wxCANCEL);
    dialog.Centre();
    if (dialog.ShowModal() == wxID_OK) {
        result = dialog.GetValue();
        return true;
    }
    
    return false;
}

void ChromeClientWx::setStatusbarText(const String&)
{
    notImplemented();
}

bool ChromeClientWx::shouldInterruptJavaScript()
{
    notImplemented();
    return false;
}

bool ChromeClientWx::tabsToLinks() const
{
    notImplemented();
    return false;
}

IntRect ChromeClientWx::windowResizerRect() const
{
    notImplemented();
    return IntRect();
}

void ChromeClientWx::addToDirtyRegion(const IntRect&)
{
    notImplemented();
}

void ChromeClientWx::scrollBackingStore(int dx, int dy, 
                    const IntRect& scrollViewRect, 
                    const IntRect& clipRect)
{
    notImplemented();
}

void ChromeClientWx::updateBackingStore()
{
    notImplemented();
}

void ChromeClientWx::mouseDidMoveOverElement(const HitTestResult&, unsigned modifierFlags)
{
    notImplemented();
}

void ChromeClientWx::setToolTip(const String&)
{
    notImplemented();
}
void ChromeClientWx::print(Frame*)
{
    notImplemented();
}

unsigned long long ChromeClientWx::requestQuotaIncreaseForNewDatabase(Frame*, const SecurityOriginData&, const String&, unsigned long long)
{
    notImplemented();
    return 0;
}

unsigned long long ChromeClientWx::requestQuotaIncreaseForDatabaseOperation(Frame*, const SecurityOriginData&, const String&, unsigned long long)
{
    notImplemented();
    return 0;
}

}
