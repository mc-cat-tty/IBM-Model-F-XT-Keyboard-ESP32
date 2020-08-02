#include <BleKeyboard.h>
#include "driver/adc.h"

#define clk_pin 18
#define data_pin 21
#define led LED_BUILTIN  // Used for NumLock

// FSM states defining
#define START_BITS_START 0x00
#define START_BITS_END 0x01
#define PAYLOAD_RECEIVING 0x02

BleKeyboard bk("Keyboard Model F XT", "IBM");  // Bluetooth Keyboard

uint8_t val;
uint8_t lastVal;
int received_bits = 0;
uint8_t state = START_BITS_START;
//size_t res = 0;  // Sending result (0 --> error; 1 --> OK)
//uint8_t sending_counter = 0;  // This variable prevents infinite sending loop

unsigned char translationTable[128] = {
  0,  // Not Used
  KEY_ESC,
  '1',
  '2',
  '3',
  '4',
  '5',
  '6',
  '7',
  '8',
  '9',
  '0',
  '-',  // ' and ?
  0xb6,
  KEY_BACKSPACE,
  
  KEY_TAB,
  'q',
  'w',
  'e',
  'r',
  't',
  'y',
  'u',
  'i',
  'o',
  'p',
  0xb7,
  0xb8,
  KEY_RETURN,
  
  KEY_LEFT_CTRL,
  'a',
  's',
  'd',
  'f',
  'g',
  'h',
  'j',
  'k',
  'l',
  0xbb,
  0xbc,
  'ù',
  
  KEY_LEFT_SHIFT,
  0x60,
  'z',
  'x',
  'c',
  'v',
  'b',
  'n',
  'm',
  ',',
  '.',
  0xc0,
  KEY_RIGHT_SHIFT,
  KEY_RIGHT_GUI,
  
  KEY_LEFT_ALT,
  ' ',
  KEY_CAPS_LOCK,

  KEY_F1,
  KEY_F2,
  KEY_F3,
  KEY_F4,
  KEY_F5,
  KEY_F6,
  KEY_F7,
  KEY_F8,
  KEY_F9,
  KEY_F10,
  
  0xdb,  // Num Lock
  KEY_RIGHT_CTRL,
  0xe7,
  0xe8,
  0xe9,
  0xde,
  0xe4,
  0xe5,
  0xe6,
  0xdf,
  0xe1,
  0xe2,
  0xe3,
  0xea,
  0xeb
};


void IRAM_ATTR clk_down() {
  switch (state) {
    case START_BITS_START:
      if (!digitalRead(data_pin))
        state = START_BITS_END;
      else
        state = START_BITS_START;
      break;
    case START_BITS_END:
      if (digitalRead(data_pin))
        state = PAYLOAD_RECEIVING;
      else
        state = START_BITS_END;
      break;
    case PAYLOAD_RECEIVING:
      if (received_bits < 7) {  // Receiving
        val |= (digitalRead(data_pin) << received_bits);
        received_bits++;
      }
      else {  // Out Key
        val |= (digitalRead(data_pin) << received_bits);
        
        if (val != lastVal && (val & 0x7f) <= 83) {
                                                                                                pinMode(data_pin, OUTPUT);  // These instructions prevent Keyboard from sending data during time-consuming operations (BLE connection)
                                                                                                digitalWrite(data_pin, LOW);
          int msb = val & 0x80;  // Only the byte's MSB is on
          
          /*char buf[256];
          if (msb)
             sprintf(buf, "Released 0x%02x\n", val & 0x7f); // Debugging purpose
          else
             sprintf(buf, "Pressed 0x%02x\n", val & 0x7f);  // Only the byte's MSB is off
          Serial.print(buf);*/
          
             
          /*if (bk.isConnected()) {
            unsigned char key = translationTable[val & 0x7f];
            if (msb) {  // msb == 1 --> release
              if (key >= 0x80)  // If it is a modifier key
                bk.release(key);
            }
            else {  // msb == 0 --> press
              if (key >= 0x80) {  // If it is a modifier key
                sending_counter = 0;
                do {
                  res = bk.press(key);
                  sending_counter++;
                } while (res == 0 && sending_counter < 2);
              }
              else {
                sending_counter = 0;
                do {
                  res = bk.write(key);
                  sending_counter++;
                } while (res == 0 && sending_counter < 2);
              }
            }

            if ((val & 0x7f) == 0x45 and !msb)
              digitalWrite(led, !digitalRead(led));
            }*/

            if (bk.isConnected()) {
              unsigned char key = translationTable[val & 0x7f];
              if (msb) {  // msb == 1 --> release
                if (key >= 0x80)  // If it is a modifier key
                  bk.release(key);
              }                  
              else {  // msb == 0 --> press
                if (key >= 0x80 && key != KEY_BACKSPACE && key != KEY_DELETE)  // If it is a modifier key
                    bk.press(key);
                else
                    bk.write(key);
              }
  
              if ((val & 0x7f) == 0x45 and !msb)
                digitalWrite(led, !digitalRead(led));
            }
  
            lastVal = val;
                                                                                                pinMode(data_pin, INPUT_PULLUP);  // Re-activate Keyboard sending data
        }
        
        received_bits = 0;
        val = 0x00;
        state = START_BITS_START;
      }
      break;
  }
}

void goToDeepSleep()
{
  adc_power_off();
  //esp_wifi_stop();
}

void setup() {
  /* BATTERY-SAVE */
  //setCpuFrequencyMhz(80);
  goToDeepSleep();
  
  /* LED INIT */
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);

  /* BLE INIT */
  bk.begin();
  bk.setBatteryLevel(50);  /// Future update
  bk.releaseAll();

  //remove
  /*Serial.begin(115200);
  Serial.println("start");*/
  //end remove

  /* SOFT RESET */
  pinMode(data_pin, INPUT);  // Data Line Hi
  digitalWrite(data_pin, HIGH);
  pinMode(clk_pin, INPUT);
  digitalWrite(clk_pin, HIGH);
  delay(5);
  digitalWrite(clk_pin, LOW);  // Falling Edge
  delay(21);  // Wait ~20ms
  digitalWrite(clk_pin, HIGH);  // Rising Edge

  /* RECEIVING PIN MODE */
  pinMode(clk_pin, INPUT_PULLUP);
  pinMode(data_pin, INPUT_PULLUP);

  /* WAIT */
  //delay(100);

  attachInterrupt(digitalPinToInterrupt(clk_pin), clk_down, FALLING);
}

void loop() {
  ;;
}
