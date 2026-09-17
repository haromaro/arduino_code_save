#include <Wire.h>
#include <Adafruit_LiquidCrystal.h>

Adafruit_LiquidCrystal lcd(0x20); 

const int buttonPin = 4; 

// 시간 기준 설정
const unsigned long dotThreshold = 250;   // 이 시간보다 짧으면 점(.), 길면 선(-)
const unsigned long clearThreshold = 3000; // 3초 이상 꾹 누르면 LCD 전체 초기화 (추가)
const unsigned long letterTimeout = 1000;  // 1초 동안 입력 없으면 글자 판독
const unsigned long wordTimeout = 3000;    // 3초 동안 입력 없으면 띄어쓰기

unsigned long pressedTime = 0;
unsigned long releasedTime = 0;
boolean isPressed = false;
boolean isCleared = false; // 화면 초기화가 중복 실행되는 것을 방지하는 플래그

String morseCode = "";   
String lcdBuffer = "";   

char morseToChar(String morse) {
  if (morse == ".-") return 'A';    if (morse == "-...") return 'B';
  if (morse == "-.-.") return 'C';  if (morse == "-..") return 'D';
  if (morse == ".") return 'E';     if (morse == "..-.") return 'F';
  if (morse == "--.") return 'G';   if (morse == "....") return 'H';
  if (morse == "..") return 'I';    if (morse == ".---") return 'J';
  if (morse == "-.-") return 'K';   if (morse == ".-..") return 'L';
  if (morse == "--") return 'M';    if (morse == "-.") return 'N';
  if (morse == "---") return 'O';   if (morse == ".--.") return 'P';
  if (morse == "--.-") return 'Q';  if (morse == ".-.") return 'R';
  if (morse == "...") return 'S';   if (morse == "-") return 'T';
  if (morse == "..-") return 'U';   if (morse == "...-") return 'V';
  if (morse == ".--") return 'W';   if (morse == "-..-") return 'X';
  if (morse == "-.--") return 'Y';  if (morse == "--..") return 'Z';
  
  if (morse == ".----") return '1'; if (morse == "..---") return '2';
  if (morse == "...--") return '3'; if (morse == "....-") return '4';
  if (morse == ".....") return '5'; if (morse == "-....") return '6';
  if (morse == "--...") return '7'; if (morse == "---..") return '8';
  if (morse == "----.") return '9'; if (morse == "-----") return '0';
  
  return '?'; 
}

void setup() {
  pinMode(buttonPin, INPUT); 
  Serial.begin(9600);
  
  lcd.begin(16, 2); 
  lcd.setBacklight(HIGH);
  lcd.print("Morse Input Ready");
  delay(1500);
  lcd.clear();
  
  releasedTime = millis();
}

void loop() {
  int buttonState = digitalRead(buttonPin);
  unsigned long now = millis();

  // 1. 스위치를 막 누른 순간
  if (buttonState == HIGH && !isPressed) {
    pressedTime = now;
    isPressed = true;
    isCleared = false; // 누르기 시작할 때 플래그 초기화
  }
  
  // 누르고 있는 동안 실시간 체크 (3초 이상 눌렀는지 확인)
  if (buttonState == HIGH && isPressed && !isCleared) {
    if (now - pressedTime >= clearThreshold) {
      lcd.clear();
      lcd.print("RESET...");
      delay(500);
      lcd.clear();
      
      // 모든 데이터 버퍼 비우기
      morseCode = "";
      lcdBuffer = "";
      isCleared = true; // 버튼을 뗄 때까지 중복 리셋 방지
    }
  }
  
  // 2. 스위치를 뗀 순간 (정상 입력 처리)
  else if (buttonState == LOW && isPressed) {
    unsigned long duration = now - pressedTime;
    isPressed = false;
    releasedTime = now;

    // 만약 이미 3초가 지나 화면이 비워진 상태라면, 점/선 입력을 처리하지 않고 패스
    if (!isCleared && duration > 30) {
      if (duration < dotThreshold) {
        morseCode += ".";  
      } else {
        morseCode += "-";  
      }
      
      lcd.setCursor(0, 1);
      lcd.print("Code: " + morseCode + "    ");
    }
  }

  // 3. 글자 완성 처리 (1초 대기)
  if (!isPressed && morseCode.length() > 0) {
    if (now - releasedTime > letterTimeout) {
      char decodedChar = morseToChar(morseCode);
      
      if (decodedChar != '?') {
        lcdBuffer += decodedChar;
        
        if (lcdBuffer.length() > 16) {
          lcdBuffer = lcdBuffer.substring(1); 
        }
        
        lcd.setCursor(0, 0);
        lcd.print(lcdBuffer);
      }
      
      morseCode = "";
      lcd.setCursor(0, 1);
      lcd.print("                "); 
    }
  }
  
  // 4. 자동 공백 처리 (3초 대기)
  if (!isPressed && morseCode.length() == 0 && lcdBuffer.length() > 0 && !lcdBuffer.endsWith(" ")) {
    if (now - releasedTime > wordTimeout) {
      lcdBuffer += " ";
      lcd.setCursor(0, 0);
      lcd.print(lcdBuffer);
    }
  }
}
