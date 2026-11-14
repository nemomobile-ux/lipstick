# WindowPixmapItem test QML

Simple QML for testing `WindowPixmapItem` (preview of a compositor window, rounded corners, status).

**Important:** `WindowPixmapItem` reads windows from `LipstickCompositor::instance()`, so it must be loaded/used **inside a running Lipstick compositor** (e.g. glacier-home) to preview real client windows. Running this as a standalone app will not show previews from another compositor process.

## Usage in a compositor (nested on X11)

1. Start your Lipstick compositor nested on X11.
2. Start a Wayland client against its `WAYLAND_DISPLAY`.
3. Load `windowpixmapitem.qml` in the compositor context (or integrate its `WindowPixmapItem` usage). Select a window from the list to bind `windowId`.

The view shows `hasPixmap` and `windowSize` for the selected window and renders it with a rounded mask.

## Standalone run

```sh
QML2_IMPORT_PATH=.../build/plugin \
LD_LIBRARY_PATH=.../build/src:.../build/plugin \
./windowpixmapitem [-platform xcb] [windowId]
```

Useful only for UI/layout checks; `hasPixmap` will be false without a compositor instance providing windows.
