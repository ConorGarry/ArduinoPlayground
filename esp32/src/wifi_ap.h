#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <esp_netif.h>
#include <esp_idf_version.h>
#include "config.h"

// Three "show the portal" mechanisms, most reliable first:
//   1. DHCP option 114 (RFC 8910) — modern OSes read URL from lease, no DNS.
//      ESP-IDF 5.1+ only. Defeats Android Private DNS.
//   2. Wildcard DNS + HTTP 302 on classic probe paths (web_routes.h). Older
//      devices; defeated by Private DNS.
//   3. SSID includes "192.168.4.1" as visual fallback (config.h).

namespace wifi_ap {

inline DNSServer dnsServer;

// LWIP stores the pointer, not a copy — must outlive the program.
static const char CAPTIVE_PORTAL_URI[] = "http://192.168.4.1/";

inline IPAddress apIp() {
  return IPAddress(AP_IP_OCTET_1, AP_IP_OCTET_2, AP_IP_OCTET_3, AP_IP_OCTET_4);
}

inline void setupCaptivePortalDhcpOption() {
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 1, 0)
  esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
  if (!netif) {
    Serial.println("[wifi_ap] SoftAP netif not found — DHCP option 114 skipped");
    return;
  }
  // Option only settable while DHCP server is stopped.
  esp_netif_dhcps_stop(netif);
  esp_err_t err = esp_netif_dhcps_option(
      netif,
      ESP_NETIF_OP_SET,
      ESP_NETIF_CAPTIVEPORTAL_URI,
      (void*) CAPTIVE_PORTAL_URI,
      (uint32_t) strlen(CAPTIVE_PORTAL_URI));
  esp_netif_dhcps_start(netif);
  if (err == ESP_OK) {
    Serial.print("[wifi_ap] DHCP option 114 captive portal URI = ");
    Serial.println(CAPTIVE_PORTAL_URI);
  } else {
    Serial.print("[wifi_ap] DHCP option 114 set failed: ");
    Serial.println(esp_err_to_name(err));
  }
#else
  Serial.println("[wifi_ap] ESP-IDF < 5.1 — DHCP option 114 (RFC 8910) not available");
#endif
}

inline void begin() {
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIp(), apIp(), IPAddress(255, 255, 255, 0));
  // max_connection defaults to 4 — the 5th phone can't even associate. Raise to
  // 8 (hw max is 10) so one phone drives while up to seven wait on the holding
  // screen. args: ssid, password, channel, hidden, max_connection.
  WiFi.softAP(AP_SSID, nullptr, 1, 0, 8);

  // Must be after softAP() so the netif exists.
  setupCaptivePortalDhcpOption();

  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", apIp());

  Serial.print("[wifi_ap] SSID=");
  Serial.print(AP_SSID);
  Serial.print(" IP=");
  Serial.println(apIp());
}

inline void poll() {
  dnsServer.processNextRequest();
}

}  // namespace wifi_ap
