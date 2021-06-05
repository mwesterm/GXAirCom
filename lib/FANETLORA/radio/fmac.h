/*
 * mac.h
 *
 *  Created on: 30 Sep 2016
 *      Author: sid
 */

#ifndef FANET_STACK_FMAC_H_
#define FANET_STACK_FMAC_H_

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <Arduino.h>
#include "LoRa.h"

//#include "../Legacy/Legacy.h"

/* Debug */
#define MAC_debug_mode				0
//#define MAC_debug_mode				100
#define RX_DEBUG 0


//define the pins used by the LoRa transceiver module
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 23
#define DIO0 26

//#define FREQUENCY868 868200012
#define FREQUENCY868 868200000
#define FREQUENCY915 916039978

//#define ManuId 0x07 // for the moment we use soft-Rf, cause then airwhere is showing it on the map
#define ManuId 0x08 //08 ... GxAircom



/*
 * Hard coded tx time assumption:
 * -SR7
 * -BW250
 * -CR8 (worst case)
 *
 * payload (byte) -> airtime (ms) -> airtime per byte payload (ms)
 * 0		9.43		0
 * 1		13.44		4.1
 * 2-5		17.54		4-1.63
 * 10		25.73		1.63
 * 64		87.17		1.2
 * 201		246.91		1.18
 * (number according to LoRa calculator)
 *
 * -> tx time assumption:
 * 15ms + 2*payload(bytes)
 * MAC_TX_MINHEADERTIME_MS + (blength * MAC_TX_TIMEPERBYTE_MS)
 */

/*
 * Timing defines
 * ONLY change if you know what you are doing. Can destroy the hole nearby network!
 */

#define MAC_SLOT_MS				20

#define MAC_TX_MINPREAMBLEHEADERTIME_MS		15
#define MAC_TX_TIMEPERBYTE_MS			2
#define MAC_TX_ACKTIMEOUT			1000
#define MAC_TX_RETRANSMISSION_TIME		1000
#define MAC_TX_RETRANSMISSION_RETRYS		3
#define MAC_TX_BACKOFF_EXP_MIN			7
#define MAC_TX_BACKOFF_EXP_MAX			12

#define MAC_FORWARD_MAX_RSSI_DBM		-90		//todo test
#define MAC_FORWARD_MIN_DB_BOOST		20
#define MAC_FORWARD_DELAY_MIN			100
#define MAC_FORWARD_DELAY_MAX			300

#define NEIGHBOR_MAX_TIMEOUT_MS			250000		//4min + 10sek

#define MAC_SYNCWORD				0xF1

/*
 * Number defines
 */
#define MAC_NEIGHBOR_SIZE			64
#define MAC_MAXNEIGHBORS_4_TRACKING_2HOP	5
#define MAC_CODING48_THRESHOLD			8

#define MAC_FIFO_SIZE				8
#define MAC_FRAME_LENGTH			254

#define ADDRESSTYPE_RANDOM 0
#define ADDRESSTYPE_ICAO 1
#define ADDRESSTYPE_FLARM 2
#define ADDRESSTYPE_OGN 3

// SEND FRAME RETURN VALUES
#define	TX_OK						0
#define	TX_TX_ONGOING					-1
#define	TX_RX_ONGOING					-2
#define	TX_FSK_ONGOING					-3
#define TX_ERROR					-100

#define LORA_TIME 5000
#define FSK_TIME 2000

#define LEGACY_RANGE 50
#define LEGACY_8682_BEGIN		400
#define LEGACY_8682_END			800
#define LEGACY_8684_BEGIN		900
#define LEGACY_8684_END			1200

#define MODE_LORA 1
#define MODE_FSK_8682 2
#define MODE_FSK_8684 3

#define RF_MODE_FANET_RX_TX 0
#define RF_MODE_FANET_RX_TX_LEG_TX 1
#define RF_MODE_FANET_RX_TX_LEG_RX_TX 2
#define RF_MODE_FANET_TX_LEG_RX_TX 3
#define RF_MODE_FANET_TX_LEG_RX 4
#define RF_MODE_LEG_RX_TX 5
#define RF_MODE_LEG_RX 6
#define RF_MODE_FANET_RX_TX_LEG_RX 7




//#include "main.h"
#include "lib/LinkedList2.h"
#include "lib/TimerObject.h"

#include "frame.h"

/* note: zero copy stack might be faster and more memory efficient, but who cares @ 9kBaud and 64Ks of ram... */

