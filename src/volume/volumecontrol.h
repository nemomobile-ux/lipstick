/***************************************************************************
**
** Copyright (c) 2012 Jolla Ltd.
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

#ifndef VOLUMECONTROL_H
#define VOLUMECONTROL_H

#include <QTimer>
#include <QObject>
#include "lipstickglobal.h"

class HomeWindow;
class PulseAudioControl;
class VolumeKeyListener;
class MDConfItem;

class QDBusPendingCallWatcher;

namespace ResourcePolicy {
    class ResourceSet;
}

/*!
 * \class VolumeControl
 *
 * \brief Shows a window for displaying the volume level.
 *
 * Creates a transparent window which can be used to show
 * the current volume level.
 */
class LIPSTICK_EXPORT VolumeControl : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(int maximumVolume READ maximumVolume NOTIFY maximumVolumeChanged)
    Q_PROPERTY(int safeVolume READ safeVolume NOTIFY safeVolumeChanged)
    Q_PROPERTY(int restrictedVolume READ restrictedVolume NOTIFY restrictedVolumeChanged)
    Q_PROPERTY(bool windowVisible READ windowVisible WRITE setWindowVisible NOTIFY windowVisibleChanged)
    Q_PROPERTY(bool callActive READ callActive NOTIFY callActiveChanged)
    Q_PROPERTY(int mediaState READ mediaState NOTIFY mediaStateChanged)
    Q_ENUMS(MediaState)

public:
    enum MediaState {
        MediaStateUnknown,
        MediaStateInactive,
        MediaStateForeground,
        MediaStateBackground,
        MediaStateActive
    };

    /*!
     * Creates a volume controller.
     *
     * \param parent the parent object
     */
    explicit VolumeControl(QObject *parent = nullptr);

    VolumeControl(bool hwKeysCapability, QObject *parent = nullptr);

    /*!
     * Destroys the volume controller.
     */
    virtual ~VolumeControl();

    /*!
     * Returns the current volume.
     *
     * \return the current volume
     */
    int volume() const;

    /*!
     * Sets the current volume to \a volume.
     */
    void setVolume(int volume);

    /*!
     * Returns the maximum volume.
     *
     * \return the maximum volume
     */
    int maximumVolume() const;

    /*!
     * Returns the safe volume.
     *
     * \return the safe volume
     */
    int safeVolume() const;

    /*!
     * Returns the maximum volume that the system will allow. If the user has not acknowleged the
     * safe volume warning \l safeVolume() will be returned otherwise \l maximumVolume() will be
     * returned.
     *
     * \return the restricted volume
     */
    int restrictedVolume() const;

    /*!
     * Returns whether the volume window is visible or not.
     *
     * \return \c true if the volume window is visible, \c false otherwise
     */
    bool windowVisible() const;

    /*!
     * Sets the visibility of the volume window.
     *
     * \param visible \c true if the volume window should be visible, \c false otherwise
     */
    void setWindowVisible(bool visible);

    /*!
     * Returns whether a call is active or not.
     *
     * \return \c true if a call is active, \c false otherwise
     */
    bool callActive() const;

    int mediaState() const;

    //! \reimp
    virtual bool eventFilter(QObject *watched, QEvent *event);
    //! \reimp_end

signals:
    //! Sent when the volume has changed.
    void volumeChanged();

    //! Sent when a volume up/down key was pressed or released
    void volumeKeyPressed(int key);
    void volumeKeyReleased(int key);

    //! Sent when the maximum volume has changed.
    void maximumVolumeChanged();

    //! Sent when the safe volume has changed.
    void safeVolumeChanged();

    //! Sent when the restricted volume has changed.
    void restrictedVolumeChanged();

    //! Sent when the visibility of the volume window has changed.
    void windowVisibleChanged();

    //! Sent when the call activity status has changed.
    void callActiveChanged();

    void mediaStateChanged();

    /*!
     * Sent when high volume or long listening time warning should show to user.
     *
     * \param initial \c true if warning is initial, listening time == 0 \c false otherwise
     */
    void showAudioWarning(bool initial);

public slots:
    /*!
     * Sets the audio warning acknowledged.
     *
     * \param acknowledged \c true if the used has acknowledged warning, \c false otherwise.
     */
    void setWarningAcknowledged(bool acknowledged);

private slots:
    //! Sets the volume and maximum volume
    void setVolume(int volume, int maximumVolume);

    //! An internal slot to handle the case when we got the hardware volume keys resource
    void hwKeyResourceAcquired();

    //! An internal slot to handle the case when we lost the hardware volume keys resource
    void hwKeyResourceLost();

    void hwKeysEnabled();
    void hwKeysDisabled();

    //! Used to capture safe volume level and reset it to safe when needed.
    void handleHighVolume(int safeLevel);

    //! Used to show long listening time warning
    void handleLongListeningTime(int listeningTime);

    //! Used to show call active status
    void handleCallActive(bool callActive);

    void handleMediaStateChanged(const QString &state);

    void createWindow();

    void inputPolicyChanged(const QString &status);
    void inputPolicyReply(QDBusPendingCallWatcher *watcher);

private:
    void setVolumeUpKeyState(bool pressed);
    void setVolumeDownKeyState(bool pressed);
    void evaluateKeyState();

    //! Returns whether the audio warning has been acknowledged by user.
    bool warningAcknowledged() const;

    //! The volume control window
    HomeWindow *m_window;

    //! PulseAudio volume controller
    PulseAudioControl *m_pulseAudioControl;

    //! A resource object for access to the volume keys
    ResourcePolicy::ResourceSet *m_hwKeyResource;

    //! Whether to react to volume key presses
    bool m_hwKeysAcquired;
    bool m_hwKeysEnabled;
    bool m_hwKeysActive;

    //! The current volume
    int m_volume;

    //! The maximum volume
    int m_maximumVolume;

    //! Stores audio warning acknowledgement state
    MDConfItem *m_audioWarning;

    //! The current safe volume
    int m_safeVolume;

    //! Call active status
    bool m_callActive;

    bool m_upPressed;
    bool m_downPressed;

    int m_mediaState;

#ifdef UNIT_TEST
    friend class Ut_VolumeControl;
#endif
};

#endif // VOLUMECONTROL_H
