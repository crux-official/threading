#ifndef CRUX_THREADING_API_H
#define CRUX_THREADING_API_H

#include "kernel_types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// The threading concern's contract: the one struct of function pointers the
// threading driver fills and serves. This is the PRIMITIVE — the wrapper
// around the platform thread primitive, exactly the way a memory concern
// wraps malloc: the concern reports the hardware TRUTH and hands back an
// opaque handle the caller passes back; it makes NO scheduling decision.
//
// Dispatch: as every concern here, through the engine's registry channel —
// the driver's served ThreadingAPI, selected by the concern's stable
// dispatch id; the driver is selected per platform by the composition
// (threading_posix on linux/darwin, threading_win32 on windows,
// threading_web on the Emscripten target, threading_none the declared
// absence); the 1:1 concern bijection says this interface is driven by
// exactly one implementation in every cell that composes it.
//
// What is NOT here, deliberately: a pool, a batch barrier, a queue, a worker
// count policy. Those are a composition one layer UP — the thread_pool
// standalone extension (backlog 0037) builds them on this primitive. The
// primitive's only synchronization unit is the thread itself: Create it, and
// the happens-before is Join.
//
// The unit-of-work contract (what runs in the thread): the function is
// self-contained with respect to the caller's other threads — it reads and
// writes memory the caller owns, and after Thread_Join returns its writes
// are visible to the joining thread (the happens-before). If you
// Thread_Detach instead of joining, the function must additionally be
// self-contained in absolute terms (no result the caller reads, no teardown
// the caller sequences after it).

// Opaque (the platform thread id, wrapped): the caller tracks it and releases
// it via Join/Detach — never dereferenced, never freed directly. (The web
// inline-degrade sentinel is the one handle that is static + never freed: its
// Join/Detach are no-ops.)
typedef struct CruxThread CruxThread;

typedef struct ThreadingAPI {
  PackageVersion version;

  // Spawn a thread that runs fn(arg) to completion; return the opaque
  // handle, or NULL if the driver cannot spawn one (allocation, OS refusal).
  // The handle is the driver's — the app tracks it and passes it back to
  // Join/Detach; it is never dereferenced.
  //
  // The one NAMED degradation: on an Emscripten page that is NOT cross-origin
  // isolated (no SharedArrayBuffer — a real thread cannot exist there,
  // pthread_create would fail EAGAIN), the driver runs fn(arg) INLINE on the
  // calling thread and returns an ALREADY-FINISHED handle, so a later Join is
  // a no-op. The app's fact that no real threads exist is Thread_Concurrency
  // returning 0, not a NULL here.
  CruxThread *(*Thread_Create)(void (*fn)(void *arg), void *arg);

  // Wait for the thread to finish — the happens-before (its writes are
  // visible on return) — and release the handle. The handle is invalid after
  // Join. NULL is a no-op. Joining an already-finished (inline-degraded)
  // handle returns immediately.
  void (*Thread_Join)(CruxThread *t);

  // Do not wait; the driver owns the thread's lifetime to completion. The
  // function must be self-contained in absolute terms (see the contract
  // above) — there is no happens-before with a detached thread. The handle is
  // invalid after Detach. NULL is a no-op.
  void (*Thread_Detach)(CruxThread *t);

  // The hardware's LOGICAL core count (always a true fact — no platform
  // estimates it): sysconf(_SC_NPROCESSORS_ONLN) on native,
  // navigator.hardwareConcurrency on web (emscripten_num_logical_cores), 0 for
  // threading_none. This is the "how many logical cores" truth the app may
  // configure from.
  int (*Thread_Count_Logical)(void);

  // The hardware's PHYSICAL core count. EXACT on native (Linux sysfs topology
  // dedup of (physical_package_id, core_id); Windows
  // GetLogicalProcessorInformationEx(RelationProcessorCore)). On web physical
  // is NOT a hard fact (no API) — the driver returns an HONEST best-effort
  // estimate (logical / 2, the "probably physical" SMT heuristic) and the
  // driver names it as an estimate; do not treat the web value as a fact. 0
  // for threading_none.
  int (*Thread_Count_Physical)(void);

  // The HARD CEILING on how many real threads can run CONCURRENTLY right now
  // — the fact a worker pool keys its size (and its serial degradation) off.
  // This is NOT the core count; it is the platform+environment's live
  // concurrency fact:
  //   native        : physical cores (∩ the process affinity mask) — the
  //                   one-per-core ceiling.
  //   web isolated  : the pre-run Web Worker pool size (the page's
  //                   SharedArrayBuffer fact, the PTHREAD_POOL_SIZE value).
  //   web NOT isolated : 0 — no real threads are possible; a pool keyed on 0
  //                   runs serial.
  //   threading_none : 0.
  int (*Thread_Concurrency)(void);
} ThreadingAPI;

#ifdef __cplusplus
}
#endif

#endif
