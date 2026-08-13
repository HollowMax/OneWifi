#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>
#include <mosquitto.h>
#include "wifi_util.h"
#include "wifi_mqtt.h"

#define MQTT_TLS_CA_CERT_FILE     "/tmp/mqtt_certs/ca.crt"
#define MQTT_TLS_CLIENT_CERT_FILE "/tmp/mqtt_certs/mwo.crt"
#define MQTT_TLS_CLIENT_KEY_FILE  "/tmp/mqtt_certs/mwo.key"

#define MQTT_WIFI_STATS_TOPIC "pod/wie"

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
        wifi_util_dbg_print(WIFI_MON, "%s:%d Received MY_MQTT message on topic %s payload %.*s\n", __func__, __LINE__, message->topic, message->payloadlen, message->payload);
    } else {
        wifi_util_dbg_print(WIFI_MON, "%s:%d Received MY_MQTT empty topic\n", __func__, __LINE__);
    }
}

static int mqtt_subscribe_topic(const char *topic) {
    int rc;

    wifi_util_dbg_print(WIFI_MON, "%s:%d Subscribing to MQTT topic: %s\n", __func__, __LINE__, topic);
    rc = mosquitto_subscribe(g_mqtt_client.mosq, NULL, topic, 0);
    if (rc != MOSQ_ERR_SUCCESS)
    {
        wifi_util_error_print(WIFI_MON, "%s:%d Failed to subscribe to MQTT topic %s: %s\n", __func__, __LINE__,  topic, mosquitto_strerror(rc));
        return 1;
    }
    return 0;
}

static int mqtt_broker_connect(void) {
    int rc = 0;
    struct mqtt_broker_conf mqtt_broker_conf = {
        .ip   = "mqtt.gw.broker",
        .port = 8883,
    };

    wifi_util_dbg_print(WIFI_MON, "%s:%d\n", __func__, __LINE__);

    mosquitto_tls_set(g_mqtt_client.mosq, MQTT_TLS_CA_CERT_FILE, NULL, MQTT_TLS_CLIENT_CERT_FILE, MQTT_TLS_CLIENT_KEY_FILE, NULL);

    wifi_util_dbg_print(WIFI_MON, "%s:%d Connecting to mqtt broker: ip: %s, port: %d\n", __func__, __LINE__, mqtt_broker_conf.ip, mqtt_broker_conf.port);
    rc = mosquitto_connect(g_mqtt_client.mosq, mqtt_broker_conf.ip, mqtt_broker_conf.port, MQTT_KEEPALIVE_TIME);
    if (rc != MOSQ_ERR_SUCCESS)
    {
        wifi_util_error_print(WIFI_MON, "%s:%d Failed to connect to MQTT broker: %s\n", __func__, __LINE__, mosquitto_strerror(rc));
        return 1;
    }

    mqtt_subscribe_topic(MQTT_WIFI_STATS_TOPIC);

    mosquitto_message_callback_set(g_mqtt_client.mosq, mqtt_on_message_cb);

    return 0;
}

static int mqtt_broker_reconnect(void)
{
    int rc = 0;

    wifi_util_dbg_print(WIFI_MON, "%s:%d Reconnecting to MQTT broker\n", __func__, __LINE__);
    rc = mosquitto_reconnect(g_mqtt_client.mosq);
    if (rc != MOSQ_ERR_SUCCESS)
    {
        wifi_util_error_print(WIFI_MON, "%s:%d Failed to reconnect to MQTT broker: %s\n", __func__, __LINE__, mosquitto_strerror(rc));
        return 1;
    }

    mqtt_subscribe_topic(MQTT_WIFI_STATS_TOPIC);

    mosquitto_message_callback_set(g_mqtt_client.mosq, mqtt_on_message_cb);

    return 0;
}

static void *mqtt_poll_thread(void *arg)
{
    (void)arg;
    while (g_mqtt_client.is_msg_polling_running) {
        int rc = mosquitto_loop(g_mqtt_client.mosq, 0, 1);
        if (rc == MOSQ_ERR_CONN_LOST || rc == MOSQ_ERR_NO_CONN)
            mqtt_broker_reconnect();
        sleep(1);
    }
    return NULL;
}

int mqtt_init(void) {
    wifi_util_dbg_print(WIFI_MON, "%s:%d\n", __func__, __LINE__);
    wifi_util_error_print(WIFI_MON, "%s:%d\n", __func__, __LINE__);
    mosquitto_lib_init();

    g_mqtt_client.mosq = mosquitto_new(NULL, true, NULL);
    if (!g_mqtt_client.mosq)
    {
        wifi_util_error_print(WIFI_MON, "%s:%d Failed to initialize Mosquitto library\n", __func__, __LINE__);
        return 1;
    }

    mqtt_broker_connect();

    g_mqtt_client.is_msg_polling_running = true;
    pthread_create(&g_mqtt_client.msg_polling_thread, NULL, mqtt_poll_thread, NULL);

    return 0;
}

void mqtt_deinit(void) {
    g_mqtt_client.is_msg_polling_running = false;
    pthread_join(g_mqtt_client.msg_polling_thread, NULL);
    if (g_mqtt_client.mosq)
        mosquitto_destroy(g_mqtt_client.mosq);
    mosquitto_lib_cleanup();
}
