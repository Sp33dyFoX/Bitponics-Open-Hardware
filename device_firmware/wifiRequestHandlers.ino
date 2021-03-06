//********************************************************************************
//********************************************************************************

void wifiAssocRequestHandler(){

  char errorMsg[28];

  //Serial.println(buf);
  if(wifi.match(F("HTTP/1.1 200"))){

    //Serial.println(F("success!"));
    if (wifi.match(F("STATES="))){
      Serial.print(F("states = "));
      wifi.getsTerm(statesBuf, sizeof(statesBuf), '\n');
      Serial.println(statesBuf);
      int relayState[2] = {
        int(statesBuf[2]-'0'), int(statesBuf[6]-'0')
        };
        for(int i=0; i<2;i++){
          if(relayState[i] != curRelayState[i] || completedPosts == 0) switchRelay(i, relayState[i]);
        }
      completedPosts ++;
    }
    if(wifi.match(F("CALIB_MODE="))){
      Serial.print(F("calib mode = "));

      char calibBuf[6];
      wifi.getsTerm(calibBuf, sizeof(calibBuf),'\n');
      Serial.println(calibBuf);
      Serial.print(F("Last mode: "));
      Serial.println(calibMode);
      if(strcmp(calibMode, calibBuf)) {
        strncpy(calibMode, calibBuf, 6);
        calibrate();
      }
      else Serial.println(F("Already calibrated"));
    }  
    else{
      memset(&calibMode[0], 0, sizeof(calibMode)); // clearing the calibMode array
    }

  }
  else {

    wifi.gets(errorMsg,sizeof(errorMsg));
    Serial.println(F("error "));
    Serial.println(errorMsg);
    errorCount ++;
    if(errorCount > 10){
      resetWifi();
      resetBoard();
    }

  }

  Serial.println(F("------- Close -------"));

  timeout = millis();
  wifi.close();
  wifi.flushRx();
  bReceivedStatus = true;


}

//********************************************************************************
//********************************************************************************
void wifiApRequestHandler(){

  if (wifi.gets(data, sizeof(data))) {
    if      (strncmp_P(data, PSTR("GET"), 3) == 0) apGetResponse();
    else if (strncmp_P(data, PSTR("*GET"), 4) == 0) apGetResponse(); 
    else if (strncmp_P(data, PSTR("N*GET"), 5) == 0) apGetResponse();
    else if (strncmp_P(data, PSTR("EN*GET"), 6) == 0) apGetResponse();
    else if (strncmp_P(data, PSTR("PEN*GET"), 7) == 0) apGetResponse();
    else if (strncmp_P(data, PSTR("OPEN*GET"), 8) == 0) apGetResponse();
    else if (strncmp_P(data, PSTR("*OPEN*GET"), 9) == 0) apGetResponse();

    else if (strncmp_P(data, PSTR("POST"), 4) == 0) apPostResponse();
    else if (strncmp_P(data, PSTR("*POST"), 5) == 0) apPostResponse();
    else if (strncmp_P(data, PSTR("N*POST"), 6) == 0) apPostResponse();
    else if (strncmp_P(data, PSTR("EN*POST"), 7) == 0) apPostResponse();
    else if (strncmp_P(data, PSTR("PEN*POST"), 8) == 0) apPostResponse(); 
    else if (strncmp_P(data, PSTR("OPEN*POST"), 9) == 0) apPostResponse();
    else if (strncmp_P(data, PSTR("*OPEN*POST"), 10) == 0) apPostResponse();
    else {
      // Unexpected request
      Serial.print(F("Unexpected Request: "));
      Serial.println(data);
      wifi.flushRx();		// discard rest of input
      //Serial.println(F("Sending 404"));
      //send404();
    } 


  }

}

void apGetResponse(){

  Serial.println(F("-> Received GET request"));
  while (wifi.gets(data, sizeof(data)) > 0) {	      
    /* Skip rest of request */
    Serial.println(data);
  }

  sendInitialJSON();
  Serial.println(F("-> Sent JSON ")); 

}

void apPostResponse(){

  /* Form POST */
  // char ssid[32];
  char pass[32];
  char mode[16];
  Serial.println(F("-> Received POST"));

  /* Get posted field value */
  if (wifi.match(F("SSID="))) wifi.getsTerm(ssid, sizeof(ssid),'\n');  
  if (wifi.match(F("PASS="))) wifi.getsTerm(pass, sizeof(pass),'\n');
  if (wifi.match(F("MODE="))) wifi.getsTerm(mode, sizeof(mode),'\n');
  if (wifi.match(F("SKEY="))) wifi.getsTerm(SKEY,sizeof(SKEY),'\n');
  if (wifi.match(F("PKEY="))) wifi.gets(PKEY, sizeof(PKEY));


  printHeaders();

  wifi.sendChunk(F("{ \"ssid\": \""));
  wifi.sendChunk(ssid);
  wifi.sendChunk(F("\", \"pass\": \""));
  wifi.sendChunk(pass);
  wifi.sendChunk(F("\", \"mode\": \""));
  wifi.sendChunk(mode);
  wifi.sendChunk(F("\", \"skey\": \""));
  wifi.sendChunk(SKEY);
  wifi.sendChunk(F("\", \"pkey\": \""));
  wifi.sendChunk(PKEY);
  wifi.sendChunkln(F("\" }"));
  wifi.sendChunkln();

  //  if(strcasecmp(auth, "OPEN_MODE") == 0) mode = 0;
  //  else mode = 3;

  if(wifi.setAuth(3)) Serial.println(F("-> Set Auth Mode")); // TODO check if this is correct

  if(wifi.setPassphrase(pass)) Serial.println(F("-> Set Pass"));
  if(wifi.setDeviceID("Station")) Serial.print(F("-> Set DeviceID: "));
  Serial.println(wifi.getDeviceID(data, sizeof(data)));
  if(wifi.setSSID(ssid)) Serial.print(F("-> Set SSID: "));
  Serial.println(wifi.getSSID(data, sizeof(data)));
  if(wifi.setFtpUser(SKEY))Serial.print(F("-> Set Private Key: "));
  Serial.println(wifi.getFTPUSER(data, sizeof(data)));
  if(wifi.setFtpPassword(PKEY))Serial.print(F("-> Set Public Key: "));
  Serial.println(wifi.getFTPPASS(data, sizeof(data)));
  if(wifi.setJoin(1))Serial.print(F("-> Set JOIN: "));
  Serial.println(wifi.getJoin());
  if( wifi.save() ) Serial.println(F("-> Saved"));
  wifi.leave();

  // if(sizeof(SKEY)<16)

  // wifiConnect(ssid, pass, mode);

  resetBoard();

}

void printHeaders(){

  wifi.println(F("HTTP/1.1 200 OK"));
  wifi.println(F("Content-Type: application/json"));
  wifi.println(F("Transfer-Encoding: chunked"));
  wifi.println(F("Access-Control-Allow-Origin: *"));
  wifi.println(F("Cache-Control: no-cache"));
  wifi.println(F("Connection: close"));
  wifi.println();
}


