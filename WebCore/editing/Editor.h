/*
 * Copyright (C) 2006 Apple Computer, Inc.  All rights reserved.
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

#ifndef EDITOR_H
#define EDITOR_H

#include <wtf/Forward.h>
#include <wtf/OwnPtr.h>
#include <wtf/RefPtr.h>

#include "Frame.h"

namespace WebCore {

class DeleteButtonController;
class EditorClient;
class Frame;
class HTMLElement;
class Range;
class Selection;

// make platform-specific and implement - this is temporary placeholder
typedef int Pasteboard;

class Editor {
public:
    Editor(Frame*, PassRefPtr<EditorClient>);
    ~Editor();

    void cut();
    void copy();
    void paste();
    void performDelete();

    bool shouldShowDeleteInterface(HTMLElement*);

    void respondToChangedSelection(const Selection& oldSelection);
    void respondToChangedContents();
    
    Frame::TriState selectionUnorderedListState() const;
    Frame::TriState selectionOrderedListState() const;
    
    void removeFormattingAndStyle();

    Frame* frame() const { return m_frame; }
    DeleteButtonController* deleteButtonController() const { return m_deleteButtonController.get(); }

private:
    Frame* m_frame;
    RefPtr<EditorClient> m_client;
    OwnPtr<DeleteButtonController> m_deleteButtonController;

    bool canCopy();
    bool canCut();
    bool canDelete();
    bool canDeleteRange(Range*);
    bool canPaste();
    bool canSmartCopyOrDelete();
    bool isSelectionRichlyEditable();
    Range* selectedRange();
    bool shouldDeleteRange(Range*);
    bool tryDHTMLCopy();
    bool tryDHTMLCut();
    bool tryDHTMLPaste();
    void deleteSelection();
    void deleteSelectionWithSmartDelete(bool enabled);
    void pasteAsPlainTextWithPasteboard(Pasteboard);
    void pasteWithPasteboard(Pasteboard, bool allowPlainText);
    void writeSelectionToPasteboard(Pasteboard);

};

} // namespace WebCore

#endif // EDITOR_H
