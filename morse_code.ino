// --- Morse Code Keypad, LCD, LED and Piezo Transmitter (Arduino C++) ---
// Reads input from a 4x4 Keypad, displays the typed message on an LCD, 
// and transmits the message in Morse code (LED flash and Piezo tone) when '#' is pressed.

#include <Keypad.h>
#include <LiquidCrystal_I2C.h> 

// --- LCD CONFIGURATION ---
// I2C address (0x27 is common, check your module/Tinkercad default)
// Dimensions: 16 columns, 2 rows
LiquidCrystal_I2C lcd(0x27, 16, 2); 
const int MAX_INPUT_LENGTH = 16;
String inputBuffer = ""; // Stores characters typed by the user

// --- PIN DEFINITIONS ---
const int ledPin = 13;      // Digital pin for the LED output
const int piezoPin = 9;     // Digital pin for the Piezo Buzzer output

// --- TIMING CONSTANTS ---
const int DOT_DURATION = 150; // Time in milliseconds (ms) for one dot (T)
const int PIEZO_FREQ = 750;   // Tone frequency in Hz (750Hz is a good, clear tone)

// --- KEYPAD CONFIGURATION ---
const byte ROWS = 4; // Four rows
const byte COLS = 4; // Four columns

// Define the symbols on the 4x4 keypad. Note the use of '*' for CLEAR and '#' for SEND.
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'} // '*' = Clear, '#' = Send
};

// Define the Arduino pins connected to the Keypad rows and columns
byte rowPins[ROWS] = {12, 11, 10, 8}; 
byte colPins[COLS] = {7, 6, 5, 4};   

// Initialize the Keypad object
Keypad customKeypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// --- MORSE CODE MAPPING (A-Z, 0-9) ---
const char* morseMap[] = {
  ".-",    // A (Index 0)
  "-...",  // B
  "-.-.",  // C
  "-..",   // D
  ".",     // E
  "..-.",  // F
  "--.",   // G
  "....",  // H
  "..",    // I
  ".---",  // J
  "-.-",   // K
  ".-..",  // L
  "--",    // M
  "-.",    // N
  "---",   // O
  ".--.",  // P
  "--.-",  // Q
  ".-.",   // R
  "...",   // S
  "-",     // T
  "..-",   // U
  "...-",  // V
  ".--",   // W
  "-..-",  // X
  "-.--",  // Y
  "--..",  // Z
  "-----", // 0 (Index 26)
  ".----", // 1
  "..---", // 2
  "...--", // 3
  "....-", // 4
  ".....", // 5
  "-....", // 6
  "--...", // 7
  "---..", // 8
  "----."  // 9 (Index 35)
};

// --- HELPER FUNCTIONS FOR OUTPUT ---

// Turns ON the LED and Piezo
void outputOn() {
  digitalWrite(ledPin, HIGH); 
  tone(piezoPin, PIEZO_FREQ);
}

// Turns OFF the LED and Piezo
void outputOff() {
  digitalWrite(ledPin, LOW);  
  noTone(piezoPin);
}

// Creates a single dot flash and beep
void dot(const char* morse) {
  outputOn();
  delay(DOT_DURATION);        
  outputOff();
  delay(DOT_DURATION); // Space between elements (1 * T)
  
  // Update LCD bottom row to show progress
  lcd.setCursor(0, 1);
  lcd.print("TX: ");
  lcd.print(morse);
  lcd.print(" ");
}

// Creates a single dash flash and beep
void dash(const char* morse) {
  outputOn();
  delay(DOT_DURATION * 3); // 3 * T          
  outputOff();
  delay(DOT_DURATION); // Space between elements (1 * T)

  // Update LCD bottom row to show progress
  lcd.setCursor(0, 1);
  lcd.print("TX: ");
  lcd.print(morse);
  lcd.print(" ");
}

