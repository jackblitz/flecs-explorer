#define _POSIX_C_SOURCE 200809L

#include "panel/panel_signals.h"

#include <curses.h>
#include <signal.h>
#include <string.h>

static volatile sig_atomic_t gResizeReceived = 0;
static volatile sig_atomic_t gQuitReceived = 0;

static struct sigaction gOldSigInt;
static struct sigaction gOldSigTerm;
static struct sigaction gOldSigWinch;
static bool gSignalsInstalled = false;

/**
 * Signal handler callback intercepting terminal resize and termination signals.
 *
 * @param sig Signal number received.
 */
static void HandleSignal(int sig)
{
    if (sig == SIGWINCH) {
        gResizeReceived = 1;
    } else if (sig == SIGINT || sig == SIGTERM) {
        gQuitReceived = 1;
        if (!isendwin()) {
            endwin();
        }
    }
}

/**
 * Installs POSIX signal handlers for SIGWINCH, SIGINT, and SIGTERM.
 *
 * Saves existing signal actions for graceful restoration.
 *
 * @return APP_OK on success, APP_ERROR_SYSTEM on sigaction failure.
 */
AppResult PanelSignalsInstall(void)
{
    if (gSignalsInstalled) {
        return APP_OK;
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = HandleSignal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGWINCH, &sa, &gOldSigWinch) != 0) {
        return APP_ERROR_SYSTEM;
    }

    if (sigaction(SIGINT, &sa, &gOldSigInt) != 0) {
        sigaction(SIGWINCH, &gOldSigWinch, NULL);
        return APP_ERROR_SYSTEM;
    }

    if (sigaction(SIGTERM, &sa, &gOldSigTerm) != 0) {
        sigaction(SIGWINCH, &gOldSigWinch, NULL);
        sigaction(SIGINT, &gOldSigInt, NULL);
        return APP_ERROR_SYSTEM;
    }

    gSignalsInstalled = true;
    return APP_OK;
}

/**
 * Restores previous signal handlers for SIGWINCH, SIGINT, and SIGTERM.
 */
void PanelSignalsRestore(void)
{
    if (!gSignalsInstalled) {
        return;
    }

    sigaction(SIGWINCH, &gOldSigWinch, NULL);
    sigaction(SIGINT, &gOldSigInt, NULL);
    sigaction(SIGTERM, &gOldSigTerm, NULL);

    gSignalsInstalled = false;
    gResizeReceived = 0;
    gQuitReceived = 0;
}

/**
 * Checks if a SIGWINCH window resize signal was received.
 *
 * @return true if SIGWINCH occurred; false otherwise.
 */
bool PanelSignalsReceivedResize(void)
{
    return (gResizeReceived != 0);
}

/**
 * Clears the SIGWINCH window resize notification flag.
 */
void PanelSignalsClearResize(void)
{
    gResizeReceived = 0;
}

/**
 * Checks if a termination signal (SIGINT or SIGTERM) was received.
 *
 * @return true if SIGINT or SIGTERM occurred; false otherwise.
 */
bool PanelSignalsReceivedQuit(void)
{
    return (gQuitReceived != 0);
}
