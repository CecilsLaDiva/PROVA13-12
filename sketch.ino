#include <WiFi.h>
#include <HTTPClient.h>

#define led_verde 2 // Pin used to control the Green Led
#define led_vermelho 40 // Pin used to control the Red Led
#define led_amarelo 9 // Pin used to control the Yellow Led

const int buttonPin = 18;  // the number of the pushbutton pin
int buttonState = LOW;
int lastButtonState = LOW; // previous state of the button
unsigned long lastDebounceTime = 0; // the last time the output pin was used
const unsigned long debounceDelay = 50; // debounce time (why wokwi does noot have a capacitor i will never understand)

const int ldrPin = 4;  // the number of the pushbutton pin
int threshold=600;

unsigned long previousMillis = 0;
const long intervalo = 1000; // interval at which to blink using mills (godoi's favourite)
bool ledState = LOW;

unsigned long previousMillisMode = 0;
const long intervaloVerde = 3000; // 3 seconds
const long intervaloAmarelo = 2000; // 2 seconds
const long intervaloVermelho = 5000; // 5 seconds
int mode = 0; // 0 is green , 1 is yellow , 2is red

unsigned long buttonPressTime = 0;
bool buttonPressed = false;
int buttonPressCount = 0;

void setup() {
  // Initial config. of the the pins to control the leds as outputs of the ESP32
  pinMode(led_amarelo,OUTPUT);
  pinMode(led_verde,OUTPUT);
  pinMode(led_vermelho,OUTPUT);

  // Input "start"
  pinMode(buttonPin, INPUT); // Initialize the pushbutton pin as an input

  digitalWrite(led_amarelo, LOW);
  digitalWrite(led_verde, LOW);
  digitalWrite(led_vermelho, LOW);

  Serial.begin(9600); // Config. to debug the serial interface between the ESP32 and the computer with 9600 of baud rate

  WiFi.begin("Wokwi-GUEST", ""); // Open WiFi connection with SSID Wokwi-GUEST

  while (WiFi.status() != WL_CONNECT_FAILED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("Conectado ao WiFi com sucesso!"); // Considering that the loop above was exited, the ESP32 is now connected to WiFi (another option is to place this command inside the if below)

  // Checks button status
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH) {
    Serial.println("Botão pressionado!");
  } else {
    Serial.println("Botão não pressionado!");
  }

  if(WiFi.status() == WL_CONNECTED){ // If the ESP32 is connected to the Internet
    HTTPClient http;

    String serverPath = "http://www.google.com.br/"; // Endpoint of the HTTP requisition

    http.begin(serverPath.c_str());

    int httpResponseCode = http.GET(); // HTTP Request Result Code

    // This code checks the HTTP response code and prints it along with the payload if the response code is bigger than 0.
    if (httpResponseCode>0) { 
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
    }
    else { // If the response code is less than or equal to 0, it prints an error code.
      Serial.print("Error code: vish ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
  // Checks the WiFi connection status and prints a message if the WiFi is disconnected.
  else { 
    Serial.println("WiFi Disconnected");
  }
}

// Function to get the interval for the current mode
long getIntervaloForMode(int mode) {
  if (mode == 0) {
    return intervaloVerde;
  } else if (mode == 1) {
    return intervaloAmarelo;
  } else if (mode == 2) {
    return intervaloVermelho;
  }
  return 0;
}

// In the loop function, the value from LDR is read and prints a message to turn on or off the LED based on the light intensity
void loop() {
  int ldrstatus=analogRead(ldrPin);

  unsigned long currentMillis = millis();

  if(ldrstatus<=threshold){
    Serial.print("Está escuro! É noite!");
    Serial.println(ldrstatus);

    // Blink the yellow LED every second using millis()
    if (currentMillis - previousMillis >= intervalo) {
      previousMillis = currentMillis;
      ledState = !ledState;
      digitalWrite(led_amarelo, ledState);
    }
  } else {
    Serial.print("Está claro! É manhã!");
    Serial.println(ldrstatus);
    digitalWrite(led_amarelo, LOW); // Ensure the yellow LED is off

    // Handle the LED sequence for bright mode
    if (currentMillis - previousMillisMode >= getIntervaloForMode(mode)) {
      previousMillisMode = currentMillis;
      mode = (mode + 1) % 3; // Goes through 0, 1, 2
      // Turn off all LEDs
      digitalWrite(led_verde, LOW);
      digitalWrite(led_amarelo, LOW);
      digitalWrite(led_vermelho, LOW);
      // Turn on the appropriate LED for the current mode
      if (mode == 0) {
        digitalWrite(led_verde, HIGH);
      } else if (mode == 1) {
        digitalWrite(led_amarelo, HIGH);
      } else if (mode == 2) {
        digitalWrite(led_vermelho, HIGH);
      }
    }
  }

  // Check if the button is pressed when the red LED is on
  int reading = digitalRead(buttonPin);
  if (reading != lastButtonState) {
    lastDebounceTime = currentMillis;
  }

  if ((currentMillis - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (mode == 2 && buttonState == HIGH && !buttonPressed) {
        buttonPressed = true;
        buttonPressTime = currentMillis;
        buttonPressCount++;
      }
    }
  }
  lastButtonState = reading;

  // If the button was pressed and 1 second has passed, change to green mode
  if (buttonPressed && currentMillis - buttonPressTime >= 1000) {
    buttonPressed = false;
    mode = 0;
    previousMillisMode = currentMillis;
    digitalWrite(led_vermelho, LOW);
    digitalWrite(led_verde, HIGH);
  }

  // If the button is pressed 3 times, send an HTTP request
  if (buttonPressCount >= 3) {
    buttonPressCount = 0; // Reset the count
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      String serverPath = "http://www.google.com.br/"; // Endpoint of the HTTP requisition
      http.begin(serverPath.c_str());
      int httpResponseCode = http.GET(); // HTTP Request Result Code
      if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: oh ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);
      } else {
        Serial.print("Error code: eita");
        Serial.println(httpResponseCode);
      }
      http.end();
    } else {
      Serial.println("WiFi Disconnected");
    }
  }
}