/*
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies)
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

#ifndef QWebPageClient_h
#define QWebPageClient_h

#include <QRect>

class QWebPageClient {
public:
    virtual void scroll(int dx, int dy, const QRect&) = 0;
    virtual void update(const QRect&) = 0;
    virtual void setInputMethodEnabled(bool enable) = 0;
#if QT_VERSION >= 0x040600
    virtual void setInputMethodHint(Qt::InputMethodHint hint, bool enable) = 0;
#endif
    inline void resetCursor()
    {
#ifndef QT_NO_CURSOR
        if (!cursor().bitmap() && cursor().shape() == m_lastCursor.shape())
            return;
        updateCursor(m_lastCursor);
#endif
    }

    inline void setCursor(const QCursor& cursor)
    {
#ifndef QT_NO_CURSOR
        m_lastCursor = cursor;
        if (!cursor.bitmap() && cursor.shape() == this->cursor().shape())
            return;
        updateCursor(cursor);
#endif
    }

    virtual QPalette palette() const = 0;
    virtual int screenNumber() const = 0;
    virtual QWidget* ownerWidget() const = 0;

    virtual QObject* pluginParent() const = 0;

protected:
#ifndef QT_NO_CURSOR
    virtual QCursor cursor() const = 0;
    virtual void updateCursor(const QCursor& cursor) = 0;
#endif

private:
#ifndef QT_NO_CURSOR
    QCursor m_lastCursor;
#endif
};

#endif
