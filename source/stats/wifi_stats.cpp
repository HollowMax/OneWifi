#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "wifi_stats.hpp"
#include "tlv/tlv.hpp"

static void list_to_text(char *dst, size_t dst_size, const int *values, int count)
{
    int i;
    int written = 0;

    if (dst == NULL || dst_size == 0 || values == NULL || count <= 0) {
        if (dst != NULL && dst_size > 0) {
            dst[0] = '\0';
        }
        return;
    }

    dst[0] = '\0';
    for (i = 0; i < count; i++) {
        int n = snprintf(dst + written, dst_size - (size_t)written,
                         (i == 0) ? "%d" : " %d", values[i]);
        if (n < 0 || (size_t)n >= dst_size - (size_t)written) {
            break;
        }
        written += n;
    }
}

static char *trim_in_place(char *s)
{
    char *end;

    while (*s != '\0' && isspace((unsigned char)*s)) {
        s++;
    }

    if (*s == '\0') {
        return s;
    }

    end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }

    return s;
}

static void safe_copy(char *dst, size_t dst_size, const char *src)
{
    if (dst == NULL || dst_size == 0 || src == NULL) {
        return;
    }

    snprintf(dst, dst_size, "%s", src);
}

static int parse_yes_no(const char *v)
{
    if (v == NULL) {
        return 0;
    }

    return (strcmp(v, "Yes") == 0 || strcmp(v, "yes") == 0) ? 1 : 0;
}

static void parse_int_list(const char *value, int *out, int max_count, int *out_count)
{
    char work[512];
    char *tok;
    char *saveptr = NULL;

    if (value == NULL || out == NULL || out_count == NULL || max_count <= 0) {
        return;
    }

    *out_count = 0;
    snprintf(work, sizeof(work), "%s", value);

    tok = strtok_r(work, " \t\r\n", &saveptr);
    while (tok != NULL) {
        if (*out_count >= max_count) {
            break;
        }

        out[*out_count] = atoi(tok);
        (*out_count)++;
        tok = strtok_r(NULL, " \t\r\n", &saveptr);
    }
}

static int parse_station_row(char *line, ClientStatInfo *stats)
{
    unsigned int aid, erp_u, hour, min, sec;
    char txrate[16], rxrate[16];
    char caps[32], xcaps[32], acaps[32], state_str[32], htcaps[32], vhtcaps[32], mode[32];
    char *rest, *mode_ptr, *end = NULL;
    size_t ies_len;
    int consumed = 0;

    /* Format mirrors wlanconfig printf:
     * addr aid chan txrateM rxrateM rssi minrssi maxrssi idle txseq rxseq
     * caps xcaps acaps erp(hex) state(hex) maxrate htcaps vhtcaps HH:MM:SS */
    sscanf(line,
        "%17s %u %d %15s %15s %d %d %d %d %d %d"
        " %31s %31s %31s %x %31s %d %31s %31s"
        " %u:%u:%u %n",
        stats->addr, &aid, &stats->chan,
        txrate, rxrate,
        &stats->rssi, &stats->min_rssi, &stats->max_rssi,
        &stats->idle, &stats->txseq, &stats->rxseq,
        caps, xcaps, acaps,
        &erp_u, state_str, &stats->maxrate_dot11,
        htcaps, vhtcaps,
        &hour, &min, &sec, &consumed);


    stats->aid = (int)aid;
    stats->erp = (int)erp_u;
    safe_copy(stats->txrate,    sizeof(stats->txrate),    txrate);
    safe_copy(stats->rxrate,    sizeof(stats->rxrate),    rxrate);
    safe_copy(stats->caps,      sizeof(stats->caps),      caps);
    safe_copy(stats->xcaps,     sizeof(stats->xcaps),     xcaps);
    safe_copy(stats->acaps,     sizeof(stats->acaps),     acaps);
    safe_copy(stats->state,     sizeof(stats->state),     state_str);
    safe_copy(stats->htcaps,    sizeof(stats->htcaps),    htcaps);
    safe_copy(stats->vhtcaps,   sizeof(stats->vhtcaps),   vhtcaps);
    snprintf(stats->assoc_time, sizeof(stats->assoc_time),
             "%02u:%02u:%02u", hour, min, sec);

    /* Tail: [IEs...] IEEE80211_MODE_xxx [rxnss txnss psmode] */
    rest = line + consumed;
    mode_ptr = strstr(rest, "IEEE80211_MODE");
    if (mode_ptr != NULL) {
        ies_len = (size_t)(mode_ptr - rest);
        if (ies_len > 0 && ies_len < sizeof(stats->ies)) {
            memcpy(stats->ies, rest, ies_len);
            stats->ies[ies_len] = '\0';
            end = stats->ies + ies_len - 1;
            while (end > stats->ies && isspace((unsigned char)*end)) {
                *end-- = '\0';
            }
        }
        sscanf(mode_ptr, "%31s %d %d %d",
        mode,
        &stats->rxnss, 
        &stats->txnss, 
        &stats->psmode);

        safe_copy(stats->mode, sizeof(stats->mode), mode);
    } else if (rest[0] != '\0') {
        safe_copy(stats->ies, sizeof(stats->ies), rest);
    }

    return 0;
}

