/*
 * Class A LoRaWAN sample application
 *
 * Copyright (c) 2020 Manivannan Sadhasivam <mani@kernel.org>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/device.h>
#include <zephyr/drivers/lora.h>
#include <zephyr/sys/util.h>
#include <zephyr/zephyr.h>

#define DEFAULT_RADIO_NODE DT_ALIAS(lora0)
BUILD_ASSERT(DT_NODE_HAS_STATUS(DEFAULT_RADIO_NODE, okay),
             "No default LoRa radio specified in DT");
#define DEFAULT_RADIO DT_LABEL(DEFAULT_RADIO_NODE)


#define DELAY K_MSEC(500)

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(lora_recv);

#define MAX_DATA_LEN 255

char data[MAX_DATA_LEN] = {0};
#define RF_FREQUENCY                                915000000 // Hz
// #define RF_FREQUENCY                                470000000 // Hz

void main(void)
{
  int ret = 0;
  const struct device *lora_dev = DEVICE_DT_GET(DEFAULT_RADIO_NODE);

  if (!device_is_ready(lora_dev)) {
    LOG_ERR("%s Device not ready", lora_dev->name);
    return;
  }

  struct lora_modem_config config = {
    .frequency = RF_FREQUENCY,
    .bandwidth = BW_125_KHZ,
    .datarate = SF_12,
    .coding_rate = CR_4_8,
    .preamble_len = 10,
    .tx_power = 0,
    .tx = false,
  };

  ret = lora_config(lora_dev, &config);
  if (ret < 0) {
    LOG_ERR("LoRa config failed");
    return ;
  }

  while (1) {
    int16_t rssi = 0;
    int8_t snr = 0;
    int len = 0;
    len = lora_recv(lora_dev, data, MAX_DATA_LEN, K_SECONDS(10), &rssi, &snr);

    if (len < 0) {
      LOG_ERR("failed to recv data");
    } else {

      LOG_INF("received data len: %d", len);
    }
    k_sleep(DELAY);
  }
}
