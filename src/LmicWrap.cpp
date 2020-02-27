#include "LmicWrap.h"

//////////////////////////////////////////////////
// Configure / change these variables 
//////////////////////////////////////////////////

// change TX_INTERVAL to comply with the duty cycle 
const unsigned TX_INTERVAL = dutyCycle; // seconds

// public var
static osjob_t sendjob;


#ifdef ABP

  // Set ABP keys in config.h
  static const PROGMEM uint8_t NWKSKEY[16] = NWKS;
  static const PROGMEM uint8_t APPSKEY[16] = APPS;
  static const uint32_t DEVADDR = DEV;
  
  // not needed for ABP, but OTAA. Linker will complain.
  void os_getArtEui (uint8_t* buf) {  }
  void os_getDevEui (uint8_t* buf) {  }
  void os_getDevKey (uint8_t* buf) {  }
  
#endif

#ifdef OTAA

  static const uint8_t PROGMEM APPEUI[8]= APPE;
  void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8); }
  static const uint8_t PROGMEM DEVEUI[8]= DEV;
  void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8); }
  static const uint8_t PROGMEM APPKEY[16] = APPK;
  void os_getDevKey (u1_t* buf) { memcpy_P(buf, APPKEY, 16);}

#endif

// Change values in config.h
const lmic_pinmap lmic_pins = {
  .nss = pin_nss,
  .rxtx = pin_rxtx,
  .rst = pin_rst,
  .dio = pin_dio,
};

//////////////////////////////////////////////////
////////////////////////////////////////////////// 
//////////////////////////////////////////////////

LmicWrap::LmicWrap() { } 
LmicWrap::~LmicWrap() { } 

void LmicWrap::init() {
  Serial.println("Init inside");

  // LMIC init
  os_init();
  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();

#ifdef ABP

#ifdef PROGMEM
  // On AVR, these values are stored in flash and only copied to RAM
  // once. Copy them to a temporary buffer here, LMIC_setSession will
  // copy them into a buffer of its own again.
  uint8_t appskey[sizeof(APPSKEY)];
  uint8_t nwkskey[sizeof(NWKSKEY)];
  memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
  memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
  LMIC_setSession (0x1, DEVADDR, nwkskey, appskey);
#else
  // If not running an AVR with PROGMEM, just use the arrays directly
  LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);
#endif
  LMIC_setLinkCheckMode(0);

  // TTN uses SF9 for its RX2 window.
  LMIC.dn2Dr = DR_SF9;

  // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
  // Use DR_SF12 for maximal range, DR_SF7 for maximal bandwidth
  LMIC_setDrTxpow(DR_SF7, 14);

  //LMIC_setAdrMode(1);
  
#endif // END OF ABP
}

void onEvent (ev_t ev) {
  Serial.print(os_getTime());
  Serial.print(": ");
  switch (ev) {
    case EV_SCAN_TIMEOUT:
      Serial.println(F("EV_SCAN_TIMEOUT"));
      break;
    case EV_BEACON_FOUND:
      Serial.println(F("EV_BEACON_FOUND"));
      break;
    case EV_BEACON_MISSED:
      Serial.println(F("EV_BEACON_MISSED"));
      break;
    case EV_BEACON_TRACKED:
      Serial.println(F("EV_BEACON_TRACKED"));
      break;
    case EV_JOINING:
      Serial.println(F("EV_JOINING"));
      break;
    case EV_JOINED:
      Serial.println(F("EV_JOINED"));
      break;
    case EV_RFU1:
      Serial.println(F("EV_RFU1"));
      break;
    case EV_JOIN_FAILED:
      Serial.println(F("EV_JOIN_FAILED"));
      break;
    case EV_REJOIN_FAILED:
      Serial.println(F("EV_REJOIN_FAILED"));
      break;
    case EV_TXCOMPLETE:
      Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
      if (LMIC.txrxFlags & TXRX_ACK)
        Serial.println(F("Received ack"));
      if (LMIC.dataLen) {
        Serial.println(F("Received "));
        Serial.println(LMIC.dataLen);
        Serial.println(F(" bytes of payload"));
      }
      os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), LoRaLMICWrap::sendData);
      break;
    case EV_LOST_TSYNC:
      Serial.println(F("EV_LOST_TSYNC"));
      break;
    case EV_RESET:
      Serial.println(F("EV_RESET"));
      break;
    case EV_RXCOMPLETE:
      Serial.println(F("EV_RXCOMPLETE"));
      break;
    case EV_LINK_DEAD:
      Serial.println(F("EV_LINK_DEAD"));
      break;
    case EV_LINK_ALIVE:
      Serial.println(F("EV_LINK_ALIVE"));
      break;
    default:
      Serial.println(F("Unknown event"));
      break;
  }
}

void LmicWrap::prepData(osjob_t* j, unsigned char mydata[]) {
  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND) {
    Serial.println(F("OP_TXRXPEND, not sending"));
  }
  else {
    // Prepare upstream data transmission at the next possible time.
    LMIC_setTxData2(1, mydata, sizeof(mydata), 0);
    Serial.println(F("Packet queued"));
  }
}

void LmicWrap::startJoining() {
  LMIC_startJoining();
}

void LmicWrap::evJoined() {
  LMIC_setLinkCheckMode(0);
}

// Can use this function to send data processed from another class
void LmicWrap::sendData()
{
  prepData(&sendjob, LN.getData());
}
