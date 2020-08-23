/*
 * I hate the String/string/char[]/const char* mess!!!!!!!!!!
 * This code looks like shit, but it is for evaluation only. Maybe ;-)
 */

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  if (String(topic).equals(inTopic)) {
    uint32_t cmd = strtol((const char *) &payload[0], NULL, 16);
    if (cmd > 0x13) {
      //target temperature received
      //Not implemented.
    } else {
      //a button is received
      switch (cmd) {
        case 0x5:
          //power
          virtualBTN = PWR;
          power_cmd = !power_sts;
          break;
        case 0x6:
          //up
          virtualBTN = UP;
          BTN_timeout = millis() + 500;
          break;
        case 0x7:
          //down
          virtualBTN = DWN;
          BTN_timeout = millis() + 500;
          break;
        case 0x8:
          //filter
          virtualBTN = FLT;
          filter_cmd = !filter_sts;
          break;
        case 0xA:
          //heater
          virtualBTN = HTR;
          heater_cmd = !(heater_red_sts | heater_green_sts);
          break;
        case 0xB:
          //unit
          virtualBTN = UNT;
          celsius_cmd = !celsius_sts;
          break;
        case 0xC:
          //air
          virtualBTN = AIR;
          air_cmd = !air_sts;
          break;
        case 0xD:
          //timer
          virtualBTN = TMR;
          break;
        case 0xE:
          //lock
          virtualBTN = LCK;
          locked_cmd = !locked_sts;
          break;
        case 0xF:
          //no button
          virtualBTN = NOBTN;
          break;
        case 0x10:
          //reset chlorine timer
          setClTimer();
          break;
        default:
          break;
      }
    }
    //mqttPublish()???
  }
}

void mqttReconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqttClient.publish("outTopic", "hello world");
      // ... and resubscribe
      mqttClient.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void mqttPublish() {
    //calculate days since clTimestamp
  uint32_t daysSinceClReset = 0;
  daysSinceClReset = DateTime.now() - clTime;
  daysSinceClReset /= (3600 * 24);

  float heatingRatio = (float)getHeatingTime() / (DateTime.now() - DateTime.getBootTime());

  snprintf (mqtt_msg, MQTT_BUFFER_SIZE, (const char *)(cur_tmp_val));
  mqttClient.publish(String(outTopic+"/temp").c_str(), mqtt_msg, MQTT_BUFFER_SIZE);
  snprintf (mqtt_msg, MQTT_BUFFER_SIZE, (const char *)(set_tmp_val));
  mqttClient.publish(String(outTopic+"/target").c_str(), mqtt_msg,MQTT_BUFFER_SIZE);
  snprintf (mqtt_msg, MQTT_BUFFER_SIZE, (const char *)(locked_sts));
  mqttClient.publish(String(outTopic+"/locked").c_str(), mqtt_msg,MQTT_BUFFER_SIZE);
  snprintf (mqtt_msg, MQTT_BUFFER_SIZE, (const char *)(air_sts));
  mqttClient.publish(String(outTopic+"/air").c_str(), mqtt_msg,MQTT_BUFFER_SIZE);
  snprintf (mqtt_msg, MQTT_BUFFER_SIZE, (const char *)(celsius_sts));
  mqttClient.publish(String(outTopic+"/celsius").c_str(), mqtt_msg,MQTT_BUFFER_SIZE);
  snprintf (mqtt_msg, MQTT_BUFFER_SIZE, (const char *)(heater_red_sts));
  mqttClient.publish(String(outTopic+"/heating").c_str(), mqtt_msg,MQTT_BUFFER_SIZE);
  snprintf (mqtt_msg, MQTT_BUFFER_SIZE, (const char *)(heater_green_sts));
  mqttClient.publish(String(outTopic+"/heater").c_str(), mqtt_msg,MQTT_BUFFER_SIZE);
  snprintf (mqtt_msg, MQTT_BUFFER_SIZE, (const char *)(filter_sts));
  mqttClient.publish(String(outTopic+"/filter").c_str(), mqtt_msg,MQTT_BUFFER_SIZE);
  snprintf (mqtt_msg, MQTT_BUFFER_SIZE, (const char *)(power_sts));
  mqttClient.publish(String(outTopic+"/target").c_str(), mqtt_msg,MQTT_BUFFER_SIZE);
  snprintf (mqtt_msg, MQTT_BUFFER_SIZE, (DateTime.format(DateFormatter::SIMPLE)).c_str());
  mqttClient.publish(String(outTopic+"/time").c_str(), mqtt_msg,MQTT_BUFFER_SIZE);
  snprintf (mqtt_msg, MQTT_BUFFER_SIZE, String(heatingRatio).c_str());
  mqttClient.publish(String(outTopic+"/ratio").c_str(), mqtt_msg,MQTT_BUFFER_SIZE);
  snprintf (mqtt_msg, MQTT_BUFFER_SIZE, (const char *)(daysSinceClReset));
  mqttClient.publish(String(outTopic+"/days").c_str(), mqtt_msg,MQTT_BUFFER_SIZE);
}