// Sends the Morse code for a single character
void sendChar(char c) {
  char upperC = toupper(c);
  const char* morseSequence = NULL;

  if (upperC >= 'A' && upperC <= 'Z') {
    morseSequence = morseMap[upperC - 'A'];
  } else if (upperC >= '0' && upperC <= '9') {
    morseSequence = morseMap[upperC - '0' + 26];
  } else {
    // Treat unknown characters as an unknown signal (or ignore)
    return;
  }

  // Debug output
  Serial.print(upperC);
  Serial.print(" ");

  // Iterate through the Morse sequence string
  for (int i = 0; morseSequence[i] != '\0'; i++) {
    if (morseSequence[i] == '.') {
      dot(morseSequence);
    } else if (morseSequence[i] == '-') {
      dash(morseSequence);
    }
  }

  // Space between letters (3 * T) - minus the 1*T already done by dot/dash
  delay(DOT_DURATION * 2); 
}

// --- MAIN FUNCTIONS ---

// Initializes the LCD screen
void lcdInit() {
  lcd.init();
  lcd.backlight(); // Turn on backlight
  lcd.setCursor(0, 0);
  lcd.print("Morse Keypad TX");
  lcd.setCursor(0, 1);
  lcd.print("Type & Press #");
}

// Clears the buffer and resets the LCD input row
void clearBuffer() {
  inputBuffer = "";
  lcd.setCursor(0, 0);
  lcd.print("                "); // Clear top row
  lcd.setCursor(0, 1);
  lcd.print("                "); // Clear bottom row
  lcd.setCursor(0, 0);
  lcd.print("Type Message:");
}

// Transmits the entire buffer
void transmitBuffer() {
  Serial.println("\n--- START TRANSMISSION ---");
  Serial.print("Message: ");
  Serial.println(inputBuffer);

  // Update LCD for transmission status
  lcd.setCursor(0, 0);
  lcd.print("--- TRANSMITTING ---");
  lcd.setCursor(0, 1);
  lcd.print("TX: ");

  // Iterate over all characters in the buffered message
  for (int i = 0; i < inputBuffer.length(); i++) {
    sendChar(inputBuffer.charAt(i));
  }
  
  // Transmission complete
  delay(DOT_DURATION * 7); // Final word space
  Serial.println("--- TRANSMISSION COMPLETE ---\n");
  
  // Reset for next message
  clearBuffer();
  lcd.setCursor(0, 1);
  lcd.print("Type & Press #");
}

// --- SETUP ---
void setup() {
  // Initialize output pins
  pinMode(ledPin, OUTPUT);
  // Piezo pin does not require pinMode() if using tone() function

  // Initialize serial communication for debugging/status
  Serial.begin(9600);

  // Initialize LCD
  lcdInit();
  
  Serial.println("--- System Initialized ---");
  Serial.println("Keypad: '*' to CLEAR, '#' to SEND");
}

// --- MAIN LOOP ---
void loop() {
  // Check for a key press
  char customKey = customKeypad.getKey();

  if (customKey) {
    if (customKey == '#') {
      // '#' key pressed - Start Transmission
      if (inputBuffer.length() > 0) {
        transmitBuffer();
      } else {
        lcd.setCursor(0, 1);
        lcd.print("Buffer Empty!   ");
      }

    } else if (customKey == '*') {
      // '*' key pressed - Clear Buffer
      Serial.println("Input cleared.");
      clearBuffer();

    } else if (inputBuffer.length() < MAX_INPUT_LENGTH) {
      // Regular character key pressed (0-9, A-D)
      inputBuffer += customKey;
      Serial.print("Buffer: ");
      Serial.println(inputBuffer);

      // Display the current buffer on the top row of the LCD
      lcd.setCursor(0, 0);
      lcd.print("                "); // Clear line
      lcd.setCursor(0, 0);
      lcd.print(inputBuffer);
      
    } else {
      // Buffer is full
      lcd.setCursor(0, 1);
      lcd.print("Buffer FULL!    ");
    }
  }
}
