/*
 * Class A LoRaWAN sample application
 *
 * Copyright (c) 2020 Manivannan Sadhasivam <mani@kernel.org>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/device.h>
#include <zephyr/lorawan/lorawan.h>.h>
#include <zephyr/lorawan/lorawan.h>
#include <zephyr/zephyr.h>

#define DEFAULT_RADIO_NODE DT_ALIAS(lora0)
BUILD_ASSERT(DT_NODE_HAS_STATUS(DEFAULT_RADIO_NODE, okay),
             "No default LoRa radio specified in DT");
#define DEFAULT_RADIO DT_LABEL(DEFAULT_RADIO_NODE)

/* Customize based on network configuration */
// #define LORAWAN_DEV_EUI			{ 0x60, 0x81, 0xF9, 0xB1, 0x65, 0x50, 0x26, 0xB7 }
// #define LORAWAN_JOIN_EUI		{ 0x60, 0x81, 0xF9, 0x3B, 0xA8, 0xA6, 0xF0, 0xB6 }
// #define LORAWAN_APP_KEY			{ 0x73, 0x33, 0x78, 0x62, 0x5D, 0x89, 0xAC, 0x89, 0xEC, 0xDB, 0x61, 0xCD, 0x95, 0x33, 0x13, 0xED }

// Currently requires ChirpStack setup with LoRaWAN 1.0.4 / RP1.0.1
#define LORAWAN_DEV_EUI                                                        \
  { \
    0x8c, 0x26, 0x2e, 0x11, 0xf2, 0x61, 0xb7, 0xdd \
      }
#define LORAWAN_JOIN_EUI		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
#define LORAWAN_APP_KEY                                                        \
  {                                                                            \
0x18, 0xA4, 0x1C, 0xC8, 0x3D, 0x27, 0x36, 0x0E, 0x0D, 0x3E, 0x47, 0x53, 0xF5, 0x11, 0xBF, 0xF7 \
  }
#define LORAWAN_NWK_KEY                                                        \
  {                                                                            \
0x3D, 0xCE, 0x53, 0x04, 0x7D, 0xFD, 0x0C, 0xF5, 0x22, 0x47, 0x8A, 0xBD, 0xE3, 0x10, 0x75, 0x15 \
  }

#define DELAY K_MSEC(10000)

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(lorawan_class_a);

// Max Payload of DR_0 is 11, but "HelloWorld" seems exceeds it
char data[] =
    {
        'h', 'e', 'l', 'l', 'o',
    };

static void
dl_callback(uint8_t port, bool data_pending, int16_t rssi, int8_t snr,
            uint8_t len, const uint8_t *data) {
  LOG_INF("Port %d, Pending %d, RSSI %ddB, SNR %ddBm", port, data_pending, rssi, snr);
  if (data) {
    LOG_HEXDUMP_INF(data, len, "Payload: ");
  }
}

static void lorwan_datarate_changed(enum lorawan_datarate dr)
{
  uint8_t unused, max_size;

  lorawan_get_payload_sizes(&unused, &max_size);
  LOG_INF("New Datarate: DR_%d, Max Payload %d", dr, max_size);
}

void main(void)
{
  const struct device *lora_dev;
  struct lorawan_join_config join_cfg;
  uint8_t dev_eui[] = LORAWAN_DEV_EUI;
  uint8_t join_eui[] = LORAWAN_JOIN_EUI;
  uint8_t app_key[] = LORAWAN_APP_KEY;
  uint8_t nwk_key[] = LORAWAN_NWK_KEY;
  int ret;

  struct lorawan_downlink_cb downlink_cb = {
    .port = LW_RECV_PORT_ANY,
    .cb = dl_callback
  };

  lora_dev = device_get_binding(DEFAULT_RADIO);
  if (!lora_dev) {
    LOG_ERR("%s Device not found", DEFAULT_RADIO);
    return;
  }

  ret = lorawan_start();
  if (ret < 0) {
    LOG_ERR("lorawan_start failed: %d", ret);
    return;
  }

  lorawan_register_downlink_callback(&downlink_cb);
  lorawan_register_dr_changed_callback(lorwan_datarate_changed);

  join_cfg.mode = LORAWAN_ACT_OTAA;
  join_cfg.dev_eui = dev_eui;
  join_cfg.otaa.join_eui = join_eui;
  join_cfg.otaa.app_key = app_key;
  join_cfg.otaa.nwk_key = app_key;

  do {
    LOG_INF("Joining network over OTAA");
    ret = lorawan_join(&join_cfg);
    if (ret < 0) {
      LOG_ERR("lorawan_join_network failed: %d", ret);
      k_sleep(DELAY);
      continue;
    }
    break;
  }while(1);

  while (1) {
    LOG_INF("Sending data...");
    ret = lorawan_send(2, data, sizeof(data),
                       LORAWAN_MSG_CONFIRMED);

    /*
     * Note: The stack may return -EAGAIN if the provided data
     * length exceeds the maximum possible one for the region and
     * datarate. But since we are just sending the same data here,
     * we'll just continue.
     */
    if (ret == -EAGAIN) {
      LOG_ERR("lorawan_send failed: %d. Continuing...", ret);
      k_sleep(DELAY);
      continue;
    }

    if (ret < 0) {
      LOG_ERR("lorawan_send failed: %d", ret);
      k_sleep(DELAY);
      continue;
    }

    LOG_INF("Data sent!");
    k_sleep(DELAY);
  }
}
