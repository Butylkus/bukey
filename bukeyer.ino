#include <EEPROM.h>
#include <GyverEncoder.h>


//////////////////////////////////////////////////
////                BuKeyer                 //////
////              ver. 240530               //////
////             Author: R3PLN              //////
////          https://t.me/butpub           //////
//////////////////////////////////////////////////



//////////////////////////////////////////////////
////      Базовые настройки и умолчания     //////
//////////////////////////////////////////////////

const int key_pin = 13; //пин, на который пойдёт + при воспроизведении
const int dot_paddle = 5; //пин левого лепестка
const int dash_paddle = 6; //пин правого лепестка
const int buzzer_pin = 9; //пин бузера самоконтроля


const int left = 3;
const int right = 2;
const int buttenc = 4;

int key_speed = 75; //скорость в минус первой степени, она же длительность точки

bool self_control = true; // включить буззер самоконтроля? тру или фолс
bool keying = true; // тру - режим ключа, фолс - "только писк", тренировка или чо там 
bool test_mode = false; //тру - автоцикуляние ТЕСТ, фолс - ЦЩЦЩДЕ

// Ещё пара переменных для внутреннего использования
int menu = 0; //пункты меню
int tone_mem; //тон сигнала буззера из памяти

//Для перевода строк в компорте и распознавалки надо уметь считать паузы.
//Это и есть переменные счётчика. Разница между ними больше стандартных пауз означает сигнал к распознаванию
unsigned long start_time; // базовый счётчик - начало
unsigned long stop_time; // базовый счётчик - конец
unsigned long counter; // это просто разница между ними

bool sent_flag = false; // флаг "послано в порт", чтобы не срать в порт постоянно
bool settings_mode = false; //флаг режима настроек ключа

bool sending = false; //флаг "сейчас идёт сигнал"
bool pausing = false; //флаг "пауза между сигналами"
unsigned long send_timer; //таймер передачи сигналов


//Переменная, которая распознаёт коды
String symbol_code = "";



//CQ CQ DE
#define normalCQ dash(); dot(); dash(); dot(); play_pause(); dash(); dash(); dot(); dash(); play_space(); dash(); dot(); dash(); dot(); play_pause(); dash(); dash(); dot(); dash(); play_space(); dash(); dot(); dot(); play_pause() ; dot(); play_space();
//UB3PEQ
#define Callsign dot(); dot(); dash(); play_pause();  dash(); dot();  dot(); dot(); play_pause(); dot(); dot(); dot(); dash(); dash(); play_pause();    dot(); dash(); dash(); dot(); play_pause(); dot(); play_pause(); dash(); dash(); dot(); dash(); play_space();
//PSE K
#define CQinvite dot(); dash(); dash(); dot(); play_pause(); dot();dot();dot(); play_pause(); dot(); play_pause(); dash(); dot(); dash();
//TEST
#define CQtest dash(); play_pause(); dot(); play_pause(); dot();dot();dot(); play_pause(); dash();




//Настраиваем энкодер...
Encoder enc1(left, right, buttenc, TYPE2);


int key_tone = 600; //тон для бузера самоконтроля. ноты идут по порядку согласно известным законам. По дефолту 440 (ля 4й октавы).


void setup() {
  pinMode(key_pin, OUTPUT);
  pinMode(buzzer_pin, OUTPUT);
  pinMode(dash_paddle, INPUT_PULLUP);
  pinMode(dot_paddle, INPUT_PULLUP);

  //А теперь прочтём данные из постоянной памяти и используем их, если они есть.
  EEPROM.get(10, tone_mem);
  byte key_speed_mem = byte(EEPROM.read(20)); // Скорость ключевания (на досуге переписать эту строку на гет)
  EEPROM.get(30, self_control); // Включен ли буззер в настройках?

  //Проверяем значения в памяти
  if (tone_mem<400 or tone_mem>1500){
    tone_mem = 600; // если значения номера нет в памяти или оно невменяемое, то ставим по умолчанию
    }
  key_tone = tone_mem; // Если значение нормальное, то принимаем настроенный тон
    

  if (key_speed_mem<18 or key_speed_mem>250){
    key_speed = 75; // если скорость в памяти невменяемая, то ставим 75 мсек
    }
  else{
    key_speed = key_speed_mem; // Если значение нормальное, то принимаем его
    }
  
  
  Serial.begin(9600);

  
//  Serial.print(key_tone);
//  Serial.println(tone_mem);
//  Serial.print(key_speed);
//  Serial.print(key_speed_mem);
//  if (self_control) { Serial.println("включен");} else { Serial.println("выключен");}
  Serial.println("Ключ готов к работе!");




  
  }



