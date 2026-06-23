#include <stdint.h>
#include "../../../lib/datapipeline/dppline.h"
#include "wifi_mgr.h"
#include "wifi_qm.h"
#include "qm_report.h"

int qm_init(wifi_app_t *app, unsigned int create_flag)
{
    int rc = RETURN_OK;
    if (app_init(app, create_flag) != 0) {
        return RETURN_ERR;
    }
    dpp_init();
    wifi_util_info_print(WIFI_QM, "%s:%d: Init QM app success\n", __func__, __LINE__);

    return rc;
}

static int handle_qm_webconfig_event(wifi_app_t *app, wifi_event_t *event)
{

    webconfig_subdoc_data_t *webconfig_data = NULL;

    webconfig_data = event->u.webconfig_data;
    if (webconfig_data == NULL) {
        wifi_util_dbg_print(WIFI_QM, "%s %d webconfig_data is NULL\n", __func__, __LINE__);
        return RETURN_ERR;
    }
    if (webconfig_data->type != webconfig_subdoc_type_associated_clients || webconfig_data->u.decoded.assoclist_notifier_type != assoclist_notifier_full) {
        return RETURN_ERR;
    }
    qm_reportm_report(&webconfig_data->u.decoded);

    return RETURN_OK;
}

int qm_event(wifi_app_t *app, wifi_event_t *event)
{
    switch (event->event_type) {
        case wifi_event_type_webconfig:
            handle_qm_webconfig_event(app, event);
            break;
        default:
        break;
    }
    return RETURN_OK;
}

int qm_deinit(wifi_app_t *app)
{
    return RETURN_OK;
}
