#include "wifi_webconfig.h"
#include "ds_dlist.h"
#include "dpp_mlo_client.h"
#include "wifi_util.h"
#include "sm_utils.h"
#include "collection.h"

static int mlo_clients_create_dpp_link_stats(assoc_dev_data_t *assoc_dev_data, ds_dlist_t *dpp_mlo_client_link_stats_list, rdk_wifi_radio_t *wifi_radio)
{
    dpp_mlo_client_link_stats_t *dpp_mlo_client_link_stats = NULL;

    dpp_mlo_client_link_stats = dpp_mlo_client_link_stats_alloc();
    if (dpp_mlo_client_link_stats == NULL) {
        wifi_util_error_print(WIFI_QM, "%s:%d: Failed to allocate memory.\n", __func__, __LINE__);
        return RETURN_ERR;
    }

    dpp_mlo_client_link_stats->band = radio_index_to_dpp_radio_type(wifi_radio->feature.radio_index);
    dpp_mlo_client_link_stats->association_link = assoc_dev_data->association_link;
    dpp_mlo_client_link_stats->rsn_capabilities = assoc_dev_data->conn_security.rsn_capabilities;
    dpp_mlo_client_link_stats->authentication_state = assoc_dev_data->dev_stats.cli_AuthenticationState;
    dpp_mlo_client_link_stats->last_data_downlink_rate = assoc_dev_data->dev_stats.cli_LastDataDownlinkRate;
    dpp_mlo_client_link_stats->last_data_uplink_rate = assoc_dev_data->dev_stats.cli_LastDataUplinkRate;
    dpp_mlo_client_link_stats->signal_strength = assoc_dev_data->dev_stats.cli_SignalStrength;
    dpp_mlo_client_link_stats->retransmissions = assoc_dev_data->dev_stats.cli_Retransmissions;
    dpp_mlo_client_link_stats->active = assoc_dev_data->dev_stats.cli_Active;
    dpp_mlo_client_link_stats->snr = assoc_dev_data->dev_stats.cli_SNR;
    dpp_mlo_client_link_stats->data_frames_sent_ack = assoc_dev_data->dev_stats.cli_DataFramesSentAck;
    dpp_mlo_client_link_stats->data_frames_sent_no_ack = assoc_dev_data->dev_stats.cli_DataFramesSentNoAck;
    dpp_mlo_client_link_stats->bytes_sent = assoc_dev_data->dev_stats.cli_BytesSent;
    dpp_mlo_client_link_stats->bytes_received = assoc_dev_data->dev_stats.cli_BytesReceived;
    dpp_mlo_client_link_stats->rssi = assoc_dev_data->dev_stats.cli_RSSI;
    dpp_mlo_client_link_stats->min_rssi = assoc_dev_data->dev_stats.cli_MinRSSI;
    dpp_mlo_client_link_stats->max_rssi = assoc_dev_data->dev_stats.cli_MaxRSSI;
    dpp_mlo_client_link_stats->disassociations = assoc_dev_data->dev_stats.cli_Disassociations;
    dpp_mlo_client_link_stats->authentication_failures = assoc_dev_data->dev_stats.cli_AuthenticationFailures;
    dpp_mlo_client_link_stats->active_num_spatial_streams = assoc_dev_data->dev_stats.cli_activeNumSpatialStreams;
    dpp_mlo_client_link_stats->packets_sent = assoc_dev_data->dev_stats.cli_PacketsSent;
    dpp_mlo_client_link_stats->packets_received = assoc_dev_data->dev_stats.cli_PacketsReceived;
    dpp_mlo_client_link_stats->errors_sent = assoc_dev_data->dev_stats.cli_ErrorsSent;
    dpp_mlo_client_link_stats->retrans_count = assoc_dev_data->dev_stats.cli_RetransCount;
    dpp_mlo_client_link_stats->failed_retrans_count = assoc_dev_data->dev_stats.cli_FailedRetransCount;
    dpp_mlo_client_link_stats->retry_count = assoc_dev_data->dev_stats.cli_RetryCount;
    dpp_mlo_client_link_stats->multiple_retry_count = assoc_dev_data->dev_stats.cli_MultipleRetryCount;
    dpp_mlo_client_link_stats->max_uplink_rate = assoc_dev_data->dev_stats.cli_MaxUplinkRate;
    dpp_mlo_client_link_stats->max_downlink_rate = assoc_dev_data->dev_stats.cli_MaxDownlinkRate;
    dpp_mlo_client_link_stats->last_connect_time = assoc_dev_data->last_connect_time;
    dpp_mlo_client_link_stats->ml_capabilities = assoc_dev_data->dev_stats.cli_MLModeCapa;
    dpp_mlo_client_link_stats->tid_link_map_negotiation = assoc_dev_data->dev_stats.cli_TIDLinkMapNegotiation;

    memcpy(dpp_mlo_client_link_stats->link_address, assoc_dev_data->link_address, MAC_ADDRESS_LEN);
    strncpy(dpp_mlo_client_link_stats->interference_sources, assoc_dev_data->dev_stats.cli_InterferenceSources, DPP_MLO_INTF_SOURCES_LEN);
    strncpy(dpp_mlo_client_link_stats->operating_standard, assoc_dev_data->dev_stats.cli_OperatingStandard, DPP_MLO_OP_STANDARD_LEN);
    strncpy(dpp_mlo_client_link_stats->operating_channel_bandwidth, assoc_dev_data->dev_stats.cli_OperatingChannelBandwidth, DPP_MLO_OP_BW_LEN);
    strncpy(dpp_mlo_client_link_stats->wpa_key_mgmt, assoc_dev_data->conn_security.wpa_key_mgmt, DPP_MLO_KEY_MGMT_LEN);
    strncpy(dpp_mlo_client_link_stats->pairwise_cipher, assoc_dev_data->conn_security.pairwise_cipher, DPP_MLO_CIPHER_LEN);

    ds_dlist_insert_tail(dpp_mlo_client_link_stats_list, dpp_mlo_client_link_stats);

    return RETURN_OK;
}

