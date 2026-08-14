#pragma once

/**
 * @file test_rest.h
 * @brief Unified REST subsystem test suite runner.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Runs all unit and integration tests across the REST subsystem:
 *        - HttpClient & dynamic buffer tests
 *        - FlecsWorldData models & deep-cloning tests
 *        - FlecsSession background polling & synchronization engine tests
 *        - End-to-end simulated Flecs payload integration scenarios
 *
 * @return 0 on success, non-zero on test assertion failure.
 */
int TestRestRun(void);

#ifdef __cplusplus
}
#endif
