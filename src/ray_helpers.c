#ifndef EPHO_RAY_HELPERS_H
#define EPHO_RAY_HELPERS_H

#include <raylib.h>

Vector2 get_display_size();

#endif // EPHO_RAY_HELPERS_H

#ifdef EPHO_RAY_HELPERS_IMPLEMENTATION

Vector2 get_display_size()
{
    int cm;
    Vector2 size;

    InitWindow(1,1,"");
    cm = GetCurrentMonitor();
    size = (Vector2) { GetMonitorWidth(cm), GetMonitorHeight(cm)};
    CloseWindow();

    return size;
}

#endif // EPHO_RAY_HELPERS_IMPLEMENTATION
