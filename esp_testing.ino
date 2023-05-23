long randNumber;

// Thinkspeak variables
#define DEBUG true                      // Prints AT responses to serial terminal, set to False if you do not want to print
String wifiSSID= "YOUR_WIFI_NAME";      // WiFi SSID
String wifiKey = "YOUR_WIFI_PASSWORD";  // WiFi password
String apiKey = "API_KEY";              // API key from thing speak, used to post data

void setup() {
  
  // Set up baud Rate
   Serial.begin(115200);
   Serial1.begin(115200);

  // Setting up Wifi Connection
  sendData("AT+RST\r\n",2000,DEBUG); 
  delay(3000);
  // Configure to access AP
  sendData("AT+CWMODE=3\r\n",1000,DEBUG); 
  Serial.println("Attempting to connect to WiFi");
  // Setting up command for wifi connection
  String cnnct = ("AT+CWJAP=\"" + wifiSSID + "\",\"" + wifiKey +"\"\r\n");
  String response = sendData(cnnct,20000,DEBUG);
  // If connection failed, try again
  while (response.indexOf("FAIL") != -1){ 
      Serial.println("Failed to connect...Retrying");
      response = sendData(cnnct,20000,DEBUG);
  }
  Serial.println("Sending data to ThinkSpeak...");
}

void getPPM() {
  randNumber = random(300);   // Generate Random Numbers to post on Thingspeak.
}  

String sendData(String command, const int timeout, boolean debug){
    String output = "";
    // Send the read character to the Serial1
    Serial1.println(command); 
    // Current time
    long int time = millis(); 
    // Wait for a response until timeout is up
    while( (time+timeout) > millis()){
      // The esp has data so display its output to the serial window 
      while(Serial1.available()){        
        char c = Serial1.read(); // read the next character.
        output+=c;
      }
       // If response received break from the timeout loop
      if ((output.indexOf("FAIL") != -1)||(output.indexOf("OK")!=-1)){ 
        break;
      }
    }
    // Print the output for debugging
    if(debug){
      Serial.println(output);
    }    
    return output;
}

void loop() {
    getPPM();
    
    // Command for setting connection
    sendData("AT+CIPSTART=\"TCP\",\"184.106.153.149\",80\r\n",500,DEBUG);        // Turn on server on port 80
    String dataWrite = "GET /update?api_key=" + apiKey + "&field1=" + String(randNumber) + +"\r\n";
    // Configuring for the outgoing command length
    String cmd = "AT+CIPSEND=";
    int length = dataWrite.length();
    cmd += length;
    cmd += "\r\n";
    
    // Set up TCP connection
    sendData(cmd,500,DEBUG); 
    // Send data to thing speak
    sendData(dataWrite,1000,DEBUG); 

    Serial.println(randNumber);
    // Delay for 5 seconds before sending data
    long int time = millis(); 
    while((time+5000)>millis()){ 
    }  
}
