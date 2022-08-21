#ifndef __ENUMS_H__
#define __ENUMS_H__

/* regions */
enum eCountry {
    AUTO = 0,
    EU = 1,
};

/* Boards */
enum eBoard {
    T_BEAM = 0,
    HELTEC_LORA = 1,
    T_BEAM_V07 = 2,
    TTGO_TSIM_7000 = 4,
    T_BEAM_SX1262 = 5,
    TTGO_TCALL_800 = 6,
    HELTEC_WIRELESS_STICK_LITE = 7,
    UNKNOWN = 255
};

/* aneometer */
enum eAneometer {
    DAVIS = 0,
    TX20 = 1
};

/* Mode of operation */
enum eMode{
  AIR_MODULE = 0,
  GROUND_STATION = 1,
  FANET_INTERFACE = 2,
  DEVELOPER = 100
};

/* output-mode */
enum eOutput{
  oSERIAL = 0,
  oUDP = 1,
  oBLUETOOTH = 2,
  oBLE = 3
};

/* Displays */
enum eDisplay{
  NO_DISPLAY = 0,
  OLED0_96 = 1,
  EINK2_9 = 2,
  EINK2_9_V2 = 3
};

/* Wifi-Connect-Mode */
enum eWifiMode{
  CONNECT_NONE = 0,
  CONNECT_ONCE = 1,
  CONNECT_ALWAYS = 2
};

/* Screen Option */
enum eScreenOption{
  ALWAYS_ON = 0,
  ON_WHEN_TRAFFIC = 1,
  ALWAYS_OFF = 2,
  WEATHER_DATA = 3
};

/* Vario Output Option */
enum eOutputVario{
  OVARIO_NONE = 0,
  OVARIO_LK8EX1 = 1,
  OVARIO_LXPW = 2

};

/* ground-station power-mode */
enum eGsPower{
  GS_POWER_ALWAYS_ON = 0,
  GS_POWER_SAFE = 1,
  GS_POWER_BATT_LIFE = 2

};

/* Fanet-mode */
enum eFnMode{
  FN_GROUNT_AIR_TRACKING = 0,
  FN_AIR_TRACKING = 1,
};


/* state of modem */
enum eModemState{
  DISCONNECTED = 0,
  CONNECTING = 1,
  CONNECTED = 2
};

/* state of display */
enum eDisplayState{
  DISPLAY_STAT_OFF = 0,
  DISPLAY_STAT_ON = 1
};

/* state of display */
enum eRadarDispMode{
  CLOSEST = 0,
  LIST = 1,
  FRIENDS = 2
};




#endif