static void parse_key_value_line(char *line, ClientStatInfo *stats)
{
    char *sep;
    char *key;
    char *value;

    sep = strchr(line, ':');
    if (sep == NULL) {
        return;
    }

    *sep = '\0';
    key = trim_in_place(line);
    value = trim_in_place(sep + 1);

    if (strcmp(key, "Minimum Tx Power") == 0) {
        stats->min_tx_power = atoi(value);
    } else if (strcmp(key, "Maximum Tx Power") == 0) {
        stats->max_tx_power = atoi(value);
    } else if (strcmp(key, "HT Capability") == 0) {
        stats->ht_capable = parse_yes_no(value);
    } else if (strcmp(key, "VHT Capability") == 0) {
        stats->vht_capable = parse_yes_no(value);
    } else if (strcmp(key, "MU capable") == 0) {
        stats->mu_capable = parse_yes_no(value);
    } else if (strcmp(key, "SNR") == 0) {
        stats->snr = atoi(value);
    } else if (strcmp(key, "Operating band") == 0) {
        safe_copy(stats->operating_band, sizeof(stats->operating_band), value);
    } else if (strcmp(key, "Current Operating class") == 0) {
        stats->current_operating_class = atoi(value);
    } else if (strcmp(key, "Supported Operating classes") == 0) {
        parse_int_list(
            value,
            stats->supported_operating_classes,
            WIFI_STATS_MAX_SUPPORTED_CLASSES,
            &stats->supported_operating_classes_count);
    } else if (strcmp(key, "Supported Rates") == 0) {
        parse_int_list(
            value,
            stats->supported_rates,
            WIFI_STATS_MAX_SUPPORTED_RATES,
            &stats->supported_rates_count);
    } else if (strcmp(key, "Max STA phymode") == 0) {
        safe_copy(stats->max_sta_phymode, sizeof(stats->max_sta_phymode), value);
    } else {
        return; /* unrecognized key: not an extended field */
    }

    stats->has_extended_info = true;
}

/* A station row starts with a MAC address: xx:xx:xx:xx:xx:xx */
static bool is_station_row(const char *line)
{
    for (int i = 0; i < 17; i++) {
        char c = line[i];

        if ((i % 3) == 2) {
            if (c != ':') {
                return false;
            }
        } else if (!isxdigit((unsigned char)c)) {
            return false;
        }
    }

    return true;
}

/* Parses every station reported by "wlanconfig <ifname> list sta".
 * The output is a header line followed by, per station, one table row
 * and several "Key : Value" detail lines. Grows vap->clients as needed. */
static int per_vap_parse(vapInfo *vap, FILE *pipe)
{
    char line[2048];
    char *trimmed;
    ClientStatInfo *current = NULL;

    while (fgets(line, sizeof(line), pipe) != NULL) {
        trimmed = trim_in_place(line);
        if (trimmed[0] == '\0') {
            continue;
        }

        if (is_station_row(trimmed)) {
            ClientStatInfo *grown = (ClientStatInfo *)realloc(
                vap->clients,
                sizeof(ClientStatInfo) * (size_t)(vap->clients_count + 1));
            if (grown == NULL) {
                return vap->clients_count;
            }

            vap->clients = grown;
            current = &vap->clients[vap->clients_count];
            memset(current, 0, sizeof(*current));
            parse_station_row(trimmed, current);
            vap->clients_count++;
        } else if (current != NULL) {
            parse_key_value_line(trimmed, current);
        }
    }

    return vap->clients_count;
}

