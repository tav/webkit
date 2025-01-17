/*
 * Copyright (C) 2008, 2009 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ScriptString_h
#define ScriptString_h

#include "PlatformString.h"
#include "ScriptStringImpl.h"
#include "V8Binding.h"

namespace WebCore {

class ScriptString {
public:
    ScriptString() : m_impl(0) {}
    ScriptString(const String& s) : m_impl(new ScriptStringImpl(s)) {}
    ScriptString(const char* s) : m_impl(new ScriptStringImpl(s)) {}

    operator String() const { return m_impl->toString(); }

    bool isNull() const { return !m_impl.get() || m_impl->isNull(); }
    size_t size() const { return m_impl->size(); }

    ScriptString& operator=(const char* s)
    {
        m_impl = new ScriptStringImpl(s);
        return *this;
    }

    ScriptString& operator+=(const String& s)
    {
        m_impl->append(s);
        return *this;
    }

    v8::Handle<v8::Value> v8StringOrNull() const
    {
        return isNull() ? v8::Handle<v8::Value>(v8::Null()) : v8::Handle<v8::Value>(m_impl->v8StringHandle());
    }

private:
    RefPtr<ScriptStringImpl> m_impl;
};

} // namespace WebCore

#endif // ScriptString_h
