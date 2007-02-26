/*
 * Copyright (C) 2006 Nikolas Zimmermann <zimmermann@kde.org>
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

#if ENABLE(SVG)
#include "SVGResourceMasker.h"

#include "ImageBuffer.h"
#include "TextStream.h"

using namespace std;

namespace WebCore {

SVGResourceMasker::SVGResourceMasker()
    : SVGResource()
{
}

SVGResourceMasker::~SVGResourceMasker()
{
}

void SVGResourceMasker::setMask(auto_ptr<ImageBuffer> mask)
{
    m_mask.set(mask.release());
}

ImageBuffer* SVGResourceMasker::mask() const
{
    return m_mask.get();
}

TextStream& SVGResourceMasker::externalRepresentation(TextStream& ts) const
{
    ts << "[type=MASKER]";
    return ts;
}

SVGResourceMasker* getMaskerById(Document* document, const AtomicString& id)
{
    SVGResource* resource = getResourceById(document, id);
    if (resource && resource->isMasker())
        return static_cast<SVGResourceMasker*>(resource);

    return 0;
}

} // namespace WebCore

#endif