int get_wifi_stat_info(WifiStatInfo *stats)
{
    char command[256];
    bool sta_found = false;
    const char *interfaces[] = { "home-ap-24", "home-ap-l50", "home-ap-u50" };

    if (stats == NULL) {
        return -1;
    }

    memset(stats, 0, sizeof(WifiStatInfo));
    stats->vap_count = VAP_COUNT;

    for (size_t i = 0; i < VAP_COUNT; i++) {
        vapInfo *vap = &stats->vaps[i];

        safe_copy(vap->vap_name, sizeof(vap->vap_name), interfaces[i]);
        vap->clients = NULL;
        vap->clients_count = 0;

        snprintf(command, sizeof(command), "wlanconfig %s list sta", interfaces[i]);

        FILE *pipe = popen(command, "r");
        if (pipe == NULL) {
            continue;
        }

        if (per_vap_parse(vap, pipe) > 0) {
            sta_found = true;
        }

        pclose(pipe);
    }

    return sta_found ? 0 : -1;
}

static void serialize_client_station(std::vector<uint8_t>& station_value, ClientStatInfo& wifi_info)
{
    char buf[1024] = { 0 };

    serialize_tlv(station_value, build(TLV_TYPE_WIFI_ADDR, wifi_info.addr));

    snprintf(buf, sizeof(buf), "%d", wifi_info.aid);
    serialize_tlv(station_value, build(TLV_TYPE_WIFI_AID, buf));
    memset(buf, 0, sizeof(buf));

    snprintf(buf, sizeof(buf), "%d", wifi_info.chan);
    serialize_tlv(station_value, build(TLV_TYPE_WIFI_CHAN, buf));
    memset(buf, 0, sizeof(buf));

    serialize_tlv(station_value, build(TLV_TYPE_WIFI_TXRATE, wifi_info.txrate));
    serialize_tlv(station_value, build(TLV_TYPE_WIFI_RXRATE, wifi_info.rxrate));

    snprintf(buf, sizeof(buf), "%d", wifi_info.rssi);
    serialize_tlv(station_value, build(TLV_TYPE_WIFI_RSSI, buf));
    memset(buf, 0, sizeof(buf));

    snprintf(buf, sizeof(buf), "%d", wifi_info.min_rssi);
    serialize_tlv(station_value, build(TLV_TYPE_WIFI_MIN_RSSI, buf));
    memset(buf, 0, sizeof(buf));

    snprintf(buf, sizeof(buf), "%d", wifi_info.max_rssi);
    serialize_tlv(station_value, build(TLV_TYPE_WIFI_MAX_RSSI, buf));
    memset(buf, 0, sizeof(buf));

    snprintf(buf, sizeof(buf), "%d", wifi_info.idle);
    serialize_tlv(station_value, build(TLV_TYPE_WIFI_IDLE, buf));
    memset(buf, 0, sizeof(buf));

    snprintf(buf, sizeof(buf), "%d", wifi_info.txseq);
    serialize_tlv(station_value, build(TLV_TYPE_WIFI_TXSEQ, buf));
    memset(buf, 0, sizeof(buf));

    snprintf(buf, sizeof(buf), "%d", wifi_info.rxseq);
    serialize_tlv(station_value, build(TLV_TYPE_WIFI_RXSEQ, buf));
    memset(buf, 0, sizeof(buf));

    serialize_tlv(station_value, build(TLV_TYPE_WIFI_CAPS, wifi_info.caps));
    serialize_tlv(station_value, build(TLV_TYPE_WIFI_XCAPS, wifi_info.xcaps));
    serialize_tlv(station_value, build(TLV_TYPE_WIFI_ACAPS, wifi_info.acaps));

    snprintf(buf, sizeof(buf), "%d", wifi_info.erp);
    serialize_tlv(station_value, build(TLV_TYPE_WIFI_ERP, buf));
    memset(buf, 0, sizeof(buf));

    serialize_tlv(station_value, build(TLV_TYPE_WIFI_STATE, wifi_info.state));

    snprintf(buf, sizeof(buf), "%d", wifi_info.maxrate_dot11);
    serialize_tlv(station_value, build(TLV_TYPE_WIFI_MAXRATE_DOT11, buf));
    memset(buf, 0, sizeof(buf));

    serialize_tlv(station_value, build(TLV_TYPE_WIFI_HTCAPS, wifi_info.htcaps));
    serialize_tlv(station_value, build(TLV_TYPE_WIFI_VHTCAPS, wifi_info.vhtcaps));
    serialize_tlv(station_value, build(TLV_TYPE_WIFI_ASSOC_TIME, wifi_info.assoc_time));
    serialize_tlv(station_value, build(TLV_TYPE_WIFI_IES, wifi_info.ies));
    serialize_tlv(station_value, build(TLV_TYPE_WIFI_MODE, wifi_info.mode));

    snprintf(buf, sizeof(buf), "%d", wifi_info.rxnss);
    serialize_tlv(station_value, build(TLV_TYPE_WIFI_RXNSS, buf));
    memset(buf, 0, sizeof(buf));

    snprintf(buf, sizeof(buf), "%d", wifi_info.txnss);
    serialize_tlv(station_value, build(TLV_TYPE_WIFI_TXNSS, buf));
    memset(buf, 0, sizeof(buf));

    snprintf(buf, sizeof(buf), "%d", wifi_info.psmode);
    serialize_tlv(station_value, build(TLV_TYPE_WIFI_PSMODE, buf));
    memset(buf, 0, sizeof(buf));

    if (wifi_info.has_extended_info) {
        snprintf(buf, sizeof(buf), "%d", wifi_info.min_tx_power);
        serialize_tlv(station_value, build(TLV_TYPE_WIFI_MIN_TX_POWER, buf));
        memset(buf, 0, sizeof(buf));

        snprintf(buf, sizeof(buf), "%d", wifi_info.max_tx_power);
        serialize_tlv(station_value, build(TLV_TYPE_WIFI_MAX_TX_POWER, buf));
        memset(buf, 0, sizeof(buf));

        snprintf(buf, sizeof(buf), "%d", wifi_info.ht_capable);
        serialize_tlv(station_value, build(TLV_TYPE_WIFI_HT_CAPABLE, buf));
        memset(buf, 0, sizeof(buf));

        snprintf(buf, sizeof(buf), "%d", wifi_info.vht_capable);
        serialize_tlv(station_value, build(TLV_TYPE_WIFI_VHT_CAPABLE, buf));
        memset(buf, 0, sizeof(buf));

        snprintf(buf, sizeof(buf), "%d", wifi_info.mu_capable);
        serialize_tlv(station_value, build(TLV_TYPE_WIFI_MU_CAPABLE, buf));
        memset(buf, 0, sizeof(buf));

        snprintf(buf, sizeof(buf), "%d", wifi_info.snr);
        serialize_tlv(station_value, build(TLV_TYPE_WIFI_SNR, buf));
        memset(buf, 0, sizeof(buf));

        serialize_tlv(station_value, build(TLV_TYPE_WIFI_OPERATING_BAND, wifi_info.operating_band));

        snprintf(buf, sizeof(buf), "%d", wifi_info.current_operating_class);
        serialize_tlv(station_value, build(TLV_TYPE_WIFI_CURRENT_OPERATING_CLASS, buf));
        memset(buf, 0, sizeof(buf));

        list_to_text(
            buf,
            sizeof(buf),
            wifi_info.supported_operating_classes,
            wifi_info.supported_operating_classes_count);
        serialize_tlv(station_value, build(TLV_TYPE_WIFI_SUPPORTED_OPERATING_CLASSES, buf));
        memset(buf, 0, sizeof(buf));

        snprintf(buf, sizeof(buf), "%d", wifi_info.supported_operating_classes_count);
        serialize_tlv(station_value, build(TLV_TYPE_WIFI_SUPPORTED_OPERATING_CLASSES_COUNT, buf));
        memset(buf, 0, sizeof(buf));

        list_to_text(
            buf,
            sizeof(buf),
            wifi_info.supported_rates,
            wifi_info.supported_rates_count);
        serialize_tlv(station_value, build(TLV_TYPE_WIFI_SUPPORTED_RATES, buf));
        memset(buf, 0, sizeof(buf));

        snprintf(buf, sizeof(buf), "%d", wifi_info.supported_rates_count);
        serialize_tlv(station_value, build(TLV_TYPE_WIFI_SUPPORTED_RATES_COUNT, buf));
        memset(buf, 0, sizeof(buf));

        serialize_tlv(station_value, build(TLV_TYPE_WIFI_MAX_STA_PHYMODE, wifi_info.max_sta_phymode));
    }
}