void dot() {
  if (keying) {digitalWrite(key_pin, 1);}
  if (self_control) {tone(buzzer_pin, key_tone);}
  delay(key_speed);
  if (keying) {digitalWrite(key_pin, 0);}
  noTone(buzzer_pin);
  delay(key_speed);
  //Serial.print("*");
  }

void dash() {
  if (keying) {digitalWrite(key_pin, 1);}
  if (self_control) {tone(buzzer_pin, key_tone);}
  delay(key_speed*3);
  if (keying) {digitalWrite(key_pin, 0);}
  noTone(buzzer_pin);
  delay(key_speed);
  //Serial.print("-");
  }

//Пауза между символами (три точки)
void play_pause() {
  sending = false;
  pausing = true;
  send_timer = millis();
  while (pausing == true){
    if ((millis() - send_timer) < key_speed*2){
      pausing = true;
      sending = false;
      }
    else{
      pausing = false;
      sending = false;
      }
  }
  //Serial.println("");
}

//пауза между слвоами (семь точек)
void play_space() {
  sending = false;
  pausing = true;
  send_timer = millis();
  while (pausing == true){
    if ((millis() - send_timer) < key_speed*6){
      pausing = true;
      }
    else{
        pausing = false;
      }
  }
  //Serial.println("");
}


void play_normal_cq() {
    normalCQ
    Callsign Callsign
    CQinvite
  }

void play_test_cq() {
    Callsign
    CQtest
  }




