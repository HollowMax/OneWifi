#ifndef DPP_MLO_CLIENT_H_INCLUDED
#define DPP_MLO_CLIENT_H_INCLUDED

#include "ds.h"
#include "ds_dlist.h"

#include "dpp_types.h"

#define DPP_MLO_KEY_MGMT_LEN        32
#define DPP_MLO_CIPHER_LEN          32
#define DPP_MLO_OP_STANDARD_LEN     16
#define DPP_MLO_OP_BW_LEN           16
#define DPP_MLO_INTF_SOURCES_LEN    64

typedef struct
{
    radio_type_t                    band;
    mac_address_t                   link_address;
    bool                            association_link;
    char                            wpa_key_mgmt[DPP_MLO_KEY_MGMT_LEN];
    char                            pairwise_cipher[DPP_MLO_CIPHER_LEN];
    uint32_t                        rsn_capabilities;
    bool                            authentication_state;
    uint32_t                        last_data_downlink_rate;
    uint32_t                        last_data_uplink_rate;
    int32_t                         signal_strength;
    uint32_t                        retransmissions;
    bool                            active;
    char                            operating_standard[DPP_MLO_OP_STANDARD_LEN];
    char                            operating_channel_bandwidth[DPP_MLO_OP_BW_LEN];
    int32_t                         snr;
    char                            interference_sources[DPP_MLO_INTF_SOURCES_LEN];
    uint64_t                        data_frames_sent_ack;
    uint64_t                        data_frames_sent_no_ack;
    uint64_t                        bytes_sent;
    uint64_t                        bytes_received;
    int32_t                         rssi;
    int32_t                         min_rssi;
    int32_t                         max_rssi;
    uint32_t                        disassociations;
    uint32_t                        authentication_failures;
    uint32_t                        active_num_spatial_streams;
    uint64_t                        packets_sent;
    uint64_t                        packets_received;
    uint64_t                        errors_sent;
    uint64_t                        retrans_count;
    uint64_t                        failed_retrans_count;
    uint64_t                        retry_count;
    uint64_t                        multiple_retry_count;
    uint32_t                        max_uplink_rate;
    uint32_t                        max_downlink_rate;
    uint32_t                        last_connect_time;
    uint32_t                        ml_capabilities;
    uint32_t                        tid_link_map_negotiation;
    ds_dlist_node_t                 node;
} dpp_mlo_client_link_stats_t;

typedef struct
{
    mac_address_t                   mac_address;
    ds_dlist_t                      link_stats;  /* dpp_mlo_client_link_stats_t */
    ds_dlist_node_t                 node;
} dpp_mlo_client_record_t;

typedef struct
{
    uint64_t                        timestamp_ms;
    ds_dlist_t                      list;       /* dpp_mlo_client_record_t */
} dpp_mlo_client_report_data_t;

static inline dpp_mlo_client_link_stats_t *dpp_mlo_client_link_stats_alloc(void)
{
    dpp_mlo_client_link_stats_t *record = NULL;

    record = malloc(sizeof(dpp_mlo_client_link_stats_t));
    if (record) {
        memset(record, 0, sizeof(dpp_mlo_client_link_stats_t));
    }

    return record;
}

static inline void dpp_mlo_client_link_stats_free(dpp_mlo_client_link_stats_t *record)
{
    if (NULL != record) {
        free(record);
    }
}

static inline dpp_mlo_client_record_t *dpp_mlo_client_record_alloc(void)
{
    dpp_mlo_client_record_t *record = NULL;

    record = malloc(sizeof(dpp_mlo_client_record_t));
    if (record) {
        memset(record, 0, sizeof(dpp_mlo_client_record_t));
        ds_dlist_init(&record->link_stats, dpp_mlo_client_link_stats_t, node);
    }

    return record;
}

static inline void dpp_mlo_client_record_free(dpp_mlo_client_record_t *record)
{
    if (NULL != record) {
        ds_dlist_iter_t it;
        dpp_mlo_client_link_stats_t *ls;

        ds_dlist_foreach_iter(&record->link_stats, ls, it) {
            dpp_mlo_client_link_stats_free(ls);
        }

        free(record);
    }
}

#endif /* DPP_MLO_CLIENT_H_INCLUDED */
