const int TIME = 20;

const int DOT = TIME;
const int DASH = TIME*3;
const int SPACE = TIME*3;
const int WORD_SPACE = TIME*7;

const char* MORSE_ALPHABET[] = {
  ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---",  // A-J
  "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-",    // K-T
  "..-", "...-", ".--", "-..-", "-.--", "--..",                           // U-Z
  "-----", ".----", "..---", "...--", "....-", ".....",                   // 0-5
  "-....", "--...", "---..", "----."                                      // 6-9
};

void setup() {
  pinMode(4, OUTPUT); // Définir la LED intégrée comme une sortie
  pinMode(0, INPUT);
  Serial.begin(9600);
}

void flashDot() {
    // Serial.print(".");
    digitalWrite(4, HIGH);
    delay(DOT); 
    digitalWrite(4, LOW);
    delay(SPACE);
}

void flashDash() {
    // Serial.print("-");
    digitalWrite(4, HIGH);
    delay(DASH); 
    digitalWrite(4, LOW);
    delay(SPACE);
}

void displayMorseChar(const char* morse) {
  for (int i = 0; morse[i] != '\0'; i++) {
    if (morse[i] == '.') flashDot();
    else if (morse[i] == '-') flashDash();
  }
  delay(SPACE);
}

void flashMorseChar(char c) {
  if (c >= 'A' && c <= 'Z') {
    displayMorseChar(MORSE_ALPHABET[c - 'A']);
  } else if (c >= '0' && c <= '9') {
    displayMorseChar(MORSE_ALPHABET[c - '0' + 26]);
  } else {
    // Serial.println("error");
  }
}

void flashMessageInMorse(const String message) {
  // Sur des variables de type String on retrouve .length() comme dans de nombreux languages
  for (int i = 0; i < message.length(); i++) {
    message.toUpperCase();
    char c = message[i];
    if (c == ' ') {
      delay(WORD_SPACE);
    } else {
      flashMorseChar(c);
      // Serial.println(c);
    }
  }
  // Serial.println();
}

void loop() {

  Serial.println(analogRead(0));
  flashMessageInMorse("SALUT VINCENT TU EST NUL");
  


}
