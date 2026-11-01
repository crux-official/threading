#ifndef CRUX_STATIC_THREADING_H
#define CRUX_STATIC_THREADING_H

#include "kernel_types.h"
#include "threading_api.h"

#ifdef __cplusplus
extern "C" {
#endif

// The static rendering of the threading PRIMITIVE: plain externs, defined by
// the threading driver and linked into the binary. A static consumer pays
// zero indirection and cannot accidentally dispatch — the same shape as the
// other concerns' static renderings. (The "none" driver defines the same
// names as the honest absence — Thread_Create runs the function inline and
// returns an already-finished handle, the counts read 0 — so a static
// consumer of a threading_none cell compiles to the pure-logic path.)
extern CruxThread *Thread_Create(void (*func)(void *arg), void *arg);
extern void Thread_Join(CruxThread *thread);
extern void Thread_Detach(CruxThread *thread);
extern int Thread_Count_Logical(void);
extern int Thread_Count_Physical(void);
extern int Thread_Concurrency(void);

#ifdef __cplusplus
}
#endif

#endif
