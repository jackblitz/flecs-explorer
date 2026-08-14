#pragma once

/**
 * @file panel_types.h
 * @brief Common types, geometry primitives, and event structures for panels.
 *
 * Typical usage:
 * @code
 *     PanelConfig config = {
 *         .minWidth = 80,
 *         .minHeight = 24,
 *         .entityPanelRatio = 0.35,
 *         .enableMouse = true,
 *         .enableColors = true,
 *         .useDarculaTheme = true
 *     };
 * @endcode
 *
 * All structures in this header are plain data types, passed by value or
 * const pointer, and owned by the caller. Opaque handles (e.g. PanelManager)
 * are managed via dedicated lifecycle APIs.
 */

#include <stdbool.h>
#include <stdint.h>

#include "flecs_explorer/common/common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Identifiers for each panel managed in the framework.
 */
typedef enum PanelId {
    PANEL_HEADER = 0,    /**< Top panel: World overview & telemetry */
    PANEL_ENTITIES = 1,  /**< Left panel: ECS entity tree browser */
    PANEL_INSPECTOR = 2, /**< Right panel: Live component data inspector */
    PANEL_STATUS = 3,    /**< Bottom footer: Shortcuts & status message */
    PANEL_COUNT = 4
} PanelId;

/**
 * @brief Event classifications emitted by panel input processing.
 */
typedef enum PanelEventType {
    PANEL_EVENT_NONE = 0,
    PANEL_EVENT_KEY,
    PANEL_EVENT_MOUSE_CLICK,
    PANEL_EVENT_MOUSE_SCROLL,
    PANEL_EVENT_FOCUS_CHANGED,
    PANEL_EVENT_RESIZE,
    PANEL_EVENT_QUIT
} PanelEventType;

/**
 * @brief 2D bounding box rectangle for panel coordinate positioning.
 */
typedef struct PanelRect {
    int x;
    int y;
    int width;
    int height;
} PanelRect;

/**
 * @brief Multi-panel layout calculation result with bounding rectangles.
 */
typedef struct PanelLayout {
    int screenWidth;
    int screenHeight;
    bool isTooSmall;
    PanelRect headerRect;
    PanelRect entitiesRect;
    PanelRect inspectorRect;
    PanelRect statusRect;
} PanelLayout;

/**
 * @brief Unified event payload routed to active panels.
 */
typedef struct PanelEvent {
    PanelEventType type;
    int key;
    PanelId targetPanel;
    int localX;
    int localY;
    int scrollDelta;
} PanelEvent;

/**
 * @brief Configuration parameters for initializing the Panel Manager.
 */
typedef struct PanelConfig {
    int minWidth;
    int minHeight;
    double entityPanelRatio;
    bool enableMouse;
    bool enableColors;
    bool useDarculaTheme;
} PanelConfig;

/**
 * @brief Opaque handle representing the Panel Manager subsystem.
 */
typedef struct PanelManager PanelManager;

#ifdef __cplusplus
}
#endif