class NeighborNode
{
private:
	unsigned long last_seen;
public:
	const MacAddr addr;
	bool hasTracking;

	NeighborNode(MacAddr addr, bool tracking = false) : addr(addr), hasTracking(tracking) { last_seen = millis(); }
	void seen(void) { last_seen = millis(); }
	bool isAround(void) { return last_seen + NEIGHBOR_MAX_TIMEOUT_MS > millis(); }
};

class Fapp
{
public:
	Fapp() { }
	virtual ~Fapp() { }

	/* device -> air */
	virtual bool is_broadcast_ready(int num_neighbors) = 0;
	virtual void broadcast_successful(int type) = 0;
	virtual Frame *get_frame() = 0;
	virtual bool createLegacy(uint8_t *buffer);
	

	/* air -> device */
	virtual void handle_acked(bool ack, MacAddr &addr) = 0;
	virtual void handle_frame(Frame *frm) = 0;
};

class MacFifo
{
private:
	LinkedList2<Frame*> fifo;
public:
	/* not usable in async mode */
	Frame* get_nexttx();
	Frame* frame_in_list(Frame *frm);
	Frame* front();

	/* usable in async mode */
	bool remove_delete_acked_frame(MacAddr dest);
	bool remove_delete(Frame *frm);
	int add(Frame *frm);
	int size() { return fifo.size(); }
};

class FanetMac
{
private:
	TimerObject myTimer;
	MacFifo tx_fifo;
	MacFifo rx_fifo;
	LinkedList2<NeighborNode *> neighbors;
	Fapp *myApp = NULL;
	MacAddr _myAddr;

	int8_t _ss;
	int8_t _reset;

	unsigned long csma_next_tx = 0;
	int csma_backoff_exp = MAC_TX_BACKOFF_EXP_MIN;
	long setup_frequency;
	
	/* used for interrupt handler */
	uint8_t rx_frame[MAC_FRAME_LENGTH];
	int num_received = 0;	

	static void frameRxWrapper(int length);
	void frameReceived(int length);

	void ack(Frame* frm);

	static void stateWrapper();
	//static void setFlag(void);
	void handleIRQ();
	void handleTx();
	void handleTxLegacy();
	void handleRx();
	void switchMode(uint8_t mode,bool bStartReceive = true);
	uint8_t getAddressType(uint8_t manuId);
  //void coord2payload_absolut(float lat, float lon, uint8_t *buf);

	bool isNeighbor(MacAddr addr);
	//int16_t checkLegacyPaket(void *legacy_pkt, ufo_t *this_aircraft, ufo_t *fop);
	uint8_t _RfMode;	
	uint8_t _actMode = 0;
	//bool _fskMode;
	LoRaClass radio;

	void sendUdpData(const uint8_t *buffer,int len);
	uint32_t long legacy_next_tx = 0;
	uint8_t LegacyBuffer [26];
	uint32_t _ppsMillis = 0;
	uint8_t _ppsCount = 0;

public:

  bool doForward = true;
  float lat = 0;
  float lon = 0;
  float geoidAlt = 0;
  bool bPPS = false;
	bool bFanetTxEn = false;
	uint16_t txFntCount = 0;
	uint16_t rxFntCount = 0;
	uint16_t txLegCount = 0;
	uint16_t rxLegCount = 0;

	FanetMac() : myTimer(MAC_SLOT_MS, stateWrapper), myAddr(_myAddr) { }
	~FanetMac() { }

	bool begin(int8_t sck, int8_t miso, int8_t mosi, int8_t ss,int reset, int dio0,Fapp &app,long frequency,uint8_t level,uint8_t radioChip);
	void end();
	void handle() { myTimer.Update(); }

	bool txQueueDepleted(void) { return (tx_fifo.size() == 0); }
	bool txQueueHasFreeSlots(void){ return (tx_fifo.size() < MAC_FIFO_SIZE); }
	int transmit(Frame *frm) { return tx_fifo.add(frm); }

	uint16_t numNeighbors(void) { return neighbors.size(); }
	uint16_t numTrackingNeighbors(void);
	void setRfMode(uint8_t mode);
	void setPps(uint32_t *ppsMillis);
	/* Addr */
	const MacAddr &myAddr;
	bool setAddr(MacAddr addr);
	bool eraseAddr(void);
	MacAddr readAddr();	

};

extern FanetMac fmac;

#endif /* FANET_STACK_FMAC_H_ */
