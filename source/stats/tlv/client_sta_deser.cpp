#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tlv.hpp"
#include "client_sta_deser.h"

static void safe_copy(char *dst, size_t dst_size, const char *src)
{
    if (dst == NULL || dst_size == 0 || src == NULL)
        return;
    snprintf(dst, dst_size, "%s", src);
}

static void parse_int_list(const char *value, int *out, int max_count, int *out_count)
{
    char  work[512];
    char *tok;
    char *saveptr = NULL;

    if (value == NULL || out == NULL || out_count == NULL || max_count <= 0)
        return;

    *out_count = 0;
    snprintf(work, sizeof(work), "%s", value);

    tok = strtok_r(work, " \t\r\n", &saveptr);
    while (tok != NULL) {
        if (*out_count >= max_count)
            break;
        out[(*out_count)++] = atoi(tok);
        tok = strtok_r(NULL, " \t\r\n", &saveptr);
    }
}

static void deserialize_client(const uint8_t *payload, int payloadlen, ClientStatInfo *client)
{
    tlv_packet packets[128];
    int count = 0;

    memset(client, 0, sizeof(*client));

    if (payload == NULL || payloadlen <= 0)
        return;

    deserialize_tlv(payload, payloadlen, packets, &count);

    for (int i = 0; i < count; i++) {
        const char *val = packets[i].value.empty()
                        ? ""
                        : (const char *)packets[i].value.data();

        switch (packets[i].type) {
        case TLV_TYPE_WIFI_ADDR:            safe_copy(client->addr,  sizeof(client->addr),  val); break;
        case TLV_TYPE_WIFI_AID:             client->aid  = atoi(val); break;
        case TLV_TYPE_WIFI_CHAN:            client->chan  = atoi(val); break;
        case TLV_TYPE_WIFI_TXRATE:          safe_copy(client->txrate, sizeof(client->txrate), val); break;
        case TLV_TYPE_WIFI_RXRATE:          safe_copy(client->rxrate, sizeof(client->rxrate), val); break;
        case TLV_TYPE_WIFI_RSSI:            client->rssi      = atoi(val); break;
        case TLV_TYPE_WIFI_MIN_RSSI:        client->min_rssi  = atoi(val); break;
        case TLV_TYPE_WIFI_MAX_RSSI:        client->max_rssi  = atoi(val); break;
        case TLV_TYPE_WIFI_IDLE:            client->idle      = atoi(val); break;
        case TLV_TYPE_WIFI_TXSEQ:           client->txseq     = atoi(val); break;
        case TLV_TYPE_WIFI_RXSEQ:           client->rxseq     = atoi(val); break;
        case TLV_TYPE_WIFI_CAPS:            safe_copy(client->caps,   sizeof(client->caps),   val); break;
        case TLV_TYPE_WIFI_XCAPS:           safe_copy(client->xcaps,  sizeof(client->xcaps),  val); break;
        case TLV_TYPE_WIFI_ACAPS:           safe_copy(client->acaps,  sizeof(client->acaps),  val); break;
        case TLV_TYPE_WIFI_ERP:             client->erp           = atoi(val); break;
        case TLV_TYPE_WIFI_STATE:           safe_copy(client->state,  sizeof(client->state),  val); break;
        case TLV_TYPE_WIFI_MAXRATE_DOT11:   client->maxrate_dot11  = atoi(val); break;
        case TLV_TYPE_WIFI_HTCAPS:          safe_copy(client->htcaps,  sizeof(client->htcaps),  val); break;
        case TLV_TYPE_WIFI_VHTCAPS:         safe_copy(client->vhtcaps, sizeof(client->vhtcaps), val); break;
        case TLV_TYPE_WIFI_ASSOC_TIME:      safe_copy(client->assoc_time, sizeof(client->assoc_time), val); break;
        case TLV_TYPE_WIFI_IES:             safe_copy(client->ies,  sizeof(client->ies),  val); break;
        case TLV_TYPE_WIFI_MODE:            safe_copy(client->mode, sizeof(client->mode), val); break;
        case TLV_TYPE_WIFI_RXNSS:           client->rxnss   = atoi(val); break;
        case TLV_TYPE_WIFI_TXNSS:           client->txnss   = atoi(val); break;
        case TLV_TYPE_WIFI_PSMODE:          client->psmode  = atoi(val); break;
        case TLV_TYPE_WIFI_MIN_TX_POWER:
            client->min_tx_power = atoi(val);
            client->has_extended_info = true;
            break;
        case TLV_TYPE_WIFI_MAX_TX_POWER:
            client->max_tx_power = atoi(val);
            client->has_extended_info = true;
            break;
        case TLV_TYPE_WIFI_HT_CAPABLE:
            client->ht_capable = atoi(val);
            client->has_extended_info = true;
            break;
        case TLV_TYPE_WIFI_VHT_CAPABLE:
            client->vht_capable = atoi(val);
            client->has_extended_info = true;
            break;
        case TLV_TYPE_WIFI_MU_CAPABLE:
            client->mu_capable = atoi(val);
            client->has_extended_info = true;
            break;
        case TLV_TYPE_WIFI_SNR:
            client->snr = atoi(val);
            client->has_extended_info = true;
            break;
        case TLV_TYPE_WIFI_OPERATING_BAND:
            safe_copy(client->operating_band, sizeof(client->operating_band), val);
            client->has_extended_info = true;
            break;
        case TLV_TYPE_WIFI_CURRENT_OPERATING_CLASS:
            client->current_operating_class = atoi(val);
            client->has_extended_info = true;
            break;
        case TLV_TYPE_WIFI_SUPPORTED_OPERATING_CLASSES:
            parse_int_list(val, client->supported_operating_classes,
                           WIFI_STATS_MAX_SUPPORTED_CLASSES,
                           &client->supported_operating_classes_count);
            client->has_extended_info = true;
            break;
        case TLV_TYPE_WIFI_SUPPORTED_RATES:
            parse_int_list(val, client->supported_rates,
                           WIFI_STATS_MAX_SUPPORTED_RATES,
                           &client->supported_rates_count);
            client->has_extended_info = true;
            break;
        case TLV_TYPE_WIFI_MAX_STA_PHYMODE:
            safe_copy(client->max_sta_phymode, sizeof(client->max_sta_phymode), val);
            client->has_extended_info = true;
            break;
        default:
            /* count fields are recomputed by parse_int_list; skip */
            break;
        }
    }
}

