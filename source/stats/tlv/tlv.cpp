#include "tlv.hpp"

////////// TLV STUFF//////////////

tlv_packet build(uint8_t type, const char* value) {
    tlv_packet packet;
    packet.type = type;
    packet.length = strlen(value) + 1; // +1 for null terminator
    packet.value.assign(value, value + packet.length);
    return packet;
}

void serialize_tlv(std::vector<uint8_t>& serialized, tlv_packet packet) {
    serialized.push_back(packet.type);
    serialized.push_back((uint8_t)(packet.length & 0xFF));
    serialized.push_back((uint8_t)((packet.length >> 8) & 0xFF));
    serialized.insert(serialized.end(), packet.value.begin(), packet.value.end());
}

void serialize_tlv_container(std::vector<uint8_t>& out, uint8_t type,
                                    const std::vector<uint8_t>& value)
{
    tlv_packet packet;
    packet.type   = type;
    packet.length = (uint16_t)value.size();
    packet.value  = value;
    serialize_tlv(out, packet);
}

const char* tlv_type_to_char(enum tlv_type type){
    switch (type)
    {
    case TLV_TYPE_MEM_TOTAL:
        return "TLV_TYPE_MEM_TOTAL";
    case TLV_TYPE_MEM_FREE:
        return "TLV_TYPE_MEM_FREE";
    case TLV_TYPE_MEM_AVAILABLE:
        return "TLV_TYPE_MEM_AVAILABLE";
    case TLV_TYPE_BUFFERS:
        return "TLV_TYPE_BUFFERS";
    case TLV_TYPE_CACHED:
        return "TLV_TYPE_CACHED";
    case TLV_TYPE_SWAP_CACHED:
        return "TLV_TYPE_SWAP_CACHED";
    case TLV_TYPE_ACTIVE:
        return "TLV_TYPE_ACTIVE";
    case TLV_TYPE_INACTIVE:
        return "TLV_TYPE_INACTIVE";
    case TLV_TYPE_ACTIVE_ANON:
        return "TLV_TYPE_ACTIVE_ANON";
    case TLV_TYPE_INACTIVE_ANON:
        return "TLV_TYPE_INACTIVE_ANON";
    case TLV_TYPE_ACTIVE_FILE:
        return "TLV_TYPE_ACTIVE_FILE";
    case TLV_TYPE_INACTIVE_FILE:
        return "TLV_TYPE_INACTIVE_FILE";
    case TLV_TYPE_UNEVICTABLE:
        return "TLV_TYPE_UNEVICTABLE";
    case TLV_TYPE_MLOCKED:
        return "TLV_TYPE_MLOCKED";
    case TLV_TYPE_HIGH_TOTAL:
        return "TLV_TYPE_HIGH_TOTAL";
    case TLV_TYPE_HIGH_FREE:
        return "TLV_TYPE_HIGH_FREE";
    case TLV_TYPE_LOW_TOTAL:
        return "TLV_TYPE_LOW_TOTAL";
    case TLV_TYPE_LOW_FREE:
        return "TLV_TYPE_LOW_FREE";
    case TLV_TYPE_SWAP_TOTAL:
        return "TLV_TYPE_SWAP_TOTAL";
    case TLV_TYPE_SWAP_FREE:
        return "TLV_TYPE_SWAP_FREE";
    case TLV_TYPE_DIRTY:
        return "TLV_TYPE_DIRTY";
    case TLV_TYPE_WRITEBACK:
        return "TLV_TYPE_WRITEBACK";
    case TLV_TYPE_ANON_PAGES:
        return "TLV_TYPE_ANON_PAGES";
    case TLV_TYPE_MAPPED:
        return "TLV_TYPE_MAPPED";
    case TLV_TYPE_SHMEM:
        return "TLV_TYPE_SHMEM";
    case TLV_TYPE_SLAB:
        return "TLV_TYPE_SLAB";
    case TLV_TYPE_S_RECLAIMABLE:
        return "TLV_TYPE_S_RECLAIMABLE";
    case TLV_TYPE_S_UNRECLAIM:
        return "TLV_TYPE_S_UNRECLAIM";
    case TLV_TYPE_KERNEL_STACK:
        return "TLV_TYPE_KERNEL_STACK";
    case TLV_TYPE_PAGE_TABLES:
        return "TLV_TYPE_PAGE_TABLES";
    case TLV_TYPE_NFS_UNSTABLE:
        return "TLV_TYPE_NFS_UNSTABLE";
    case TLV_TYPE_BOUNCE:
        return "TLV_TYPE_BOUNCE";
    case TLV_TYPE_WRITEBACK_TMP:
        return "TLV_TYPE_WRITEBACK_TMP";
    case TLV_TYPE_COMMIT_LIMIT:
        return "TLV_TYPE_COMMIT_LIMIT";
    case TLV_TYPE_COMMITTED_AS:
        return "TLV_TYPE_COMMITTED_AS";
    case TLV_TYPE_VMALLOC_TOTAL:
        return "TLV_TYPE_VMALLOC_TOTAL";
    case TLV_TYPE_VMALLOC_USED:
        return "TLV_TYPE_VMALLOC_USED";
    case TLV_TYPE_VMALLOC_CHUNK:
        return "TLV_TYPE_VMALLOC_CHUNK";

    case TLV_TYPE_WIFI_STATION_LIST:
        return "TLV_TYPE_WIFI_STATION_LIST";
    case TLV_TYPE_WIFI_VAP:
        return "TLV_TYPE_WIFI_VAP";
    case TLV_TYPE_WIFI_VAP_NAME:
        return "TLV_TYPE_WIFI_VAP_NAME";
    case TLV_TYPE_WIFI_STATION:
        return "TLV_TYPE_WIFI_STATION";

    case TLV_TYPE_WIFI_ADDR:
        return "TLV_TYPE_WIFI_ADDR";
    case TLV_TYPE_WIFI_AID:
        return "TLV_TYPE_WIFI_AID";
    case TLV_TYPE_WIFI_CHAN:
        return "TLV_TYPE_WIFI_CHAN";
    case TLV_TYPE_WIFI_TXRATE:
        return "TLV_TYPE_WIFI_TXRATE";
    case TLV_TYPE_WIFI_RXRATE:
        return "TLV_TYPE_WIFI_RXRATE";
    case TLV_TYPE_WIFI_RSSI:
        return "TLV_TYPE_WIFI_RSSI";
    case TLV_TYPE_WIFI_MIN_RSSI:
        return "TLV_TYPE_WIFI_MIN_RSSI";
    case TLV_TYPE_WIFI_MAX_RSSI:
        return "TLV_TYPE_WIFI_MAX_RSSI";
    case TLV_TYPE_WIFI_IDLE:
        return "TLV_TYPE_WIFI_IDLE";
    case TLV_TYPE_WIFI_TXSEQ:
        return "TLV_TYPE_WIFI_TXSEQ";
    case TLV_TYPE_WIFI_RXSEQ:
        return "TLV_TYPE_WIFI_RXSEQ";
    case TLV_TYPE_WIFI_CAPS:
        return "TLV_TYPE_WIFI_CAPS";
    case TLV_TYPE_WIFI_XCAPS:
        return "TLV_TYPE_WIFI_XCAPS";
    case TLV_TYPE_WIFI_ACAPS:
        return "TLV_TYPE_WIFI_ACAPS";
    case TLV_TYPE_WIFI_ERP:
        return "TLV_TYPE_WIFI_ERP";
    case TLV_TYPE_WIFI_STATE:
        return "TLV_TYPE_WIFI_STATE";
    case TLV_TYPE_WIFI_MAXRATE_DOT11:
        return "TLV_TYPE_WIFI_MAXRATE_DOT11";
    case TLV_TYPE_WIFI_HTCAPS:
        return "TLV_TYPE_WIFI_HTCAPS";
    case TLV_TYPE_WIFI_VHTCAPS:
        return "TLV_TYPE_WIFI_VHTCAPS";
    case TLV_TYPE_WIFI_ASSOC_TIME:
        return "TLV_TYPE_WIFI_ASSOC_TIME";
    case TLV_TYPE_WIFI_IES:
        return "TLV_TYPE_WIFI_IES";
    case TLV_TYPE_WIFI_MODE:
        return "TLV_TYPE_WIFI_MODE";
    case TLV_TYPE_WIFI_RXNSS:
        return "TLV_TYPE_WIFI_RXNSS";
    case TLV_TYPE_WIFI_TXNSS:
        return "TLV_TYPE_WIFI_TXNSS";
    case TLV_TYPE_WIFI_PSMODE:
        return "TLV_TYPE_WIFI_PSMODE";
    case TLV_TYPE_WIFI_MIN_TX_POWER:
        return "TLV_TYPE_WIFI_MIN_TX_POWER";
    case TLV_TYPE_WIFI_MAX_TX_POWER:
        return "TLV_TYPE_WIFI_MAX_TX_POWER";
    case TLV_TYPE_WIFI_HT_CAPABLE:
        return "TLV_TYPE_WIFI_HT_CAPABLE";
    case TLV_TYPE_WIFI_VHT_CAPABLE:
        return "TLV_TYPE_WIFI_VHT_CAPABLE";
    case TLV_TYPE_WIFI_MU_CAPABLE:
        return "TLV_TYPE_WIFI_MU_CAPABLE";
    case TLV_TYPE_WIFI_SNR:
        return "TLV_TYPE_WIFI_SNR";
    case TLV_TYPE_WIFI_OPERATING_BAND:
        return "TLV_TYPE_WIFI_OPERATING_BAND";
    case TLV_TYPE_WIFI_CURRENT_OPERATING_CLASS:
        return "TLV_TYPE_WIFI_CURRENT_OPERATING_CLASS";
    case TLV_TYPE_WIFI_SUPPORTED_OPERATING_CLASSES:
        return "TLV_TYPE_WIFI_SUPPORTED_OPERATING_CLASSES";
    case TLV_TYPE_WIFI_SUPPORTED_OPERATING_CLASSES_COUNT:
        return "TLV_TYPE_WIFI_SUPPORTED_OPERATING_CLASSES_COUNT";
    case TLV_TYPE_WIFI_SUPPORTED_RATES:
        return "TLV_TYPE_WIFI_SUPPORTED_RATES";
    case TLV_TYPE_WIFI_SUPPORTED_RATES_COUNT:
        return "TLV_TYPE_WIFI_SUPPORTED_RATES_COUNT";
    case TLV_TYPE_WIFI_MAX_STA_PHYMODE:
        return "TLV_TYPE_WIFI_MAX_STA_PHYMODE";

    // Stub TLV types for testing without actual Wi-Fi data
    // case TLV_TYPE_WIFI_STUB_VAP_COUNT:
    //     return "TLV_TYPE_WIFI_STUB_VAP_COUNT";
    // case TLV_TYPE_WIFI_STUB_VAP_INDEX:
    //     return "TLV_TYPE_WIFI_STUB_VAP_INDEX";
    // case TLV_TYPE_WIFI_STUB_CLIENTS_COUNT:
    //     return "TLV_TYPE_WIFI_STUB_CLIENTS_COUNT";
    // case TLV_TYPE_WIFI_STUB_CLIENT_INDEX:
    //     return "TLV_TYPE_WIFI_STUB_CLIENT_INDEX";
    // case TLV_TYPE_WIFI_STUB_CLIENT_VAL1:
    //     return "TLV_TYPE_WIFI_STUB_CLIENT_VAL1";
    // case TLV_TYPE_WIFI_STUB_CLIENT_VAL2:
    //     return "TLV_TYPE_WIFI_STUB_CLIENT_VAL2";
    // case TLV_TYPE_WIFI_STUB_CLIENT_VAL3:
    //     return "TLV_TYPE_WIFI_STUB_CLIENT_VAL3";
    default:
        return "TLV_TYPE_INVALID";
    }
}

void deserialize_tlv(const uint8_t *payload, int payloadlen, struct tlv_packet* serialized, int *packets_count) {
    int packet_n = 0;
    int i = 0;
    while (i + 3 <= payloadlen) {
        uint16_t len = (uint16_t)(payload[i + 1] | (payload[i + 2] << 8));
        if (i + 3 + len > payloadlen) break;
        serialized[packet_n].type   = payload[i];
        serialized[packet_n].length = len;
        serialized[packet_n].value.assign(payload + i + 3, payload + i + 3 + len);
        i += 3 + len;
        packet_n++;
    }
    *packets_count = packet_n;
}
////////// TLV STUFF END//////////////