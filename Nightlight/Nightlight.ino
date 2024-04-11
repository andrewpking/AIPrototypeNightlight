#include <RGBLed.h>

// Input pins
const int INPUT_BUTTON_PIN = 2;
const int RED_LED_PIN = 11;
const int GREEN_LED_PIN = 10;
const int BLUE_LED_PIN = 9;
const int PHOTORESITOR_PIN = A0;

// The duration of my fade
const long FADE_DURATION = 600;

// Default brightness value.
uint8_t brightness = 100;
bool fixed_brightness = true;

unsigned long elapsed;

// For debouncing, from ChatGPT
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
int lastButtonState = HIGH;          // the previous reading from the input pin

// For storing the current color of the LED, from ChatGPT
uint8_t currentR = 0;
uint8_t currentG = 0;
uint8_t currentB = 0;

// Fading state variables, from ChatGPT
bool isFading = false;
unsigned long fadeStartTime;
uint8_t startR, startG, startB;
uint8_t endR, endG, endB;

// From https://github.com/wilmouths/RGBLed 
RGBLed led(RED_LED_PIN, GREEN_LED_PIN, BLUE_LED_PIN, RGBLed::COMMON_CATHODE);

void setup()
{
  pinMode(INPUT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(PHOTORESITOR_PIN, INPUT);
  setColor(255, 0, 254);
  Serial.begin(9600);
}

// Wrapper function to set LED color and update current color variables, from ChatGPT.
// Prompt: "I want fade to color to fade from the color currently assigned to the RGB_LED"
// I did change this to use led.brightness since I can directly use my global brightness value.
void setColor(uint8_t red, uint8_t green, uint8_t blue) {
  currentR = red;
  currentG = green;
  currentB = blue;
  // The boolean flag was my idea.
  if(fixed_brightness){
    led.brightness(red, green, blue, 70);
  } else {
    led.brightness(red, green, blue, brightness); // From https://github.com/wilmouths/RGBLed 
  }  
}


// From ChatGPT for fading color without delay()
void startFadeToColor(uint8_t red, uint8_t green, uint8_t blue) {
  // Initialize fade parameters
  isFading = true;
  fadeStartTime = millis();
  startR = currentR;
  startG = currentG;
  startB = currentB;
  endR = red;
  endG = green;
  endB = blue;
}

// From ChatGPT for fading color and working in the loop.
void updateFade() {
  if (isFading) {
    if (elapsed >= FADE_DURATION) {
      // Fade complete
      setColor(endR, endG, endB);
      startFadeToColor(startR, startG, startB); // Reverse the fade, this is my code.
      //Serial.println("Fade complete.");
    } else {
      // Calculate intermediate color
      float progress = (float)elapsed / FADE_DURATION;
      uint8_t newR = startR + round(progress * (endR - startR));
      uint8_t newG = startG + round(progress * (endG - startG));
      uint8_t newB = startB + round(progress * (endB - startB));
      setColor(newR, newG, newB);
    }
  } else { // If not fading, update the brightness in response to brightness.
    setColor(currentR, currentG, currentB);
  }
}

// From ChatGPT when asked to read input from a button without delay as a seperate function.
bool debounceButton(uint8_t pin) {
  int reading = digitalRead(pin);
  static bool debouncedState = HIGH; // the stable reading from the input pin
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // if the button state has changed:
    if (reading != debouncedState) {
      debouncedState = reading;
      lastButtonState = reading; // save the reading
      debouncedState == LOW; // Return true if button is pressed (LOW due to INPUT_PULLUP)
    }
  }
  lastButtonState = reading; // save the reading
  return false; // No valid button press detected
}

// Value should not exceed 100 or be less than 0.
uint8_t getBrightness(int pin){
  // Code from ChatGPT to map analogRead values to value for brightness.
  int sensorValue = analogRead(pin); // Read the analog input (0 to 1023)
  //Serial.println(sensorValue);
  // Map the sensor value (0 to 1023) to the brightness range [0, 100]
  uint8_t bright = map(sensorValue, 0, 400, 0, 100) % 100; // In a pitch dark my photoresistor reads about 400
  return bright;
}

void loop() {
  brightness = getBrightness(PHOTORESITOR_PIN);
  //Serial.println(debounceButton(INPUT_BUTTON_PIN));
  if (debounceButton(INPUT_BUTTON_PIN)) {
    static uint8_t state = 0;
    uint8_t red, green, blue = 0;
    state = (state + 1) % 3;
    switch (state) {
      case 0:
        fixed_brightness = false;
        isFading = false;
        red = 255; green = 0; blue = 244;
        setColor(red, green, blue);
        break;
      case 1: 
        fixed_brightness = true;
        startR = 255; startG = 252; startB = 102;
        endR = 0; endG = 255; endB = 244;
        startFadeToColor(endR, endG, endB);
        break;
      case 2: 
        fixed_brightness = false;
        startR = 255; startG = 252; startB = 102;
        endR = 0; endG = 255; endB = 244;
        startFadeToColor(endR, endG, endB);
        break;
    }
    // Debug information.
    Serial.println("Fading to color: " + String(red) + " " + String(green) + " " + String(blue));
    Serial.println("State: " + String(state));
    Serial.println("Brightness: " + String(brightness));
    Serial.println("IsFading: " + String(isFading));
  } 
  // Update elapse, give it some room so it will reset the fade when duration is complete.
  elapsed = (millis() - fadeStartTime) % (FADE_DURATION + 100);
  // ChatGPT code, to update the fading effect or set LED color if not fading.
  updateFade();
}