static void deserialize_vap(const uint8_t *payload, int payloadlen, vapInfo *vap)
{
    tlv_packet packets[128];
    int count = 0;

    if (payload == NULL || payloadlen <= 0)
        return;

    deserialize_tlv(payload, payloadlen, packets, &count);

    for (int i = 0; i < count; i++) {
        if (packets[i].type == TLV_TYPE_WIFI_VAP_NAME) {
            const char *val = packets[i].value.empty()
                            ? ""
                            : (const char *)packets[i].value.data();
            safe_copy(vap->vap_name, sizeof(vap->vap_name), val);
        } else if (packets[i].type == TLV_TYPE_WIFI_STATION) {
            ClientStatInfo *grown = (ClientStatInfo *)realloc(
                vap->clients,
                sizeof(ClientStatInfo) * (size_t)(vap->clients_count + 1));
            if (grown == NULL)
                return;
            vap->clients = grown;
            deserialize_client(packets[i].value.data(),
                               (int)packets[i].value.size(),
                               &vap->clients[vap->clients_count]);
            vap->clients_count++;
        }
    }
}

extern "C" int wifi_stats_deserialize(const uint8_t *payload, int payloadlen, WifiStatInfo *stats)
{
    tlv_packet packets[128];
    int count = 0;

    if (payload == NULL || payloadlen <= 0 || stats == NULL)
        return -1;

    memset(stats, 0, sizeof(*stats));
    deserialize_tlv(payload, payloadlen, packets, &count);

    for (int i = 0; i < count; i++) {
        if (packets[i].type != TLV_TYPE_WIFI_STATION_LIST)
            continue;

        tlv_packet vaps[128];
        int vap_count = 0;

        deserialize_tlv(packets[i].value.data(), (int)packets[i].value.size(), vaps, &vap_count);

        for (int j = 0; j < vap_count && stats->vap_count < VAP_COUNT; j++) {
            if (vaps[j].type != TLV_TYPE_WIFI_VAP)
                continue;

            vapInfo *vap = &stats->vaps[stats->vap_count];
            vap->clients = NULL;
            vap->clients_count = 0;
            deserialize_vap(vaps[j].value.data(), (int)vaps[j].value.size(), vap);
            stats->vap_count++;
        }
    }

    return 0;
}

extern "C" void wifi_stats_free(WifiStatInfo *stats)
{
    size_t i;
    if (stats == NULL)
        return;
    for (i = 0; i < stats->vap_count; i++) {
        free(stats->vaps[i].clients);
        stats->vaps[i].clients = NULL;
    }
}
