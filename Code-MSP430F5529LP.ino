// Load Resistance Value of the MQ sensor, usually 1K 
#define RLOAD 1                  

// Coefficient A and B using correlation function for Different Gases
#define PARALPG 591.283
#define PARBLPG 2.076502
#define PARACO2 116.6020682
#define PARBCO2 2.769034857
#define PARACO 2916.222
#define PARBCO 3.203079

// Atmospheric Gas Level of Different Gases
#define ATMOLPG 350.16
#define ATMOCO2 419.63
#define ATMOCO  9.5   

float RZERO[3];
float PPM[3];
float resistance[3];

// LCD variables
#include <LiquidCrystal.h>

#define RS P2_4
#define EN P2_5
#define D4 P1_2
#define D5 P1_3
#define D6 P1_4
#define D7 P1_5

LiquidCrystal lcd(RS, EN, D4, D5, D6, D7); // Let the library know how we have connected the LCD


// Thinkspeak variables
#define DEBUG true                                      // Prints AT responses to serial terminal, set to False if you do not want to print
String wifiSSID = "YOUR_WIFI_NAME";                      // WiFi SSID
String wifiKey = "YOUR_WIFI_PASSWORD";                  // WiFi password
String apiKey = "THINGSPEAK_CHANNEL_WRITE_API_KEY";     // API key from thing speak, used to post data

void setup() {
  
  //Set up baud rate for communication.... check you ESP's and Microntroller Baud Rate
   Serial.begin(115200);
   Serial1.begin(115200);
   lcd.begin(16, 2);
   delay(100);

   //Getting RZero
   getRZero();
   Serial.print("RZERO for CO2:"); 
   Serial.print(RZERO[0]);
   Serial.print("Kohm");
   Serial.print("\n");
   Serial.print("RZERO for LPG:"); 
   Serial.print(RZERO[1]);
   Serial.print("Kohm");
   Serial.print("\n");
   Serial.print("RZERO for CO:"); 
   Serial.print(RZERO[2]);
   Serial.print("Kohm");
   Serial.print("\n");

   //Setting up Wifi Connection

  // reset module
  sendData("AT+RST\r\n",2000,DEBUG); 
  delay(3000);
  
  //configure to access AP
  sendData("AT+CWMODE=3\r\n",1000,DEBUG); 
  Serial.println("Attempting to connect to WiFi");
  lcd.setCursor(0,0);                               // Setting the cursor 
  lcd.print("Connecting to");                       // Prints 
  lcd.setCursor(0,1);                              
  lcd.print("Wifi");                               
  delay(1000);
  lcd.clear();                                      //Then clean it
  
  //Setting up command for wifi connection
  String cnnct = ("AT+CWJAP=\"" + wifiSSID + "\",\"" + wifiKey +"\"\r\n");
  String response = sendData(cnnct,20000,DEBUG);
  
  //If connection failed, try again
  while (response.indexOf("FAIL") != -1){ 
      Serial.println("Failed to connect...Retrying");
      lcd.setCursor(0,0);
      lcd.print("Connection fail");
      delay(1000);
      lcd.clear();
      response = sendData(cnnct,20000,DEBUG);
  }
  
  Serial.println("Sending data to ThinkSpeak...");
  lcd.setCursor(0,0);
  lcd.print("Connecting to");
  lcd.setCursor(0,1);
  lcd.print("Thingspeak");
  delay(1000);
  lcd.clear();
  
}
void getResistance() {
  int val1 = analogRead(P6_1);  // MQ-135
  int val2 = analogRead(P6_2);  // MQ-2
  resistance[0] = ((4096./(float)val1) - 1.)*RLOAD;
  resistance[1] = ((4096./(float)val2) - 1.)*RLOAD;
  resistance[2] = ((4096./(float)val2) - 1.)*RLOAD;
}

void getPPM() {
  getResistance();
  PPM[0] =  PARACO2 * powf((resistance[0]/RZERO[0]), -PARBCO2);
  PPM[1] =  PARALPG * powf((resistance[1]/RZERO[1]), -PARBLPG);   
  PPM[2] =  PARACO * powf((resistance[2]/RZERO[2]), -PARBCO); 
}  

void getRZero() {
  getResistance();
  RZERO[0] = resistance[0] * powf((ATMOCO2/PARACO2), (1./PARBCO2));  // Ro = Rs * pow((ppm/a) , (1/b))
  RZERO[1] = resistance[1] * powf((ATMOLPG/PARALPG), (1./PARBLPG));
  RZERO[2] = resistance[2] * powf((ATMOCO/PARACO), (1./PARBCO));
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
       //if response received break from the timeout loop
      if ((output.indexOf("FAIL") != -1)||(output.indexOf("OK")!=-1)){ 
        break;
      }
    }
    //Print the output for debugging
    if(debug){
      Serial.println(output);
    }    
    return output;
}

void loop() {
    //Get ppm values from Sensor
    //getRZero();
    getPPM();
    
    //Command for setting connection
    sendData("AT+CIPSTART=\"TCP\",\"184.106.153.149\",80\r\n",500,DEBUG); // turn on server on port 80
    String dataWrite = "GET /update?api_key=" + apiKey + "&field1=" + String(PPM[0]) + "&field2=" + String(PPM[1]) + "&field3=" + String(PPM[2]) +"\r\n";
  
  //Configuring for the outgoing command length
    String cmd = "AT+CIPSEND=";
    int length = dataWrite.length();
    cmd += length;
    cmd += "\r\n";

    //Set up TCP connection
    sendData(cmd,500,DEBUG); 
     //Send data to thing speak
    sendData(dataWrite,1000,DEBUG);

    lcd.setCursor(0,0);
    lcd.print("CO2:");
    lcd.print(PPM[0]);
    lcd.setCursor(0,1);
    lcd.print("LPG:");
    lcd.print(PPM[1]);
    delay(1000);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("CO:");
    lcd.print(PPM[2]);
    delay(1000);
    lcd.clear();


    Serial.print("CO2:"); 
    Serial.print(PPM[0]);
    Serial.print( "ppm" );
    Serial.print("    "); 
    Serial.print("LPG:"); 
    Serial.print(PPM[1]);
    Serial.print( "ppm" );
    Serial.print("    "); 
    Serial.print("CO:"); 
    Serial.print(PPM[2]);
    Serial.print( "ppm" );
    Serial.print("\n");
    
    // Delay for 5 seconds before sending data....
    // You can set this to (time+15000) as the Free version of Thingspeak adds data to the server every 15 secs.  
    long int time = millis(); 
    while((time+5000)>millis()){ 
    }  
}
