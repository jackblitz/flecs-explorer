#pragma once

/**
 * @file panel_signals.h
 * @brief Signal handling, SIGWINCH resize detection, and emergency terminal
 * restoration.
 *
 * Typical usage:
 * @code
 *     PanelSignalsInstall();
 *     if (PanelSignalsReceivedResize()) {
 *         PanelSignalsClearResize();
 *         PanelManagerHandleResize(manager);
 *     }
 *     PanelSignalsRestore();
 * @endcode
 *
 * Internal Component:
 * Private to the panel subsystem (under src/panel/). Installs POSIX sigaction
 * handlers for SIGINT, SIGTERM, and SIGWINCH, recording atomic signals and
 * ensuring clean terminal state restoration on abnormal process exits.
 */

#include <stdbool.h>

#include "flecs_explorer/common/common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Installs POSIX signal handlers for SIGWINCH, SIGINT, and SIGTERM.
 *
 * @return APP_OK on success, APP_ERROR_SYSTEM on sigaction failure.
 */
AppResult PanelSignalsInstall(void);

/**
 * @brief Restores previous signal handlers for SIGWINCH, SIGINT, and SIGTERM.
 */
void PanelSignalsRestore(void);

/**
 * @brief Checks if a SIGWINCH window resize signal was received.
 *
 * @return true if SIGWINCH was received; false otherwise.
 */
bool PanelSignalsReceivedResize(void);

/**
 * @brief Clears the SIGWINCH window resize notification flag.
 */
void PanelSignalsClearResize(void);

/**
 * @brief Checks if a termination signal (SIGINT or SIGTERM) was received.
 *
 * @return true if termination signal was received; false otherwise.
 */
bool PanelSignalsReceivedQuit(void);

#ifdef __cplusplus
}
#endif
