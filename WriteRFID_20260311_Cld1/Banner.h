#pragma once
#include <Arduino.h>
#include <MFRC522.h>

constexpr uint16_t BANNER_MAX_PAYLOAD = 500;   // fits Uno RAM comfortably

bool bannerSerialIngest();
//void bannerIngestBegin();

void bannerLoadFromCard(const uint8_t* payload,
                        uint16_t len,
                        uint8_t entryCount);

bool bannerIsReady();


bool writeBannerToCard(MFRC522& rfid, MFRC522::MIFARE_Key& key);

void bannerReset();
void bannerShowSummary();

uint16_t crc16_ccitt(const uint8_t* data, uint16_t len, uint16_t crc = 0xFFFF);

bool bannerGetOneShot();
void bannerSetOneShot(bool on);

