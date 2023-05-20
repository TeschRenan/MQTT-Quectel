
/**
	@file mqtt_modem.cpp
	@author Renan Tesch

	MIT License

	Copyright (c) Copyright 2023 Renan Tesch
	GitHub https://github.com/TeschRenan

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/
#include "mqtt_quectel.h"

mqtt_quectel::mqtt_quectel(){

}

const char* mqtt_quectel::TAG = "[MQTT QUECTEL]";
int32_t mqtt_quectel::msgID = 1;


/**
 * @brief Starts the MQTT for operation.
 * @return [0]: Timeout or Wrong error .
 * @return [1]: Success resp.
**/
uint8_t mqtt_quectel::init(){

	uint8_t returnCode = 0;

	char temp[256] = {0};

	modemLib.flush();

	sprintf(temp,"%s\"%s\",%d%s","AT+QMTCFG=","will",0,"\r\n");

	//Configure Parameters of MQTT

	if(modemLib.atCmdWaitResponse(temp,"OK",NULL,-1, 5000, NULL, 0 ) != 0){


		ESP_LOGW(TAG, "Set Initial parameters MQTT Ok");
	

		returnCode = 1;

	}
	else{


		ESP_LOGW(TAG, "Set Initial parameters MQTT Fail");
	

		returnCode = 0;
	}

	return returnCode;

}

/**
 * @brief Open MQTT connection.
 * @param [host]: Is a IP or DNS from MQTT Broker.
 * @param [port]: Port Number from MQTT, usually is 1883.
 * @return [0]: Timeout or Wrong error .
 * @return [1]: Success resp.
**/
uint8_t mqtt_quectel::openConnection(char* host, int port){

	uint8_t returnCode = 0;

	char temp[128] = {0};

	modemLib.flush();

	sprintf(temp,"%s,\"%s\",%d%s","AT+QMTOPEN=0",host,port,"\r\n");

	//Open connection in Server 

	modemLib.write(temp);
	
	memset(temp,0,sizeof(temp));
	
	if(modemLib.getResponse(temp,"+QMTOPEN: 0,0",sizeof(temp),10000) != 0){


		ESP_LOGW(TAG, "Open connection MQTT Ok");
	

		returnCode = 1;

	}
	else{
		

		ESP_LOGE(TAG, "Open connection MQTT Fail");
	

		returnCode = 0;
	}
	

	return returnCode;

}

/**
 * @brief Initialize the MQTT connection.
 * @param [clientID]: Client ID name.
 * @param [user]: User from MQTT Auth.
 * @param [pass]: Pass from MQTT Auth.
 * @return [0]: Timeout or Wrong error .
 * @return [1]: Success resp.
**/
uint8_t mqtt_quectel::connect(char* clientID, char* user, char* pass){

	uint8_t returnCode = 0;

	char temp[128] = {0};

	modemLib.flush();

	//Connection in broker MQTT
	
	sprintf(temp,"%s,\"%s\",\"%s\",\"%s\"%s","AT+QMTCONN=0",clientID,user,pass,"\r\n");
	
	modemLib.write(temp);

	memset(temp,0,sizeof(temp));
	
	if(modemLib.getResponse(temp,"+QMTCONN: 0,0,0",sizeof(temp),10000) != 0){


		ESP_LOGW(TAG, "Connection in broker Ok");
	

		returnCode = 1;

	}
	else{
		

		ESP_LOGE(TAG, "Connection in broker Fail");
	

		returnCode = 0;
	}
	


	return returnCode;

}

/**
 * @brief Publish a message to MQTT.
 * @param [topic]: Topic to send message.
 * @param [content]: Content to send, Example "Hello World".
 * @param [qos]: QOS to send, 0 At most once, 1 At least once, 2 Exactly once
 * @return [0]: Timeout or Wrong error .
 * @return [1]: Success resp.
**/
uint8_t mqtt_quectel::publish(char* topic, char* content, int qos) {

	uint8_t returnCode = 0;
	char temp[256] = {0};

	if(qos == 0){
	
		sprintf(temp,"%s,%d,%d,%d,\"%s\"%s","AT+QMTPUB=0",0,0,0,topic,"\r\n");

	}
	else{

		sprintf(temp,"%s,%d,%d,%d,\"%s\"%s","AT+QMTPUB=0",msgID,qos,0,topic,"\r\n");

	}

	msgID++;

	if(modemLib.atCmdWaitResponse(temp,">",NULL,-1, 10000, NULL, 0 ) != 0){

		vTaskDelay(100 / portTICK_PERIOD_MS);

		modemLib.write(content);

		modemLib.write(0x1A);

		if (modemLib.verifyResponse("+QMTPUB:", 10000) != 0){
			
	
			ESP_LOGI(TAG,"PUBLISH PACKET SUCCESS");
		
			returnCode = 1;
		}	
		else {

	
			ESP_LOGI(TAG,"PUBLISH PACKET FAIL");
		
			returnCode = 0;
		}
	}

	return returnCode;

}

