#ifndef AIRBRAKE_APP_H
#define AIRBRAKE_APP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern uint8_t app_done;

void app_init(void);
void app_tick(void);
void app_finish(void);

#ifdef __cplusplus
}
#endif

#endif /* AIRBRAKE_APP_H */
