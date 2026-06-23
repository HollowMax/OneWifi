#ifndef WIFI_QM_H
#define WIFI_QM_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wifi_app wifi_app_t;

int qm_init(wifi_app_t *app, unsigned int create_flag);
int qm_deinit(wifi_app_t *app);
int qm_event(wifi_app_t *app, wifi_event_t *event);

#ifdef __cplusplus
}
#endif

#endif // WIFI_QM_H
