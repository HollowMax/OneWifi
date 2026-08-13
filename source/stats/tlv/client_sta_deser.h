#ifndef CLIENT_STA_DESER_H
#define CLIENT_STA_DESER_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define VAP_COUNT 3
#define WIFI_STATS_MAX_TEXT 64
#define WIFI_STATS_MAX_MODE 96
#define WIFI_STATS_MAX_SUPPORTED_CLASSES 32
#define WIFI_STATS_MAX_SUPPORTED_RATES 32

typedef struct {
    char addr[18];
    int  aid;
    int  chan;
    char txrate[WIFI_STATS_MAX_TEXT];
    char rxrate[WIFI_STATS_MAX_TEXT];
    int  rssi;
    int  min_rssi;
    int  max_rssi;
    int  idle;
    int  txseq;
    int  rxseq;
    char caps[WIFI_STATS_MAX_TEXT];
    char xcaps[WIFI_STATS_MAX_TEXT];
    char acaps[WIFI_STATS_MAX_TEXT];
    int  erp;
    char state[WIFI_STATS_MAX_TEXT];
    int  maxrate_dot11;
    char htcaps[WIFI_STATS_MAX_TEXT];
    char vhtcaps[WIFI_STATS_MAX_TEXT];
    char assoc_time[16];
    char ies[WIFI_STATS_MAX_TEXT];
    char mode[WIFI_STATS_MAX_MODE];
    int  rxnss;
    int  txnss;
    int  psmode;
    bool has_extended_info;
    int  min_tx_power;
    int  max_tx_power;
    int  ht_capable;
    int  vht_capable;
    int  mu_capable;
    int  snr;
    char operating_band[WIFI_STATS_MAX_TEXT];
    int  current_operating_class;
    int  supported_operating_classes[WIFI_STATS_MAX_SUPPORTED_CLASSES];
    int  supported_operating_classes_count;
    int  supported_rates[WIFI_STATS_MAX_SUPPORTED_RATES];
    int  supported_rates_count;
    char max_sta_phymode[WIFI_STATS_MAX_MODE];
} ClientStatInfo;

typedef struct {
    char            vap_name[WIFI_STATS_MAX_TEXT];
    int             clients_count;
    ClientStatInfo *clients;
} vapInfo;

typedef struct {
    vapInfo vaps[VAP_COUNT];
    size_t  vap_count;
} WifiStatInfo;

#ifdef __cplusplus
extern "C" {
#endif

int  wifi_stats_deserialize(const uint8_t *payload, int payloadlen, WifiStatInfo *stats);
void wifi_stats_free(WifiStatInfo *stats);

#ifdef __cplusplus
}
#endif

#endif /* CLIENT_STA_DESER_H */
