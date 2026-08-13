#ifndef WIFI_STATS_HPP
#define WIFI_STATS_HPP

#include <vector>
#include "wifi_deser.h"

int get_wifi_stat_info(WifiStatInfo *stats);
int serialize_wifi_info_tlv(std::vector<uint8_t>& serialized, WifiStatInfo& wifi_stats);

#endif // WIFI_STATS_HPP
