#include "pitches.h"
#include <EEPROM.h>
#include "GyverButton.h"

// Базовые настройки по умолчанию
const int key_pin = 10; //пин, на который пойдёт + при воспроизведении
const int dot_paddle = 3; //пин левого лепестка
const int dash_paddle = 4; //пин правого лепестка
const int buzzer_pin = 9; //пин бузера самоконтроля
const int button_pin = 12; //Кнопка для настроек. из пина в землю!

int key_tone = tones[35]; //тон для бузера самоконтроля. ноты идут по порядку согласно известным законам. По дефолту 440 (ля 4й октавы).
byte key_speed = 100; //скорость в минус первой степени, она же длительность точки

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

//Переменная для номера тона буззера, номер элемента из файла pitches.
byte tone_num;


// флаг " ключ в режиме настройки". Нужен для работы кнопки
bool flag_config = false;
bool flag_speed = false;
bool flag_tone = false;
bool flag_buzzkey = false;



//Запускаем кнопульку...
GButton butt1(button_pin);



void setup() {
  pinMode(key_pin, OUTPUT);
  pinMode(buzzer_pin, OUTPUT);
  pinMode(dash_paddle, INPUT_PULLUP);
  pinMode(dot_paddle, INPUT_PULLUP);

  //А теперь прочтём данные из постоянной памяти и используем их, если они есть.
  tone_num = int(EEPROM.read(10));
  key_tone = tones[tone_num]; // Тон буззера
  key_speed = int(EEPROM.read(20)); // Скорость ключевания
  self_control = bool(EEPROM.read(30)); // Включен ли буззер в настройках?
  
  Serial.begin(9600);

  
  butt1.setDebounce(30);        // настройка антидребезга (по умолчанию 80 мс)
  butt1.setTimeout(300);        // настройка таймаута на удержание (по умолчанию 500 мс)
  butt1.setClickTimeout(600);   // настройка таймаута между кликами (по умолчанию 300 мс)
  butt1.setType(HIGH_PULL);
  butt1.setDirection(NORM_OPEN);
  Serial.println("Кнопка настройки запилена...");
  Serial.print("Тональность ключа ");
  Serial.print(key_tone);
  Serial.println(" Гц...");
  Serial.print("Длительность точки ");
  Serial.print(key_speed);
  Serial.println(" мс...");
  Serial.print("Самоконтроль ");
  if (self_control) { Serial.println("включен");} else { Serial.println("выключен");}
  Serial.println("Ключ готов к работе!");
  }



void dot() {
  if (keying) {digitalWrite(key_pin, 1);}
  if (self_control) {tone(buzzer_pin, key_tone);}
  delay(key_speed);
  if (keying) {digitalWrite(key_pin, 0);}
  noTone(buzzer_pin);
  delay(key_speed);
  Serial.print("*");
  }

