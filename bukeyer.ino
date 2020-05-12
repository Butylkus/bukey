#include "pitches.h"
#include "IRLremote.h"
#include <EEPROM.h>




// Базовые настройки по умолчанию
int key_tone = tones[35]; //тон для бузера самоконтроля. ноты идут по порядку согласно известным законам. По дефолту 440 (ля 4й октавы).
byte key_speed = 100; //скорость в минус первой степени, она же длительность точки

const int key_pin = 10; //пин, на который пойдёт + при воспроизведении
const int dot_paddle = 3; //пин левого лепестка
const int dash_paddle = 4; //пин правого лепестка
const int buzzer_pin = 9; //пин бузера самоконтроля
#define ir_pin 2 //пин ик-пульта. Ну а чо? энкодер не всегда в наличии, а пультов и приёмников у всех пачка есть!

bool self_control = true; // включить буззер самоконтроля? тру или фолс
bool keying = true; // тру - режим ключа, фолс - "только писк", тренировка или чо там 

// Ещё пара переменных для внутреннего использования

//Для перевода строк в компорте и распознавалки надо уметь считать паузы.
//Это и есть переменные счётчика. Разница между ними больше стандартных пауз означает сигнал к распознаванию
unsigned long start_time; // базовый счётчик - начало
unsigned long stop_time; // базовый счётчик - конец
unsigned long counter; // это просто разница между ними

bool sent_flag = false; // флаг "послано в порт", чтобы не срать в порт постоянно
bool settings_mode = false; //флаг режима настроек ключа

//Переменная, которая распознаёт коды
String symbol_code = "";

//Переменная для номера тона буззера
byte tone_num;

//кнопки на пульте и их коды
#define BUTT_UP     0xE51CA6AD
#define BUTT_DOWN   0xD22353AD
#define BUTT_LEFT   0x517068AD
#define BUTT_RIGHT  0xAC2A56AD
#define BUTT_OK     0x1B92DDAD
#define BUTT_1      0x68E456AD
#define BUTT_2      0xF08A26AD
#define BUTT_3      0x151CD6AD
#define BUTT_4      0x18319BAD
#define BUTT_5      0xF39EEBAD
#define BUTT_6      0x4AABDFAD
#define BUTT_7      0xE25410AD
#define BUTT_8      0x297C76AD
#define BUTT_9      0x14CE54AD
#define BUTT_0      0xC089F6AD
#define BUTT_STAR   0xAF3F1BAD
#define BUTT_HASH   0x38379AD

//магия для ик-пульта...
CHashIR IRLremote;
uint32_t IRdata;





void setup() {
  pinMode(key_pin, OUTPUT);
  pinMode(buzzer_pin, OUTPUT);
  pinMode(dash_paddle, INPUT_PULLUP);
  pinMode(dot_paddle, INPUT_PULLUP);

  //А теперь прочтём данные из постоянной памяти и используем их, если они есть.
  tone_num = int(EEPROM.read(10));
  key_tone = tones[tone_num]; // ТОн буззера
  key_speed = int(EEPROM.read(20)); // Скорость ключевания

  Serial.begin(9600);
  IRLremote.begin(ir_pin);
  Serial.println("ИК-пульт вроде настроен...");
  Serial.print("Тональность ключа ");
  Serial.print(key_tone);
  Serial.println(" Гц...");
  Serial.print("Длительность точки ");
  Serial.print(key_speed);
  Serial.println(" мс...");
  Serial.println("Ключ готов к работе!");
  }



void dot() {
  if (keying) {digitalWrite(key_pin, 1);}
  if (keying) {tone(buzzer_pin, key_tone);}
  delay(key_speed);
  if (keying) {digitalWrite(key_pin, 0);}
  noTone(buzzer_pin);
  delay(key_speed);
  Serial.print("*");
  }

void dash() {
  if (keying) {digitalWrite(key_pin, 1);}
  if (keying) {tone(buzzer_pin, key_tone);}
  delay(key_speed*3);
  if (keying) {digitalWrite(key_pin, 0);}
  noTone(buzzer_pin);
  delay(key_speed);
  Serial.print("-");
  }

void play_pause() {
  delay(key_speed*3);
  }







void loop() {
  if (digitalRead(dot_paddle) == LOW or digitalRead(dash_paddle) == LOW) {
    sent_flag = false;
    if (counter > key_speed*3) {
      start_time = millis();
    }
    
    if (digitalRead(dot_paddle) == LOW) {
      dot();
      //Serial.print(digitalRead(dot_paddle));
      symbol_code = symbol_code + "0";
      }

    if (digitalRead(dash_paddle) == LOW) {
      dash();
      //Serial.print(digitalRead(dash_paddle));
      symbol_code = symbol_code + "1";
      }

    delay(1);

  }
  
  if ((digitalRead(dot_paddle) == HIGH) and (digitalRead(dash_paddle) == HIGH)) {
    stop_time = millis();
    counter = stop_time - start_time;
    if (counter > key_speed*3 and sent_flag == false) {
      Serial.print(" : ");
      Serial.println(decode_it(symbol_code)); //декодируй набранное
      sent_flag = true;
      symbol_code = "";

    }
  }
  remoteTick();
}


