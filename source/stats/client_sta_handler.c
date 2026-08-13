#include <stdlib.h>
#include <stdio.h>

#include "wifi_util.h"
#include "client_sta_deser.h"
#include "client_sta_handler.h"

/* format an int array as space-separated values into buf */
static void fmt_int_array(char *buf, size_t bufsz, const int *arr, int count)
{
    int off = 0;
    int i;
    for (i = 0; i < count && off < (int)bufsz - 1; i++) {
        int n = snprintf(buf + off, bufsz - (size_t)off, "%d ", arr[i]);
        if (n < 0) break;
        off += n;
    }
    if (off > 0) buf[off - 1] = '\0'; /* trim trailing space */
    else buf[0] = '\0';
}

static void log_client(const ClientStatInfo *client, int vap_idx, int client_idx)
{
    wifi_util_info_print(WIFI_MON,
        "vap[%d] client[%d]: addr=%s aid=%d chan=%d rssi=%d min_rssi=%d max_rssi=%d"
        " txrate=%s rxrate=%s txseq=%d rxseq=%d idle=%d erp=%d maxrate_dot11=%d"
        " rxnss=%d txnss=%d psmode=%d assoc=%s\n",
        vap_idx, client_idx,
        client->addr, client->aid, client->chan, client->rssi,
        client->min_rssi, client->max_rssi,
        client->txrate, client->rxrate, client->txseq, client->rxseq,
        client->idle, client->erp, client->maxrate_dot11,
        client->rxnss, client->txnss, client->psmode, client->assoc_time);

    wifi_util_info_print(WIFI_MON,
        "vap[%d] client[%d]: caps=%s xcaps=%s acaps=%s htcaps=%s vhtcaps=%s"
        " ies=%s mode=%s state=%s phymode=%s\n",
        vap_idx, client_idx,
        client->caps, client->xcaps, client->acaps,
        client->htcaps, client->vhtcaps,
        client->ies, client->mode, client->state, client->max_sta_phymode);

    if (client->has_extended_info) {
        char op_classes[WIFI_STATS_MAX_SUPPORTED_CLASSES * 4];
        char rates[WIFI_STATS_MAX_SUPPORTED_RATES * 4];
        fmt_int_array(op_classes, sizeof(op_classes),
                      client->supported_operating_classes,
                      client->supported_operating_classes_count);
        fmt_int_array(rates, sizeof(rates),
                      client->supported_rates, client->supported_rates_count);

        wifi_util_info_print(WIFI_MON,
            "vap[%d] client[%d]: snr=%d band=%s ht=%d vht=%d mu=%d"
            " min_tx_power=%d max_tx_power=%d op_class=%d"
            " op_classes=[%s] rates=[%s]\n",
            vap_idx, client_idx,
            client->snr, client->operating_band,
            client->ht_capable, client->vht_capable, client->mu_capable,
            client->min_tx_power, client->max_tx_power,
            client->current_operating_class,
            op_classes, rates);
    }
}

void client_sta_handle_message(const uint8_t *payload, int len)
{
    WifiStatInfo stats;
    size_t i;

    if (wifi_stats_deserialize(payload, len, &stats) != 0) {
        wifi_util_error_print(WIFI_MON, "%s:%d failed to deserialize VAP TLV payload\n",
                              __func__, __LINE__);
        return;
    }

    wifi_util_info_print(WIFI_MON, "%s:%d received VAP stats: %zu vap(s)\n",
                         __func__, __LINE__, stats.vap_count);

    for (i = 0; i < stats.vap_count; i++) {
        vapInfo *vap = &stats.vaps[i];
        int j;

        wifi_util_info_print(WIFI_MON, "%s:%d vap[%zu] '%s': %d client(s)\n",
                             __func__, __LINE__, i, vap->vap_name, vap->clients_count);

        for (j = 0; j < vap->clients_count; j++) {
            log_client(&vap->clients[j], (int)i, j);
        }
    }

    wifi_stats_free(&stats);
}