void dash() {
  if (keying) {digitalWrite(key_pin, 1);}
  if (self_control) {tone(buzzer_pin, key_tone);}
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
  
//////////////////////////////////////////////////
//// Тут проверяется и происходит настройка //////
//////////////////////////////////////////////////

  if (settings_mode == true) {
      settings();
  }

  // Переход в настройки, если мы в обычном режиме.
  else if (butt1.isHold() && settings_mode == false) {

  //Перечитаем для начала еепром
    tone_num = int(EEPROM.read(10));
    key_tone = tones[tone_num]; // ТОн буззера
    key_speed = int(EEPROM.read(20)); // Скорость ключевания
    //// EEPROM.read(30); - не читаем настройку, если окажется выключенным, то настроиться будет невозможно без ОС.
    self_control = true; //Включаем буззер принудительно!
    keying = false; // Отключаем ключевание. Зачем в эфир гадить? =)
    Serial.println("Переходим в настройки");
    settings_mode = true;
    for (int i=6; i<70; i++) {
      tone(buzzer_pin,tones[i]);
      delay (8);
    }
    noTone(buzzer_pin);
    delay (300);
  }

    //поднимаем флаги по кнопке
    if (butt1.isSingle()) {
      flag_speed = true;
      flag_tone = false;
      noTone(buzzer_pin);
      flag_buzzkey = false;
      Serial.println("Настройка скорости");
    }
    if (butt1.isDouble()) {
      flag_speed = false;
      flag_tone = true;
      flag_buzzkey = false;
      Serial.println("Настройка тона самоконтроля");
      }
    if (butt1.isTriple()) {
      flag_speed = false;
      flag_tone = false;
      flag_buzzkey = true;
      noTone(buzzer_pin);
      Serial.println("Настройка интерфейсов - передача вкл/выкл, самоконтроль вкл/выкл");
      }
      /*
   //Отладочная инфа
  Serial.print(flag_speed); 
  Serial.print(flag_tone);
  Serial.println(flag_buzzkey);
      */

//////////////////////////////////////////////////
/////// Основной рабочий цикл. Ключевание ////////
//////////////////////////////////////////////////

if (settings_mode == false) {
  
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
  
  if (digitalRead(dot_paddle) == HIGH and digitalRead(dash_paddle) == HIGH) {  // если лепестки не тронуты
    stop_time = millis(); // таймер в текущий момент
    counter = stop_time - start_time; //считаем время с момента отпускания лепестков...
    if (counter > key_speed*3 and sent_flag == false) { // если прошло три точки (как бы стандарт) и ничего не отправлялось ранее
      Serial.print(" : ");
      Serial.println(decode_it(symbol_code)); //декодируй набранное
      sent_flag = true; // флаг отправки
      symbol_code = ""; // сбрасываем в пустоту, чтоб не возникало косяков

    }
  }
}
//////////////////////////////////////////////////
////// Служебные тики сторонних библиотек ////////
//////////////////////////////////////////////////
  butt1.tick();
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


void settings(){
    //butt1.tick();

      if (digitalRead(dot_paddle) == LOW) {
          if (flag_speed == true) {
              if (key_speed >= 55){ 
                key_speed = key_speed - 5;
                Serial.println("");
                Serial.print("Скорость увеличена, длительность точки теперь: ");
                Serial.println(key_speed);
                dot(); dot(); dot();
              } else {
                Serial.println("");
                Serial.println("А ты сам-то угонисси? Давай без выебонов ;)");
              }
          } else if (flag_tone == true){
              if (tone_num < 1 ){ 
                  Serial.println("");
                  Serial.println("Воу-воу, инфразвука нам не нать!");
              } else {
                  tone_num--;
                  Serial.print("Сейчас частота буззера в герцах: ");
                  Serial.println(tones[tone_num]);
                  tone(buzzer_pin, tones[tone_num]); //Для самоконтроля нужно
              }

          } else if (flag_buzzkey == true){
               Serial.println("Самоконтроля не будет, буззер выключен");
               EEPROM.write(30, 0);
               delay(100);
          } 



          
      }

      
        if (digitalRead(dash_paddle) == LOW) {
            if (flag_speed == true) {
                if (key_speed <= 250){ 
                    key_speed = key_speed + 5;
                    Serial.println("");
                    Serial.print("Скорость уменьшена, длительность точки теперь: ");
                    Serial.println(key_speed);
                    dot(); dot(); dot();
                  } else {
                    Serial.println("");
                    Serial.println("Не дури, куда уж медленнее?");
                  }
            } else if (flag_tone == true) {
                if (tone_num >= 79 ){ 
                    Serial.println("");
                    Serial.println("Увы, буззер больше 5 кГц не смогёт...");
                } else {
                    tone_num++;
                    Serial.print("Сейчас частота буззера в герцах: ");
                    Serial.println(tones[tone_num]);
                    tone(buzzer_pin, tones[tone_num]); //Для самоконтроля нужно
                }
            } else if (flag_buzzkey == true){
               Serial.println("Самоконтроль есть, буззер включен");
               EEPROM.write(30, 1);
               delay(100);
            } 
        }

      
      
       //
      key_tone = tones[tone_num];//Это для контроля настроек. Пищит по скорости и по тону, как есть. Обратная связь, кароч! 

      
    
    /// выход из настроек
    if (butt1.isHold() && settings_mode == true) {   // если кнопка удерживается
      Serial.println("Сохраняем настройки");  
      EEPROM.write(10, byte(tone_num));
      EEPROM.write(20, byte(key_speed));
      //self_control = true; //Включаем буззер
      keying = true; // Включаем ключевание обратно
      
      settings_mode = false;
      for (int i=70; i>6; i--) {
        tone(buzzer_pin,tones[i]);
        delay (8);
      }
      noTone(buzzer_pin);
      delay (300);
      setup();
   } 
  
}
