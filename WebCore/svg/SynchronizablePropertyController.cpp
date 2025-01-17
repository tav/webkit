/*
    Copyright (C) Research In Motion Limited 2009. All rights reserved.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "config.h"

#if ENABLE(SVG)
#include "SynchronizablePropertyController.h"

#include "NamedNodeMap.h"
#include "Node.h"
#include "SVGAnimatedProperty.h"

namespace WebCore {

SynchronizablePropertyController::SynchronizablePropertyController()
{
}

void SynchronizablePropertyController::registerProperty(const QualifiedName& attrName, SVGAnimatedPropertyBase* base)
{
    // 'attrName' is ambigious. For instance in SVGMarkerElement both 'orientType' / 'orientAngle'
    // SVG DOM objects are synchronized with the 'orient' attribute. This why we need a HashSet.
    SynchronizableProperty property(base);

    PropertyMap::iterator it = m_map.find(attrName.localName());
    if (it == m_map.end()) {
        Properties properties;
        properties.add(property);
        m_map.set(attrName.localName(), properties);
        return;
    }

    Properties& properties = it->second;
    ASSERT(!properties.isEmpty());

    properties.add(property);
}

void SynchronizablePropertyController::setPropertyNeedsSynchronization(const QualifiedName& attrName)
{
    PropertyMap::iterator itProp = m_map.find(attrName.localName());
    ASSERT(itProp != m_map.end());

    Properties& properties = itProp->second;
    ASSERT(!properties.isEmpty());

    Properties::iterator it = properties.begin();
    Properties::iterator end = properties.end();

    for (; it != end; ++it) {
        SynchronizableProperty& property = *it;
        ASSERT(property.base);
        property.shouldSynchronize = true;
    }
}

void SynchronizablePropertyController::synchronizeProperty(const String& name)
{
    PropertyMap::iterator itProp = m_map.find(name);
    if (itProp == m_map.end())
        return;

    Properties& properties = itProp->second;
    ASSERT(!properties.isEmpty());

    Properties::iterator it = properties.begin();
    Properties::iterator end = properties.end();

    for (; it != end; ++it) {
        SynchronizableProperty& property = *it;
        ASSERT(property.base);

        if (!property.shouldSynchronize)
            continue;

        property.base->synchronize();
    }
}

void SynchronizablePropertyController::synchronizeAllProperties()
{
    if (m_map.isEmpty())
        return;

    PropertyMap::iterator itProp = m_map.begin();
    PropertyMap::iterator endProp = m_map.end();

    for (; itProp != endProp; ++itProp) {
        Properties& properties = itProp->second;
        ASSERT(!properties.isEmpty());

        Properties::iterator it = properties.begin();
        Properties::iterator end = properties.end();

        for (; it != end; ++it) {
            SynchronizableProperty& property = *it;
            ASSERT(property.base);

            if (!property.shouldSynchronize)
                continue;

            property.base->synchronize();
        }
    }
}

void SynchronizablePropertyController::startAnimation(const String& name)
{
    PropertyMap::iterator itProp = m_map.find(name);
    if (itProp == m_map.end())
        return;

    Properties& properties = itProp->second;
    ASSERT(!properties.isEmpty());

    Properties::iterator it = properties.begin();
    Properties::iterator end = properties.end();

    for (; it != end; ++it) {
        SynchronizableProperty& property = *it;
        ASSERT(property.base);
        property.base->startAnimation();
    }
}

void SynchronizablePropertyController::stopAnimation(const String& name)
{
    PropertyMap::iterator itProp = m_map.find(name);
    if (itProp == m_map.end())
        return;

    Properties& properties = itProp->second;
    ASSERT(!properties.isEmpty());

    Properties::iterator it = properties.begin();
    Properties::iterator end = properties.end();

    for (; it != end; ++it) {
        SynchronizableProperty& property = *it;
        ASSERT(property.base);
        property.base->stopAnimation();
    }
}

}

#endif // ENABLE(SVG)
