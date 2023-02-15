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

#ifndef LIPSTICKCOMPOSITORWINDOW_H
#define LIPSTICKCOMPOSITORWINDOW_H

#include <QWaylandQuickItem>
#include <QWaylandBufferRef>
#include <QWaylandXdgToplevel>

#include "lipstickglobal.h"

class LipstickCompositorWindowHwcNode;

class LIPSTICK_EXPORT LipstickCompositorWindow : public QWaylandQuickItem
{
    Q_OBJECT

    Q_PROPERTY(int windowId READ windowId CONSTANT)
    Q_PROPERTY(bool isInProcess READ isInProcess CONSTANT)

    Q_PROPERTY(bool delayRemove READ delayRemove WRITE setDelayRemove NOTIFY delayRemoveChanged)
    Q_PROPERTY(QVariant userData READ userData WRITE setUserData NOTIFY userDataChanged)

    Q_PROPERTY(QString category READ category CONSTANT)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(qint64 processId READ processId CONSTANT)

    Q_PROPERTY(bool focusOnTouch READ focusOnTouch WRITE setFocusOnTouch NOTIFY focusOnTouchChanged)
    Q_PROPERTY(uint notificationMode READ notificationMode WRITE setNotificationMode NOTIFY notificationModeChanged)

public:
    LipstickCompositorWindow(int windowId, const QString &, QWaylandSurface *surface, QQuickItem *parent = 0);
    ~LipstickCompositorWindow();

    QVariant userData() const;
    void setUserData(QVariant);

    int windowId() const;
    qint64 processId() const;
    QString policyApplicationId() const;

    bool delayRemove() const;
    void setDelayRemove(bool);

    QString category() const;
    virtual QString title() const;
    virtual bool isInProcess() const;

    bool eventFilter(QObject *object, QEvent *event);

    Q_INVOKABLE void terminateProcess(int killTimeout);

    bool focusOnTouch() const;
    void setFocusOnTouch(bool focusOnTouch);

    uint notificationMode();
    void setNotificationMode(uint mode);

    void setTopLevel(QWaylandXdgToplevel *topLevel);
    Q_INVOKABLE void setMinimized(const QSize &size);
    Q_INVOKABLE void setMaximized(const QSize &size);
    Q_INVOKABLE void setFullscreen(const QSize &size);
    Q_INVOKABLE void unsetMaximized();
    Q_INVOKABLE void unsetFullscreen();
    Q_INVOKABLE void resize(const QSize &size);

protected:
    void itemChange(ItemChange change, const ItemChangeData &data);

    virtual bool event(QEvent *);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent *event);
    virtual void touchEvent(QTouchEvent *event);

signals:
    void userDataChanged();
    void titleChanged();
    void delayRemoveChanged();
    void committed();
    void focusOnTouchChanged();
    void windowFlagsChanged();
    void notificationModeChanged();
    void resized();

private slots:
    void handleTouchCancel();
    void killProcess();

private:
    friend class LipstickCompositor;
    friend class WindowModel;
    friend class WindowPixmapItem;
    void imageAddref(QQuickItem *item);
    void imageRelease(QQuickItem *item);

    bool canRemove() const;
    void tryRemove();
    void refreshMouseRegion();
    void refreshGrabbedKeys();
    void handleTouchEvent(QTouchEvent *e);

    void setTitle(QString title);
    void updatePolicyApplicationId();

    QString m_title;
    qint64 m_processId;
    QString m_policyApplicationId;
    int m_windowId;
    QString m_category;
    bool m_delayRemove:1;
    bool m_windowClosed:1;
    bool m_removePosted:1;
    bool m_interceptingTouch:1;
    bool m_mapped : 1;
    bool m_focusOnTouch : 1;
    QVariant m_data;
    QVector<QQuickItem *> m_refs;
    uint m_notificationMode;
    QWaylandXdgToplevel* m_topLevel;
};

#endif // LIPSTICKCOMPOSITORWINDOW_H