//Честно спизжено у Гайвера =)
void remoteTick() {
    auto data = IRLremote.read();
    IRdata = data.command;
    bool ir_flag = true;
    bool eeprom_flag = false;

  if (ir_flag) { // если данные пришли
    switch (IRdata) {
      // режимы
      
      case BUTT_1:
        break;
        
      case BUTT_2:
        break;
        
      case BUTT_3:
        break;
        
      case BUTT_4:
        break;
        
      case BUTT_5:
        break;
        
      case BUTT_6:
        break;
        
      case BUTT_7:
        break;
        
      case BUTT_8:
        break;
        
      case BUTT_9: 
        break;
        
      case BUTT_0: 
        break;
        
      case BUTT_STAR: 
        break;
        
      case BUTT_HASH:
        break;
        
      case BUTT_OK: //updateEEPROM();
        break;
        
      case BUTT_UP:
        if (key_speed >= 55){ 
          key_speed = key_speed - 5;
          Serial.println("");
          Serial.print("Скорость увеличена, длительность точки теперь: ");
          Serial.println(key_speed);
          }
         else {
            //play_fail();
            Serial.println("");
            Serial.println("А ты сам-то угонисси? Давай без выебонов ;)");
         }
         EEPROM.write(20, byte(key_speed));
         break;
      case BUTT_DOWN:
          if (key_speed <= 250){ 
          key_speed = key_speed + 5;
          Serial.println("");
          Serial.print("Скорость уменьшена, длительность точки теперь: ");
          Serial.println(key_speed);
          }
         else {
            //play_fail();
          Serial.println("");
          Serial.println("Не дури, куда уж медленнее?");
         }
         EEPROM.write(20, byte(key_speed));
         break;
      case BUTT_LEFT:
        if (tone_num < 1 ){ 
          Serial.println("");
          Serial.println("Воу-воу, инфразвука нам не нать!");
          }
         else {
          tone_num--;
          Serial.print("Сейчас частота буззера в герцах: ");
          Serial.println(tones[tone_num]);
         }
         EEPROM.write(10, byte(tone_num));
         key_tone = tones[tone_num];
         tone(buzzer_pin, key_tone);
         delay(key_speed*3);
         noTone(buzzer_pin);
         break;
      case BUTT_RIGHT:
        if (tone_num >= 79 ){ 
          Serial.println("");
          Serial.println("Увы, буззер больше 5 кГц не смогёт...");
          }
         else {
          tone_num++;
          Serial.print("Сейчас частота буззера в герцах: ");
          Serial.println(tones[tone_num]);
         }
         EEPROM.write(10, byte(tone_num));
         key_tone = tones[tone_num];
         tone(buzzer_pin, key_tone);
         delay(key_speed*3);
         noTone(buzzer_pin);
        break;
      default: eeprom_flag = false;   // если не распознали кнопку, не обновляем настройки!
        break;
    }
    ir_flag = false;
  }
}


String decode_it(String symbol_code) {
  if (symbol_code == "01") { return "A";    }
  else if (symbol_code == "1000") { return "B (Б)"; }
  else if (symbol_code == "1010") { return "C (Ц)"; }
  else if (symbol_code == "100") { return "D"; }
  else if (symbol_code == "0") { return "E"; }
  else if (symbol_code == "0010") { return "F"; }
  else if (symbol_code == "110") { return "G"; }
  else if (symbol_code == "0000") { return "H (Х)"; }
  else if (symbol_code == "00") { return "I"; }
  else if (symbol_code == "0111") { return "J (Й)"; }
  else if (symbol_code == "101") { return "K"; }
  else if (symbol_code == "0100") { return "L"; }
  else if (symbol_code == "11") { return "M"; }
  else if (symbol_code == "10") { return "N"; }
  else if (symbol_code == "111") { return "O"; }
  else if (symbol_code == "0110") { return "P"; }
  else if (symbol_code == "1101") { return "Q (Щ)"; }
  else if (symbol_code == "010") { return "R"; }
  else if (symbol_code == "000") { return "S"; }
  else if (symbol_code == "1") { return "T"; }
  else if (symbol_code == "001") { return "U"; }
  else if (symbol_code == "0001") { return "V (Ж)"; }
  else if (symbol_code == "011") { return "W (В)"; }
  else if (symbol_code == "1001") { return "X (Ь)"; }
  else if (symbol_code == "1011") { return "Y (Ы)"; }
  else if (symbol_code == "1100") { return "Z"; }
  else if (symbol_code == "11111") { return "0"; }
  else if (symbol_code == "01111") { return "1"; }
  else if (symbol_code == "00111") { return "2"; }
  else if (symbol_code == "00011") { return "3"; }
  else if (symbol_code == "00001") { return "4"; }
  else if (symbol_code == "00000") { return "5"; }
  else if (symbol_code == "10000") { return "6"; }
  else if (symbol_code == "11000") {  return "7";  }
  else if (symbol_code == "11100") {  return "8"; }
  else if (symbol_code == "11110") { return "9"; }
  else if (symbol_code == "1110") { return "Ч"; }
  else if (symbol_code == "1111") { return "Ш"; }
  else if (symbol_code == "00100") { return "Э"; }
  else if (symbol_code == "0011") { return "Ю"; }
  else if (symbol_code == "11011") { return "Ъ"; }
  else if (symbol_code == "0101") { return "Я"; }
  else if (symbol_code == "000000") { return "."; }
  else if (symbol_code == "010101") { return ","; }
  else if (symbol_code == "111000") { return ":"; }
  else if (symbol_code == "001100") { return "?"; }
  else if (symbol_code == "110011") { return "!"; }
  else if (symbol_code == "010010") { return "\""; }
  else if (symbol_code == "101010") { return ";"; }
  else if (symbol_code == "101101") { return "()"; }
  else if (symbol_code == "00101") { return "end"; }

  else { return symbol_code;}
  }
