#include <stdbool.h>
#include <stdint.h>
#include <qm_conn.h>
#include "dppline.h"
#include "wifi_webconfig.h"
#include "wifi_util.h"
#include "qm_assoc_client_report.h"

static bool qm_mlo_clients_publish(long mlen, void *mbuf)
{
    qm_response_t res;
    bool ret;

    ret = qm_conn_send_stats(mbuf, mlen, &res);

    return ret;
}

static bool qm_report_send_to_qm_cb(void)
{
    uint32_t buf_len;
    static uint8_t qm_mlo_clients_buf[STATS_MQTT_BUF_SZ];

    if (dpp_get_queue_elements() <= 0) {
        return false;
    }

    wifi_util_dbg_print(WIFI_QM, "%s:%d Total %d elements queued for transmission.\n",__func__, __LINE__, dpp_get_queue_elements());

    if (!qm_conn_get_status(NULL)) {
        wifi_util_error_print(WIFI_QM, "%s:%d Cannot connect to QM (QM not running?)\n",__func__, __LINE__);
        return false;
    }

    while (dpp_get_queue_elements() > 0)
    {
        if (!dpp_get_report(qm_mlo_clients_buf, sizeof(qm_mlo_clients_buf), &buf_len))
        {
            wifi_util_error_print(WIFI_QM, "%s:%d DPP: Get report failed.\n",__func__, __LINE__);
            break;
        }

        if (buf_len <= 0)
        {
            continue;
        }

        wifi_util_dbg_print(WIFI_QM, "%s:%d buf_len = %d\n",__func__, __LINE__, buf_len);
        if (!qm_mlo_clients_publish(buf_len, qm_mlo_clients_buf))
        {
            wifi_util_error_print(WIFI_QM, "%s:%d Publish report failed.\n",__func__, __LINE__);
            break;
        }
    }

    return true;
}

bool qm_reportm_report(webconfig_subdoc_decoded_data_t *webconfig_dec_data)
{
    qm_mlo_clients_report_push_to_dpp(webconfig_dec_data);
    return qm_report_send_to_qm_cb();
}