void loop() {
  
//////////////////////////////////////////////////
//// Тут проверяется и происходит настройка //////
//////////////////////////////////////////////////



  if (settings_mode == true) {
    while (menu){
      settings();
    }
  }

  // Переход в настройки, если мы в обычном режиме.
  else if (enc1.isHolded() && settings_mode == false) {

  //Перечитаем для начала еепром
    EEPROM.get(10, key_tone); // Тон буззера
    EEPROM.get(20, key_speed); // Тон буззера

    //// EEPROM.read(30); - не читаем настройку, если окажется выключенным, то настроиться будет невозможно без ОС.
    self_control = true; //Включаем буззер принудительно!
    keying = false; // Отключаем ключевание. Зачем в эфир гадить? =)
    Serial.println("Переходим в настройки");
    settings_mode = true;
    //запиливаем характерный звук
    for (int i=400; i<1500; i=i+2) {
      tone(buzzer_pin,i);
      delay (1);
    }
    noTone(buzzer_pin);
    //переход в меню 1
    menu = 1;  play_space(); play_space(); dot();
  }



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

  enc1.tick(); //рабочий ход энкодера
  
  if (enc1.isDouble() and settings_mode == false){
    Serial.println("Два клика - цикуляем");
    if (test_mode) {play_test_cq();}
    else{play_normal_cq();}
  }
  if (enc1.isSingle() and settings_mode == false){
    Serial.println("Клик - даю позывной");
    Callsign
  }


  if (settings_mode == false and enc1.isLeft() and key_speed < 252) {
    Serial.println("ТИК! WPM---");
    key_speed = key_speed + 3;  
    float wpm = 60000/(50*key_speed);
    Serial.println(key_speed);
    Serial.println(wpm);
  }

  if (settings_mode == false and enc1.isRight() and key_speed > 20) {
    Serial.println("ТИК! WPM---");
    key_speed = key_speed - 3;
    float wpm = 60000/(50*key_speed);
    Serial.println(key_speed);
    Serial.println(wpm);
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




void settings(){
/* Структура меню:
+1. Режим: CQ < > TEST
+2. Самоконтроль: выкл < > вкл
+3. Скорость: (перезапись в память)
+4. Тон: ниже < > выше


*/



//Serial.println(menu);
enc1.tick();

  if (enc1.isRightH() and menu<4){
    menu++;
      for(int i=menu; i>0; i--){
        dot();
      }
    Serial.println(menu);
  }
  if (enc1.isLeftH() and menu>1){
    menu--;
      for(int i=menu; i>0; i--){
        dot();
      }
    Serial.println(menu);
  }

//Переключение режимов
  if (settings_mode and menu==1 and enc1.isRight()) {
    Serial.print("ТЕСТ");
    test_mode = true;
    CQtest
  }
  if (settings_mode and menu==1 and enc1.isLeft()) {
    Serial.print("НОРМ");
    test_mode = false;
    dash(); dot(); dash(); dot(); play_pause(); dash(); dash(); dot(); dash();
  }


//Самоконтроль, включение-выключение буззера. 
  if (settings_mode and menu==2 and enc1.isRight()) {
    self_control = true;
    dot(); dash();
    Serial.print("Буззер включен");
  }
  if (settings_mode and menu==2 and enc1.isLeft()) {
    dash(); dot();
    self_control = false;
    Serial.print("Буззер выключен");
  }
  
//Скорость ключевания в памяти
  if (settings_mode and menu==3 and enc1.isRight()) {
    if (key_speed > 21){ 
      key_speed = key_speed - 1;
      dot();dot();dot();
      Serial.print("Скорость увеличена до ");
      Serial.println(key_speed);
    } else {
      Serial.println("Придержи коней, ковбой!");
    }
  }
  if (settings_mode and menu==3 and enc1.isLeft()) {
    if (key_speed < 250){ 
      key_speed = key_speed + 1;
      dot();dot();dot();
      Serial.print("Скорость уменьшена до ");
      Serial.println(key_speed);
    } else {
      Serial.println("Ну нельзя так слоупочить!");
    }
  }
  
//Тест скорости из памяти - надо нажать энкодер.
  if (settings_mode and menu==3 and enc1.isSingle()) {
    Callsign
  }

//Тон самоконтроля в памяти
  if (settings_mode and menu==4 and enc1.isRight() and key_tone < 1501) {
     if (key_tone > 1500 ){
        Serial.println("Комаров мало?");
     } else {
        key_tone = key_tone + 10 ;
        Serial.print("Частота буззера: ");
        Serial.println(key_tone);
        tone(buzzer_pin, key_tone); //Для самоконтроля нужно
        delay(20); // Чтоб слишком быстро не менялись ноты
     }
  }
  if (settings_mode and menu==4 and enc1.isLeft() and key_tone > 399) {
     if (key_tone < 400 ){
        Serial.println("Воу-воу, инфразвука нам не нать!");
     } else {
        key_tone = key_tone - 10 ;
        Serial.print("Частота буззера: ");
        Serial.println(key_tone);
        tone(buzzer_pin, key_tone); //Для самоконтроля нужно
        delay(20); // Чтоб слишком быстро не менялись ноты
     }
  }




  
  if (enc1.isHolded() && settings_mode == true) {   // если кнопка удерживается
    Serial.println("Сохраняем настройки"); 
    EEPROM.put(10, key_tone);
    EEPROM.put(20, key_speed);
    EEPROM.put(30, self_control);
    settings_mode = false;
    menu = 0;
    keying = true; // Включаем ключевание обратно
    for (int i=1500; i>400; i--) {
      tone(buzzer_pin,i);
      delay (1);
      }
      noTone(buzzer_pin);
    setup();
  }

}
      
