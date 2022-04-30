/*
 * ble.h
 *
 */


#ifndef MAIN_BLE_H_
#define MAIN_BLE_H_

#include <esp_coexist.h>
/*
#include <BLECharacteristic.h>
#include <BLEServer.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
*/

#include <NimBLEDevice.h>
#include <string.h>

//#include <sstream>             // Part of C++ Standard library

extern struct statusData status;
extern void checkReceivedLine(char *ch_str);

NimBLECharacteristic *pCharacteristic;
NimBLEServer *pServer;
uint16_t maxMtu = 0xFFFF;


/*
const char *CHARACTERISTIC_UUID_DEVICENAME = "00002A00-0000-1000-8000-00805F9B34FB";
const char *CHARACTERISTIC_UUID_RXTX = "0000FFE1-0000-1000-8000-00805F9B34FB";
const char *CHARACTERISTIC_UUID_RXTX_DESCRIPTOR = "00002902-0000-1000-8000-00805F9B34FB";
const char *SERVICE_UUID = "0000FFE0-0000-1000-8000-00805F9B34FB";

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" 
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
*/

class MyServerCallbacks : public NimBLEServerCallbacks {

	void onConnect(NimBLEServer* pServer) {
  		//log_d("***************************** BLE CONNECTED *****************");
		status.bluetoothStat = 2; //we have a connected client
		NimBLEDevice::startAdvertising();
	};

	void onDisconnect(NimBLEServer* pServer) {
		//log_d("***************************** BLE DISCONNECTED *****************");
		status.bluetoothStat = 1; //client disconnected
		//delay(1000);
		//	pServer->
		NimBLEDevice::startAdvertising();
	}

};



class MyCallbacks : public NimBLECharacteristicCallbacks {

	void onWrite(NimBLECharacteristic *pCharacteristic) {
		static String sLine = "";
		//pCharacteristic->getData();
		std::string rxValue = pCharacteristic->getValue();
		int valueLength = rxValue.length();
		//log_i("received:%d",rxValue.length());
		if (valueLength > 0) {
			//char cstr[valueLength+1]; //+1 for 0-termination
			//memcpy(cstr, rxValue.data(), valueLength);
			//cstr[valueLength] = 0; //zero-termination !!
			//log_i("received:%s,%d",rxValue.c_str(),valueLength);
			//log_i("received:%s",rxValue.c_str());
			//checkReceivedLine((char *)rxValue.c_str());						
			sLine += rxValue.c_str();
			if (sLine.endsWith("\n")){
				checkReceivedLine((char *)sLine.c_str());
				sLine = "";
			}
			if (sLine.length() > 512){
				sLine = "";
			}
		}
	}

};

void BleRun(){
	static uint32_t tCheckMTU = millis();
	uint32_t tAct = millis();
	if (timeOver(tAct,tCheckMTU,5000)){
		//start checking min. MTU-Size
		tCheckMTU = tAct;
		size_t count = pServer->getConnectedCount();
		uint16_t actMtu = maxMtu;
		maxMtu = 0xFFFF;
		for (int i = 0;i < count;i++){
			uint16_t peerMTU = pServer->getPeerMTU(i) - 3;
			if (peerMTU < maxMtu) maxMtu = peerMTU; //set minMTU to peerMTU
		}
		if ((actMtu != maxMtu) && (maxMtu < 0xFFFF)) log_i("new mtu-size=%d",maxMtu);
	}
}

void BLESendChunks(char *buffer,int iLen)
{
	if ((status.bluetoothStat == 2) && (iLen > 0) && (maxMtu < 0xFFFF)){ //we have a connection
		int startOffset = 0;
		int iActLen = 0;
		//log_i("offs=%d,l=%d,%s",startOffset,iLen,buffer);
		for (int k = 0; k <= iLen;k++){					
			iActLen = k-startOffset+1;
			if (buffer[k+1] == 0){ //check next char is zero-termination
				pCharacteristic->sNotify.assign(&buffer[startOffset],iActLen);
				//log_i("sOffs=%d,l=%d,k=%d,%s",startOffset,iActLen,k,pCharacteristic->sNotify.c_str());
				pCharacteristic->notify();
				vTaskDelay(5);
				break;
			}else if (buffer[k] == '\n'){ //new-line --> send chunk		
				pCharacteristic->sNotify.assign(&buffer[startOffset],iActLen);
				//log_i("sOffs=%d,l=%d,k=%d,%s",startOffset,iActLen,k,pCharacteristic->sNotify.c_str());
				startOffset += iActLen;
				pCharacteristic->notify();
				vTaskDelay(5);
			}else if (iActLen >= (maxMtu-1)){ //maxMtu -1, because of zero-termination of String						
				pCharacteristic->sNotify.assign(&buffer[startOffset],iActLen);
				//log_i("sOffs=%d,l=%d,k=%d,%s",startOffset,iActLen,k,pCharacteristic->sNotify.c_str());
				startOffset += iActLen;
				pCharacteristic->notify();
				vTaskDelay(5);
			}
		}
	}
}

void BLESendChunks(String str)
{
	if (status.bluetoothStat == 2){ //we have a connection
		String substr;
		for (int k = 0; k < str.length(); k += _min(str.length(), 20)) {
			substr = str.substring(k, k + _min(str.length() - k, 20));
			//pCharacteristic->setValue(substr.c_str());
			pCharacteristic->sNotify = std::string(substr.c_str());
			pCharacteristic->notify();			
			vTaskDelay(5);
		}
	}else{
		str = "";
	}
	//vTaskDelay(20);
}

void NEMEA_Checksum(String *sentence)
{

	char chksum[3];
	const char *n = (*sentence).c_str() + 1; // Plus one, skip '$'
	uint8_t chk = 0;
	/* While current char isn't '*' or sentence ending (newline) */
	while ('*' != *n && '\n' != *n) {

		chk ^= (uint8_t)*n;
		n++;
	}

	//convert chk to hexadecimal characters and add to sentence
	sprintf(chksum, "%02X\n", chk);
	(*sentence).concat(chksum);


}


void start_ble (String bleId)
{
	esp_coex_preference_set(ESP_COEX_PREFER_BT);
  NimBLEDevice::init(bleId.c_str());
	NimBLEDevice::setMTU(256); //set MTU-Size to 256 Byte
	pServer = NimBLEDevice::createServer();
	pServer->setCallbacks(new MyServerCallbacks());
	NimBLEService *pService = pServer->createService(NimBLEUUID((uint16_t)0xFFE0));
	// Create a BLE Characteristic
	pCharacteristic = pService->createCharacteristic(NimBLEUUID((uint16_t)0xFFE1),
		NIMBLE_PROPERTY::NOTIFY| NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
	);
	pCharacteristic->setCallbacks(new MyCallbacks());
	log_i("Starting BLE");
	// Start the service
	pService->start();
	pServer->getAdvertising()->addServiceUUID(NimBLEUUID((uint16_t)0xFFE0));
	// Start advertising
	pServer->getAdvertising()->start();

	log_i("Waiting a client connection to notify...");

}

void stop_ble ()
{
	NimBLEDevice::deinit(true);
}

#endif
