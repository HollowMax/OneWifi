#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
typedef volatile bool atomic_bool;
#define MQTT_ATOMIC_STORE(v, value) __atomic_store_n((v), (value), __ATOMIC_RELAXED)
#define MQTT_ATOMIC_LOAD(v)         __atomic_load_n((v), __ATOMIC_RELAXED)
#include <unistd.h>
#include <mosquitto.h>
#include "wifi_util.h"
#include "wifi_mqtt.h"
#include "client_sta_handler.h"

#define MQTT_TLS_CA_CERT_FILE     "/tmp/mqtt_certs/ca.crt"
#define MQTT_TLS_CLIENT_CERT_FILE "/tmp/mqtt_certs/mwo.crt"
#define MQTT_TLS_CLIENT_KEY_FILE  "/tmp/mqtt_certs/mwo.key"

#define MQTT_WIFI_STATS_TOPIC "pod/AS7F70003F/wie"

#define MQTT_KEEPALIVE_TIME 30

struct mqtt_broker_conf {
    const char *ip;
    int port;
    const char *topic;
};

static struct {
    struct mosquitto *mosq;
    pthread_t         msg_polling_thread;
    atomic_bool       is_msg_polling_running;
} g_mqtt_client = { .mosq = NULL, .is_msg_polling_running = false };

static void mqtt_on_message_cb(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message) {
    if (message->topic != NULL)
    {
        wifi_util_info_print(WIFI_MON, "%s:%d message received on topic '%s' payload '%.*s'\n", __func__, __LINE__, message->topic, message->payloadlen, (char *)message->payload);
        client_sta_handle_message((const uint8_t *)message->payload, message->payloadlen);
    } else {
        wifi_util_error_print(WIFI_MON, "%s:%d message received with empty topic\n", __func__, __LINE__);
    }
}

static int mqtt_subscribe_topic(const char *topic) {
    int rc;

    wifi_util_info_print(WIFI_MON, "%s:%d subscribing to topic '%s'\n", __func__, __LINE__, topic);
    rc = mosquitto_subscribe(g_mqtt_client.mosq, NULL, topic, 0);
    if (rc != MOSQ_ERR_SUCCESS)
    {
        wifi_util_error_print(WIFI_MON, "%s:%d failed to subscribe to topic '%s': %s\n", __func__, __LINE__, topic, mosquitto_strerror(rc));
        return 1;
    }
    wifi_util_info_print(WIFI_MON, "%s:%d subscribed to topic '%s'\n", __func__, __LINE__, topic);
    return 0;
}

static int mqtt_broker_connect(void) {
    int rc = 0;
    struct mqtt_broker_conf mqtt_broker_conf = {
        .ip   = "mqtt.gw.broker",
        .port = 8883,
    };

    rc = mosquitto_tls_set(g_mqtt_client.mosq, MQTT_TLS_CA_CERT_FILE, NULL, MQTT_TLS_CLIENT_CERT_FILE, MQTT_TLS_CLIENT_KEY_FILE, NULL);
    if (rc != MOSQ_ERR_SUCCESS)
    {
        wifi_util_error_print(WIFI_MON, "%s:%d failed to configure TLS: %s\n", __func__, __LINE__, mosquitto_strerror(rc));
        return 1;
    }

    wifi_util_info_print(WIFI_MON, "%s:%d connecting to broker %s:%d\n", __func__, __LINE__, mqtt_broker_conf.ip, mqtt_broker_conf.port);
    rc = mosquitto_connect(g_mqtt_client.mosq, mqtt_broker_conf.ip, mqtt_broker_conf.port, MQTT_KEEPALIVE_TIME);
    if (rc != MOSQ_ERR_SUCCESS)
    {
        wifi_util_error_print(WIFI_MON, "%s:%d failed to connect to broker: %s\n", __func__, __LINE__, mosquitto_strerror(rc));
        return 1;
    }
    wifi_util_info_print(WIFI_MON, "%s:%d connected to broker %s:%d\n", __func__, __LINE__, mqtt_broker_conf.ip, mqtt_broker_conf.port);

    mqtt_subscribe_topic(MQTT_WIFI_STATS_TOPIC);

    mosquitto_message_callback_set(g_mqtt_client.mosq, mqtt_on_message_cb);

    return 0;
}

static int mqtt_broker_reconnect(void)
{
    int rc = 0;

    wifi_util_info_print(WIFI_MON, "%s:%d reconnecting to broker\n", __func__, __LINE__);
    rc = mosquitto_reconnect(g_mqtt_client.mosq);
    if (rc != MOSQ_ERR_SUCCESS)
    {
        wifi_util_error_print(WIFI_MON, "%s:%d failed to reconnect to broker: %s\n", __func__, __LINE__, mosquitto_strerror(rc));
        return 1;
    }
    wifi_util_info_print(WIFI_MON, "%s:%d reconnected to broker\n", __func__, __LINE__);

    mqtt_subscribe_topic(MQTT_WIFI_STATS_TOPIC);

    mosquitto_message_callback_set(g_mqtt_client.mosq, mqtt_on_message_cb);

    return 0;
}

static void *mqtt_poll_thread(void *arg)
{
    (void)arg;
    while (MQTT_ATOMIC_LOAD(&g_mqtt_client.is_msg_polling_running)) {
        int rc = mosquitto_loop(g_mqtt_client.mosq, 0, 1);
        if (rc == MOSQ_ERR_CONN_LOST || rc == MOSQ_ERR_NO_CONN)
        {
            wifi_util_error_print(WIFI_MON, "%s:%d connection lost (%s)\n", __func__, __LINE__, mosquitto_strerror(rc));
            mqtt_broker_reconnect();
        }
        sleep(1);
    }
    return NULL;
}

int mqtt_init(void) {
    wifi_util_info_print(WIFI_MON, "%s:%d initializing MQTT client\n", __func__, __LINE__);
    mosquitto_lib_init();

    g_mqtt_client.mosq = mosquitto_new(NULL, true, NULL);
    if (!g_mqtt_client.mosq)
    {
        wifi_util_error_print(WIFI_MON, "%s:%d failed to create mosquitto instance\n", __func__, __LINE__);
        return 1;
    }

    if (mqtt_broker_connect() != 0)
        return 1;

    MQTT_ATOMIC_STORE(&g_mqtt_client.is_msg_polling_running, true);
    pthread_create(&g_mqtt_client.msg_polling_thread, NULL, mqtt_poll_thread, NULL);

    return 0;
}

void mqtt_deinit(void) {
    MQTT_ATOMIC_STORE(&g_mqtt_client.is_msg_polling_running, false);
    pthread_join(g_mqtt_client.msg_polling_thread, NULL);
    if (g_mqtt_client.mosq)
        mosquitto_destroy(g_mqtt_client.mosq);
    mosquitto_lib_cleanup();
}
