/***************************************************************************
**
** Copyright (c) 2013 Jolla Ltd.
**
** This file is part of lipstick.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file LICENSE.LGPL included in the packaging
** of this file.
**
****************************************************************************/

#include "windowproperty.h"

#include "lipstickcompositor.h"
#include "lipstickcompositorwindow.h"

WindowProperty::WindowProperty()
    : m_windowId(0), m_waitingRefProperty(false)
{
    LipstickCompositor *c = LipstickCompositor::instance();
    if (!c)
        qWarning("WindowProperty: Compositor must be created before WindowProperty");
}

int WindowProperty::windowId() const
{
    return m_windowId;
}

void WindowProperty::setWindowId(int window)
{
    if (m_windowId == window)
        return;

    m_windowId = window;

    if (m_window) {
        QObject::disconnect(m_window, &LipstickCompositorWindow::windowPropertiesChanged,
                            this, &WindowProperty::handleWindowPropertiesChanged);
        QObject::disconnect(m_window, &QObject::destroyed,
                            this, &WindowProperty::valueChanged);
        m_window = nullptr;
    }

    LipstickCompositor *c = LipstickCompositor::instance();
    if (c) {
        m_window = qobject_cast<LipstickCompositorWindow *>(c->windowForId(window));
    }

    if (m_window) {
        QObject::connect(m_window, &LipstickCompositorWindow::windowPropertiesChanged,
                         this, &WindowProperty::handleWindowPropertiesChanged);
        QObject::connect(m_window, &QObject::destroyed,
                         this, &WindowProperty::valueChanged);
    }

    emit windowIdChanged();
    emit valueChanged();
}

void WindowProperty::handleWindowPropertiesChanged()
{
    emit valueChanged();
}

QString WindowProperty::property() const
{
    return m_property;
}

void WindowProperty::setProperty(const QString &p)
{
    if (m_property == p)
        return;

    m_property = p;
    emit propertyChanged();
    emit valueChanged();
}

void WindowProperty::connectRef()
{
    LipstickCompositor *c = LipstickCompositor::instance();
    if (c && !m_waitingRefProperty) {
        m_waitingRefProperty = true;
        QObject::connect(c, &LipstickCompositor::availableWinIdsChanged,
                         this, &WindowProperty::availableWinIdsChanged);
    }
}

QVariant WindowProperty::value()
{
    if (!m_window)
        return QVariant();

    QVariant rv = m_window->windowProperties().value(m_property);
    if (rv.metaType() == QMetaType::fromType<QString>() && rv.toString().startsWith("__winref:")) {
        QString refId = rv.toString().mid(9);
        uint id = refId.toUInt();
        connectRef();

        if (id) {
            int win = LipstickCompositor::instance()->windowIdForLink(m_windowId, id);
            return QVariant(win);
        } else {
            return QVariant(0);
        }
    } else {
        return rv;
    }
}

void WindowProperty::availableWinIdsChanged()
{
    if (value().isValid())
        emit valueChanged();
}