static int mlo_clients_create_dpp_list(webconfig_subdoc_decoded_data_t *webconfig_dec_data, ds_dlist_t *dpp_mlo_clients_list)
{
    rdk_wifi_vap_map_t *vaps = NULL;
    hash_map_t *assoc_clients_map = NULL;
    assoc_dev_data_t *assoc_dev_data = NULL;
    dpp_mlo_client_record_t *dpp_mlo_client = NULL;
    ds_dlist_iter_t result_iter;

    if (dpp_mlo_clients_list == NULL) {
        wifi_util_error_print(WIFI_QM, "%s:%d: DS list is NULL.\n", __func__, __LINE__);
        return RETURN_ERR;

    }

    for (int radio_idx = 0; radio_idx < MAX_NUM_RADIOS; radio_idx++) {
        vaps = &webconfig_dec_data->radios[radio_idx].vaps;
        for (unsigned int vap_idx = 0; vap_idx < vaps->num_vaps; vap_idx++) {
            assoc_clients_map = vaps->rdk_vap_array[vap_idx].associated_devices_map;

            if (assoc_clients_map == NULL) {
                continue;
            }

            assoc_dev_data = hash_map_get_first(assoc_clients_map);

            while (assoc_dev_data != NULL) {
                if (assoc_dev_data->dev_stats.cli_MLDEnable) {
                    for (dpp_mlo_client = ds_dlist_ifirst(&result_iter, dpp_mlo_clients_list);
                            dpp_mlo_client != NULL;
                            dpp_mlo_client = ds_dlist_inext(&result_iter)) {
                        if (memcmp(dpp_mlo_client->mac_address, assoc_dev_data->dev_stats.cli_MACAddress, MAC_ADDRESS_LEN) == 0) {
                            break;
                        }
                    }

                    if (dpp_mlo_client != NULL) {
                        mlo_clients_create_dpp_link_stats(assoc_dev_data, &dpp_mlo_client->link_stats, &webconfig_dec_data->radios[radio_idx]);
                    } else {
                        dpp_mlo_client = dpp_mlo_client_record_alloc();

                        if (!dpp_mlo_client) {
                            wifi_util_error_print(WIFI_QM, "%s:%d: Failed to allocate memory.\n", __func__, __LINE__);
                            continue;
                        }

                        memcpy(dpp_mlo_client->mac_address, assoc_dev_data->dev_stats.cli_MACAddress, MAC_ADDRESS_LEN);
                        ds_dlist_init(&dpp_mlo_client->link_stats, dpp_mlo_client_link_stats_t, node);
                        mlo_clients_create_dpp_link_stats(assoc_dev_data, &dpp_mlo_client->link_stats, &webconfig_dec_data->radios[radio_idx]);
                        ds_dlist_insert_tail(dpp_mlo_clients_list, dpp_mlo_client);
                    }
                }
                assoc_dev_data = hash_map_get_next(assoc_clients_map, assoc_dev_data);
            }
        }
    }

    return RETURN_OK;
}

static int mlo_client_dpp_report_free(dpp_mlo_client_report_data_t *report)
{
    dpp_mlo_client_record_t *mlo_client = NULL;
    dpp_mlo_client_record_t *tmp_mlo_client = NULL;
    dpp_mlo_client_link_stats_t   *mlo_client_link_stats = NULL;
    dpp_mlo_client_link_stats_t   *tmp_mlo_client_link_stats = NULL;

    if (report == NULL) {
        wifi_util_error_print(WIFI_QM, "%s:%d: Report is NULL.\n", __func__, __LINE__);
        return RETURN_ERR;
    }

    ds_dlist_foreach_safe(&report->list, mlo_client, tmp_mlo_client) {
        ds_dlist_foreach_safe(&mlo_client->link_stats, mlo_client_link_stats, tmp_mlo_client_link_stats) {
            ds_dlist_remove(&mlo_client->link_stats, mlo_client_link_stats);
        }

        ds_dlist_remove(&report->list, mlo_client);
        dpp_mlo_client_record_free(mlo_client);
    }

    return RETURN_OK;
}

int qm_mlo_clients_report_push_to_dpp(webconfig_subdoc_decoded_data_t *webconfig_dec_data)
{
    int rc = RETURN_OK;

    dpp_mlo_client_report_data_t dpp_report = {
        .timestamp_ms = get_real_ms(),
    };

    ds_dlist_init(&dpp_report.list, dpp_mlo_client_record_t, node);
    rc = mlo_clients_create_dpp_list(webconfig_dec_data, &dpp_report.list);

    if (rc == RETURN_OK && !ds_dlist_is_empty(&dpp_report.list)) {
        dpp_put_mlo_client(&dpp_report);
        wifi_util_dbg_print(WIFI_QM, "%s:%d: mlo client report is pushed to dpp. timestamp_ms=%llu\n",
                              __func__, __LINE__, dpp_report.timestamp_ms);
    }

    mlo_client_dpp_report_free(&dpp_report);

    return RETURN_OK;
}
