#include <LiquidCrystal_I2C.h>
#include <Wire.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int analogOutPin = 9;  // Analog output pin for the LED
const int btnS = 5;          // SPF button
const int buzzer = 8;
const int ledPin_SPF = 2;
const int ledPin_Shade = 3;
int prevIndex = -1;

const unsigned int limitSPF = 15000;  // 20 seconds for SPF timer

unsigned int limit = 0;
unsigned int startTimerSPF = 0;
unsigned int startTimerShade = 0;

bool spfAlertTriggered = false;
bool shadeAlertTriggered = false;
bool shadeTimerStarted = false;

int sensorValue = 0;
int outputValue = 0;

int buttonState = 0;

void setup() {
  Serial.begin(9600);

  pinMode(ledPin_SPF, OUTPUT);
  pinMode(ledPin_Shade, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(btnS, INPUT);

  digitalWrite(ledPin_SPF, LOW);
  digitalWrite(ledPin_Shade, LOW);


  lcd.init();
  lcd.clear();
  lcd.backlight();

  startTimerSPF = millis();
  startTimerShade = millis();
}

void loop() {
  float voltage = getVoltage();
  int uvIndex = getUVIndex(voltage);

// Display and serial debug (handled later)
  Serial.println("UV Index: " + String(uvIndex) + " SPF Time: " + String(millis() - startTimerSPF) + " Shade limit: " + String(limit) + " Shade timer: " + String(millis() - startTimerShade));


   lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("UV Index: ");
  lcd.setCursor(10, 0);
  lcd.print(String(uvIndex) + String(" "));

  lcd.setCursor(0, 1);  
  switch (uvIndex) {
    case 1:
    case 2:
      lcd.print("No Risk         ");
      break;
    case 3:
    case 4:
    case 5:
      lcd.print("Moderate Risk   ");
      break;
    case 6:
    case 7:
      lcd.print("High Risk       ");
      break;
    case 8:
    case 9:
    case 10:
      lcd.print("Very High       ");
      break;
    case 11:
      lcd.print("Extreme Risk    ");
      break;
    default:
      lcd.print("Invalid UV      ");
  }
  checkShadeTimer(uvIndex);
  checkSPFTimer();
}


void checkShadeTimer(int uvIndex) {
  if (uvIndex >= 3) {
    if (!shadeAlertTriggered) {  // Start the timer only if alert is not already triggered
    if (!shadeTimerStarted)
    {
      shadeTimerStarted = true;
      startTimerShade = millis();
    }
    
      if (uvIndex <= 5) {
        limit = 10000;  // 10 seconds for moderate risk
      } else if (uvIndex <= 7) {
        limit = 5000;  // 5 seconds for high risk
      } else if (uvIndex <= 10) {
        limit = 3000;  // 3 seconds for very high risk
      } else {
        limit = 0;  // Immediate alert for extreme risk
      }

      // Trigger the alert if the timer has elapsed or if immediate risk
      if (limit == 0 || (millis() - startTimerShade >= limit)) {
        shadeAlertTriggered = true;
        digitalWrite(ledPin_Shade, HIGH);
        tone(buzzer, 5000, 5000);
        lcd.setCursor(0, 0);
        lcd.print("SEEK shade!     ");
        lcd.setCursor(0, 1);
        lcd.print("                ");
        delay(4000);
        digitalWrite(ledPin_Shade, LOW);
      }
    }
  } else {
    limit = INT32_MAX;
    startTimerShade = 0;
    digitalWrite(ledPin_Shade, LOW);
    noTone(buzzer);
    shadeAlertTriggered = false;
    shadeTimerStarted = false;
  }

}

void checkSPFTimer() {
  if (!spfAlertTriggered && (millis() - startTimerSPF >= limitSPF)) {
    spfAlertTriggered = true;
    tone(buzzer, 5000, 5000);
    lcd.setCursor(0, 0);
    lcd.print("Reapply SPF!     ");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    digitalWrite(ledPin_SPF, HIGH);
    while (1) {
      if(digitalRead(btnS))
      {
        if(buttonState == 0)
        {
          buttonState = 1;
          break;
        }
        else {
          buttonState = 0;
          break;
        }
      }
    }
    startTimerSPF = millis();
    spfAlertTriggered = false;
    digitalWrite(ledPin_SPF, LOW);
  }
}

bool debounce(int pin) {
  static unsigned long lastDebounceTime = 0;
  const unsigned long debounceDelay = 50;  // 50ms debounce time
  static int lastButtonState = LOW;

  int currentButtonState = digitalRead(pin);
  if (currentButtonState != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    lastButtonState = currentButtonState;
    return currentButtonState == HIGH;
  }
  return false;
}

int getUVIndex(float voltage) {
  if (voltage > 4.38) return 1;
  else if (voltage > 4.04) return 2;
  else if (voltage > 3.70) return 3;
  else if (voltage > 3.36) return 4;
  else if (voltage > 3.04) return 5;
  else if (voltage > 2.70) return 6;
  else if (voltage > 2.36) return 7;
  else if (voltage > 2.02) return 8;
  else if (voltage > 1.68) return 9;
  else if (voltage > 1.34) return 10;
  else if (voltage > 1.00) return 11;
  else return 0;  // Invalid range
}

float getVoltage() {
  delay(1000);
  sensorValue = analogRead(A2);                  // Read analog input from sensor
  float voltage = (sensorValue / 1023.0) * 5.0;  // Convert to voltage
  return voltage;
}