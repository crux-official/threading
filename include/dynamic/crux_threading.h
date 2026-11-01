#ifndef CRUX_DYNAMIC_THREADING_H
#define CRUX_DYNAMIC_THREADING_H

#include "engine.h"
#include "kernel_types.h"
#include "threading_api.h"

#ifdef __cplusplus
extern "C" {
#endif

// The dynamic rendering of the threading PRIMITIVE: every call dispatches
// through the engine's registry channel — the threading driver's served
// ThreadingAPI, selected by the concern's stable dispatch id (DRIVER_THREADING,
// kernel_types.h). crux_engine_api is the module-global pointer defined by the
// install TU the DYNAMIC dialect compiles in and installed by the target's
// Construct (the same pattern as the engine's own dynamic rendering).
static inline CruxThread *Thread_Create(void (*func)(void *arg), void *arg) {
  return ((ThreadingAPI *)crux_engine_api->GetExtensionAPI(DRIVER_THREADING))->Thread_Create(func, arg);
}
static inline void Thread_Join(CruxThread *thread) {
  ((ThreadingAPI *)crux_engine_api->GetExtensionAPI(DRIVER_THREADING))->Thread_Join(thread);
}
static inline void Thread_Detach(CruxThread *thread) {
  ((ThreadingAPI *)crux_engine_api->GetExtensionAPI(DRIVER_THREADING))->Thread_Detach(thread);
}
static inline int Thread_Count_Logical(void) {
  return ((ThreadingAPI *)crux_engine_api->GetExtensionAPI(DRIVER_THREADING))->Thread_Count_Logical();
}
static inline int Thread_Count_Physical(void) {
  return ((ThreadingAPI *)crux_engine_api->GetExtensionAPI(DRIVER_THREADING))->Thread_Count_Physical();
}
static inline int Thread_Concurrency(void) {
  return ((ThreadingAPI *)crux_engine_api->GetExtensionAPI(DRIVER_THREADING))->Thread_Concurrency();
}

#ifdef __cplusplus
}
#endif

#endif