/**
 * @brief Subscribe a topic.
 * @param [topic]: Topic to subscribe.
 * @param [qos]: QOS to subscribe, 0 At most once, 1 At least once, 2 Exactly once
 * @return [0]: Timeout or Wrong error .
 * @return [1]: Success resp.
**/
uint8_t mqtt_quectel::subscribe(char* topic, int qos) {
	
	uint8_t returnCode = 0;
	char temp[256] = {0};
	char castQoS[8] ={0};

	modemLib.flush();

	sprintf(castQoS,"%i",qos);

	sprintf(temp,"%s,%d,\"%s\",%d%s","AT+QMTSUB=0",msgID,topic,qos,"\r\n");

	msgID++;

	modemLib.write(temp);

	modemLib.write(0x1A);

	if (modemLib.verifyResponse("OK", 5000) != 0){


		ESP_LOGI(TAG,"SUBSCRIBE PACKET SUCCESS");
	
		returnCode = 1;
	}	
	else {


		ESP_LOGI(TAG,"SUBSCRIBE PACKET FAIL");
	
		returnCode = 0;
	}

	return returnCode;

}

/**
 * @brief Unsubscribe a topic.
 * @param [topic]: Topic to unsubscribe.
 * @return [0]: Timeout or Wrong error .
 * @return [1]: Success resp.
**/
uint8_t mqtt_quectel::unsubscribe(char* topic) {
	
	uint8_t returnCode = 0;
	char temp[256] = {0};

	modemLib.flush();

	memset(temp,0,sizeof(temp));

	sprintf(temp,"%s,%d,\"%s\"%s","AT+QMTUNS=0",msgID,topic,"\r\n");

	msgID++;

	modemLib.write(temp);

	modemLib.write(0x1A);

	vTaskDelay(1000 / portTICK_PERIOD_MS);

	sprintf(temp,"%s,%d","+QMTUNS: 0",msgID);

	if (modemLib.verifyResponse(temp, 5000) != 0){
		
		ESP_LOGI(TAG,"UNSUBSCRIBE PACKET SUCCESS");
	
		returnCode = 1;
	}	
	else {

		ESP_LOGI(TAG,"UNSUBSCRIBE PACKET FAIL");
	
		returnCode = 0;
	}

	return returnCode;

}

/**
 * @brief Disconnect a MQTT Broker.
 * @return [0]: Timeout or Wrong error .
 * @return [1]: Success resp.
**/
uint8_t mqtt_quectel::disconnect() {
	
	uint8_t returnCode = 0;
	char temp[256] = {0};

	modemLib.flush();

	modemLib.write("AT+QMTDISC=0\r\n");	
	
	if(modemLib.getResponse(temp,"+QMTDISC: 0,0",sizeof(temp),10000) != 0){

		ESP_LOGW(TAG, "Disconnection in broker Ok");
	
		returnCode = 1;

	}
	else{
		
		ESP_LOGE(TAG, "Disconnection in broker Fail");
	
		returnCode = 0;
	}

	return returnCode;

}

/**
 * @brief Close connection to MQTT Broker.
 * @return [0]: Timeout or Wrong error .
 * @return [1]: Success resp.
**/
uint8_t  mqtt_quectel::closeConnection() {
	
	uint8_t returnCode = 0;
	char temp[256] = {0};

	modemLib.flush();

	modemLib.write("AT+QMTCLOSE=0\r\n");
	
	if(modemLib.getResponse(temp,"+QMTCLOSE: 0,0",sizeof(temp),10000) != 0){

		ESP_LOGW(TAG, "Close a Network Ok");
	
		returnCode = 1;

	}
	else{
		
		ESP_LOGE(TAG, "Close a Network Fail");

		returnCode = 0;
	}

	return returnCode;

}


