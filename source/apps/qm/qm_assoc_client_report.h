#ifndef QM_ASSOC_CLIENT_REPORT_H
#define QM_ASSOC_CLIENT_REPORT_H

#include "wifi_webconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

int qm_mlo_clients_report_push_to_dpp(webconfig_subdoc_decoded_data_t *webconfig_dec_data);

#ifdef __cplusplus
}
#endif

#endif
