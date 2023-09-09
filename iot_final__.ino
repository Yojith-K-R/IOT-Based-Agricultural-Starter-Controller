#include<WiFi.h>
#include "EmonLib.h"
#include<Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "ESP8266"
#define WIFI_PASSWORD "Yojithkr"
#define API_KEY "AIzaSyAw4xDLTF4-bXQ-jnOJ9MSbtsiD3g5F_rE"
#define DATABASE_URL "https://final-c00a8-default-rtdb.asia-southeast1.firebasedatabase.app/"

EnergyMonitor emon1;             // Create an instance
EnergyMonitor emon2;
EnergyMonitor emon3;
EnergyMonitor emon15;



#define LED1_PIn 17
#define LED2_PIN 5
#define LDR_PIN 34
#define onBoard 2

#define PWMChannel 0

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

int vry = 0;
int vrb = 0;
int vyb = 0;
int c = 0;
int d = 0;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;
bool cntrl = false;



float voltage1 = 0;
float voltage2 = 0;
float voltage3 = 0;
char strng[20] = "Motor turned on";
char strng1[30] = "Motor turned off";
char strng2[50] = "Motor turned off due to voltage fault";

void setup() {



  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED1_PIn, OUTPUT);
  pinMode(onBoard, OUTPUT);
  pinMode(33, INPUT);
  

  emon1.voltage(34, 250, 1.7 );
  emon2.voltage(35, 250, 1.7 );
  emon3.voltage(32, 250, 1.7 );// Voltage: input pin, calibration, phase_shift

  emon15.current(33, 50);




  Serial.begin(9600);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print ("Connecting to Wi-Fi");

  digitalWrite(onBoard , LOW);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print("."); delay (300);
  }
  Serial.println();
  Serial.print ("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  digitalWrite(onBoard , HIGH);

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp (&config, &auth, "", ""))
  {
    Serial.println("signUp OK");
    signupOK = true;
  }
  else
  {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi (true);
}

void loop() {

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0))
  {
    Serial.println();
    Serial.println();
    Serial.println();

    sendDataPrevMillis = millis();
    emon1.calcVI(20, 20);
    emon2.calcVI(20, 20);
    emon3.calcVI(20, 20); // Calculate all. No.of half wavelengths (crossings), time-out
    // Print out all variables (realpower, apparent power, Vrms, Irms, power factor)

    double Irms = emon15.calcIrms(1480);


    Serial.print("Current = ");
    Serial.println(Irms);
    Firebase.RTDB.setFloat(&fbdo, "Voltage/Current", Irms);


    vry = emon1.Vrms  * 1.73;
    vrb = emon2.Vrms * 1.73;
    vyb = emon3.Vrms * 1.73;
    Serial.print("V1 = ");
    Serial.println(vry);
    Serial.print("V2 = ");
    Serial.println(vrb);
    Serial.print("V3 = ");
    Serial.println(vyb);

    if (Firebase.RTDB.setFloat(&fbdo, "Voltage/Vry", vry))
    {

      Serial.print("AC voltage Vry : "); Serial.println(vry);
      Serial.print("successfully saved to: " + fbdo.dataPath());
      Serial.println(" (" + fbdo.dataType() + ")");

    }

    else
    {
      Serial.println("FAILED ldr: " + fbdo.errorReason());
    }

    if (Firebase.RTDB. setFloat(&fbdo, "Voltage/Vrb", vrb))
    {

      Serial.print("AC voltage Vrb : "); Serial.println(vrb);
      Serial.print("successfully saved to: " + fbdo.dataPath());
      Serial.println(" (" + fbdo.dataType() + ")");

    }
    else
    {
      Serial.println("FAILED:voltage " + fbdo.errorReason());
    }

    if (Firebase.RTDB. setFloat(&fbdo, "Voltage/Vyb", vyb))
    {

      Serial.print("AC voltage Vyb : "); Serial.println(vyb);
      Serial.print("successfully saved to: " + fbdo.dataPath());
      Serial.println(" (" + fbdo.dataType() + ")");

    }
    else
    {
      Serial.println("FAILED:voltage " + fbdo.errorReason());
    }





    if (Firebase.RTDB.getInt(&fbdo, "Motor_Control/Control"))
    {
      if (fbdo.dataType() == "boolean")
      {
        cntrl = fbdo.boolData();
        Serial.println("successfully read motor control Variable from: " + fbdo.dataPath() +  " : " + cntrl + "("  + fbdo.dataType() + ")");
      }
    }
    else
    {
      Serial.println("FAILED to read motor control variable " + fbdo.errorReason());
    }


    if (  (vry > 280  &&  vry < 500)  && (vrb > 280  &&  vrb < 500)  && (vyb > 280  &&  vyb < 500)  &&  cntrl    && c == 0)
    {


      Serial.println("Motor is on");


      if (Firebase.RTDB. setString(&fbdo, "Motor_Control/Motor_status", strng))
      {
        Serial.println("Motor turned ON status updated to firebase also");
        digitalWrite(LED2_PIN, HIGH);
        delay(4000);
        digitalWrite(LED2_PIN, LOW);
        Serial.println("Motor is on");

      }
      Serial.print("c value is "); Serial.println(c);
      c = 1;
      Serial.print("c value is "); Serial.println(c);
      d = 0;
    }

    if ( ! ((vry > 280  &&  vry < 500)  && (vrb > 280  &&  vrb < 500)  && (vyb > 280  &&  vyb < 500) )   ||   (!cntrl)  )
    {
      Serial.print("d value before looping is "); Serial.println(d);
      if (d == 0)
      { digitalWrite(LED1_PIn, HIGH);
        delay(4000);
        digitalWrite(LED1_PIn, LOW);

        if ( ! ((vry > 280  &&  vry < 500)  && (vrb > 280  &&  vrb < 500)  && (vyb > 280  &&  vyb < 500) )   )
        {
          if (Firebase.RTDB. setString(&fbdo, "Motor_Control/Motor_status", strng2))
          {
            Serial.println("Motor turned OFF due to voltage fault status updated to firebase also");

          }



        }
        else
        {
          if (Firebase.RTDB. setString(&fbdo, "Motor_Control/Motor_status", strng1))
          {
            Serial.println("Motor turned OFF status updated to firebase also");

          }
        }
        Serial.print("d value after looping is "); Serial.println(d);

        d = 1;

        c = 0;
      }


    }






  }
  
}
