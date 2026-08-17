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
    wifi_util_info_print(WIFI_MON, "vap[%d] Client[%d]\n", vap_idx, client_idx);
    wifi_util_info_print(WIFI_MON, "    addr                        : %s\n", client->addr);
    wifi_util_info_print(WIFI_MON, "    aid                         : %d\n", client->aid);
    wifi_util_info_print(WIFI_MON, "    chan                        : %d\n", client->chan);
    wifi_util_info_print(WIFI_MON, "    txrate                      : %s\n", client->txrate);
    wifi_util_info_print(WIFI_MON, "    rxrate                      : %s\n", client->rxrate);
    wifi_util_info_print(WIFI_MON, "    rssi                        : %d\n", client->rssi);
    wifi_util_info_print(WIFI_MON, "    min_rssi                    : %d\n", client->min_rssi);
    wifi_util_info_print(WIFI_MON, "    max_rssi                    : %d\n", client->max_rssi);
    wifi_util_info_print(WIFI_MON, "    idle                        : %d\n", client->idle);
    wifi_util_info_print(WIFI_MON, "    txseq                       : %d\n", client->txseq);
    wifi_util_info_print(WIFI_MON, "    rxseq                       : %d\n", client->rxseq);
    wifi_util_info_print(WIFI_MON, "    caps                        : %s\n", client->caps);
    wifi_util_info_print(WIFI_MON, "    xcaps                       : %s\n", client->xcaps);
    wifi_util_info_print(WIFI_MON, "    acaps                       : %s\n", client->acaps);
    wifi_util_info_print(WIFI_MON, "    erp                         : %d\n", client->erp);
    wifi_util_info_print(WIFI_MON, "    state                       : %s\n", client->state);
    wifi_util_info_print(WIFI_MON, "    maxrate_dot11               : %d\n", client->maxrate_dot11);
    wifi_util_info_print(WIFI_MON, "    htcaps                      : %s\n", client->htcaps);
    wifi_util_info_print(WIFI_MON, "    vhtcaps                     : %s\n", client->vhtcaps);
    wifi_util_info_print(WIFI_MON, "    assoc_time                  : %s\n", client->assoc_time);
    wifi_util_info_print(WIFI_MON, "    ies                         : %s\n", client->ies);
    wifi_util_info_print(WIFI_MON, "    mode                        : %s\n", client->mode);
    wifi_util_info_print(WIFI_MON, "    rxnss                       : %d\n", client->rxnss);
    wifi_util_info_print(WIFI_MON, "    txnss                       : %d\n", client->txnss);
    wifi_util_info_print(WIFI_MON, "    psmode                      : %d\n", client->psmode);
    wifi_util_info_print(WIFI_MON, "    max_sta_phymode             : %s\n", client->max_sta_phymode);

    if (client->has_extended_info) {
        char op_classes[WIFI_STATS_MAX_SUPPORTED_CLASSES * 4];
        char rates[WIFI_STATS_MAX_SUPPORTED_RATES * 4];
        fmt_int_array(op_classes, sizeof(op_classes),
                      client->supported_operating_classes,
                      client->supported_operating_classes_count);
        fmt_int_array(rates, sizeof(rates),
                      client->supported_rates, client->supported_rates_count);

        wifi_util_info_print(WIFI_MON, "    snr                         : %d\n", client->snr);
        wifi_util_info_print(WIFI_MON, "    operating_band              : %s\n", client->operating_band);
        wifi_util_info_print(WIFI_MON, "    ht_capable                  : %d\n", client->ht_capable);
        wifi_util_info_print(WIFI_MON, "    vht_capable                 : %d\n", client->vht_capable);
        wifi_util_info_print(WIFI_MON, "    mu_capable                  : %d\n", client->mu_capable);
        wifi_util_info_print(WIFI_MON, "    min_tx_power                : %d\n", client->min_tx_power);
        wifi_util_info_print(WIFI_MON, "    max_tx_power                : %d\n", client->max_tx_power);
        wifi_util_info_print(WIFI_MON, "    current_operating_class     : %d\n", client->current_operating_class);
        wifi_util_info_print(WIFI_MON, "    supported_operating_classes : %s\n", op_classes);
        wifi_util_info_print(WIFI_MON, "    supported_operating_classes_count: %d\n", client->supported_operating_classes_count);
        wifi_util_info_print(WIFI_MON, "    supported_rates             : %s\n", rates);
        wifi_util_info_print(WIFI_MON, "    supported_rates_count       : %d\n", client->supported_rates_count);
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