int serialize_wifi_info_tlv(std::vector<uint8_t>& serialized, WifiStatInfo& wifi_stats) {
    std::vector<uint8_t> station_list;

    for (size_t i = 0; i < wifi_stats.vap_count && i < VAP_COUNT; i++) {
        vapInfo &vap = wifi_stats.vaps[i];
        std::vector<uint8_t> vap_value;

        serialize_tlv(vap_value, build(TLV_TYPE_WIFI_VAP_NAME, vap.vap_name));

        for (int j = 0; j < vap.clients_count; j++) {
            std::vector<uint8_t> station_value;

            serialize_client_station(station_value, vap.clients[j]);
            serialize_tlv_container(vap_value, TLV_TYPE_WIFI_STATION, station_value);
        }

        serialize_tlv_container(station_list, TLV_TYPE_WIFI_VAP, vap_value);
    }

    serialize_tlv_container(serialized, TLV_TYPE_WIFI_STATION_LIST, station_list);
    return 0;
}

// int serialize_wifi_info_tlv_stub(std::vector<uint8_t>& serialized, WifiStubStatInfo& wifi_stats)
// {
//     char buf[32] = { 0 };
//     std::vector<uint8_t> station_list;

//     for (size_t i = 0; i < wifi_stats.vap_count && i < VAP_COUNT; i++) {
//         StubVapInfo &vap = wifi_stats.vaps[i];
//         std::vector<uint8_t> vap_value;

//         serialize_tlv(vap_value, build(WIFI_TLV_VAP_NAME, vap.vap_name));

//         for (int j = 0; j < vap.clients_count; j++) {
//             StubClientStatInfo &client = vap.clients[j];
//             std::vector<uint8_t> station_value;

//             serialize_tlv(station_value,
//                 build(WIFI_TLV_STA_VAL1, client.val1 != NULL ? client.val1 : ""));

//             snprintf(buf, sizeof(buf), "%d", client.val2);
//             serialize_tlv(station_value, build(WIFI_TLV_STA_VAL2, buf));
//             memset(buf, 0, sizeof(buf));

//             snprintf(buf, sizeof(buf), "%d", client.val3);
//             serialize_tlv(station_value, build(WIFI_TLV_STA_VAL3, buf));
//             memset(buf, 0, sizeof(buf));

//             serialize_tlv_container(vap_value, WIFI_TLV_STATION, station_value);
//         }

//         serialize_tlv_container(station_list, WIFI_TLV_VAP, vap_value);
//     }

//     serialize_tlv_container(serialized, WIFI_TLV_STATION_LIST, station_list);
//     return 0;
//

// int serialize_wifi_info_tlv_stub(std::vector<uint8_t>& serialized, WifiStubStatInfo& wifi_stats)
// {
//     char buf[32] = { 0 };
//     std::vector<uint8_t> station_list;

//     for (size_t i = 0; i < wifi_stats.vap_count && i < VAP_COUNT; i++) {
//         StubVapInfo &vap = wifi_stats.vaps[i];
//         std::vector<uint8_t> vap_value;

//         serialize_tlv(vap_value, build(WIFI_TLV_VAP_NAME, vap.vap_name));

//         for (int j = 0; j < vap.clients_count; j++) {
//             StubClientStatInfo &client = vap.clients[j];
//             std::vector<uint8_t> station_value;

//             serialize_tlv(station_value,
//                 build(WIFI_TLV_STA_VAL1, client.val1 != NULL ? client.val1 : ""));

//             snprintf(buf, sizeof(buf), "%d", client.val2);
//             serialize_tlv(station_value, build(WIFI_TLV_STA_VAL2, buf));
//             memset(buf, 0, sizeof(buf));

//             snprintf(buf, sizeof(buf), "%d", client.val3);
//             serialize_tlv(station_value, build(WIFI_TLV_STA_VAL3, buf));
//             memset(buf, 0, sizeof(buf));

//             serialize_tlv_container(vap_value, WIFI_TLV_STATION, station_value);
//         }

//         serialize_tlv_container(station_list, WIFI_TLV_VAP, vap_value);
//     }

//     serialize_tlv_container(serialized, WIFI_TLV_STATION_LIST, station_list);
//     return 0;
// }