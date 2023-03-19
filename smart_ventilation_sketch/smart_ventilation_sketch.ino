/*
 * Sm@rt ventilation v.: 1.0.0.0 b
 * Автоматическое управление вентилятором вытяжного шкафа
 * Реле срабатыват при получении сигнала от датчиков движения или при принудительном включении тумблером или по таймауту
 * используется Low Level Trigger relay: HIGH - выключить, LOW - включить
 * Sm@rtDevice 03.01.2023
*/

#define DI_PIN_MOVE_DETECTOR_1 10            // пин, к которому подключен датчик движения 1 D10 Собака
#define DI_PIN_MOVE_DETECTOR_2 11            // пин, к которому подключен датчик движения 2 D11 Кошка
#define DI_PIN_FAN_TUMBLER 9                 // пин, к которому подключен тумблер для принудительного включения вентилятора; схема подключения: DI_PIN_FAN_TUMBLER --> КНОПКА --> GND, используется внутренний PULL_UP резистор
#define DO_PIN_FAN_RELAY 13                  // пин, выход на реле вентилятора D13

#define TIME_OUT_FAN_OFF 60000               // таймаут отключения вентилятора после пропадания сигнала с датчика движения (мс.), def: 60000
#define TIME_OUT_FAN_IDLE 60000*60           // таймаут простоя вентилятора (мс.), def: 60000*60 = 1 час

#define DEBUG_MODE

#include "VirtualButton.h"
VButton fan_tumbler;                         // тумблер принудительного включения вентилятора

unsigned long just_now;                      // текущий момент времени
unsigned long fan_work_timer;                // время работы вентилятора
boolean fan_on = false;                      // состояние вентилятора вкл/откл

void setup() {
  digitalWrite(DO_PIN_FAN_RELAY, HIGH);      // при старте вентилятор выключен
  pinMode(DI_PIN_MOVE_DETECTOR_1, INPUT);
  pinMode(DI_PIN_MOVE_DETECTOR_2, INPUT);
  pinMode(DI_PIN_FAN_TUMBLER, INPUT_PULLUP); // внутренняя подтяжка на пин тумблера

  just_now = millis();                       // max: 4 294 967 295
  fan_work_timer = just_now;                 

#ifdef DEBUG_MODE
  Serial.begin(9600);
  Serial.println("Start program");
#endif
}

void loop() {
  //fan_tumbler.tick();
  fan_tumbler.poll(!digitalRead(DI_PIN_FAN_TUMBLER)); // передаём значение пина в poll, 1 - кнопка нажата, 0 - не нажата, поэтому инверсия

  just_now = millis();

  if(fan_work_timer > just_now) {
    fan_work_timer = just_now;
  }

  if (fan_tumbler.hold()) { // принудительно включен тумблером
#ifdef DEBUG_MODE
    Serial.println("Tumbler is ON");
#endif
    switchOn();
  } else if(/*!fan_on*/ digitalRead(DI_PIN_MOVE_DETECTOR_1) == HIGH || digitalRead(DI_PIN_MOVE_DETECTOR_2) == HIGH) { // есть сигнал с датчиков движения, включаем
    switchOn();
  } else if (fan_on && (just_now - fan_work_timer > TIME_OUT_FAN_OFF)) { // движения нет, вышел таймаут, отключаем
    switchOff();
  } else if (!fan_on && (just_now - fan_work_timer > TIME_OUT_FAN_IDLE)) { // долго не работал, включаем, пусть поработает (периодический запуск 1 раз в час)
    switchOn();
  }

#ifdef DEBUG_MODE
  Serial.print("Move detector 1: "); Serial.println(digitalRead(DI_PIN_MOVE_DETECTOR_1));
  Serial.print("Move detector 2: "); Serial.println(digitalRead(DI_PIN_MOVE_DETECTOR_2));

  if (digitalRead(DI_PIN_MOVE_DETECTOR_1) == HIGH) {
   Serial.println("Move 1 detected");
  }

  if (digitalRead(DI_PIN_MOVE_DETECTOR_2) == HIGH) {
   Serial.println("Move 2 detected");
  }

  if (fan_on) {
   Serial.println("Fan is working...");
  }

  delay(1000);
#endif
}

void switchOn() {
#ifdef DEBUG_MODE
  if(!fan_on) {
    Serial.println("Fan switched ON");
  }
#endif

  fan_on = true;
  fan_work_timer = just_now;
  digitalWrite(DO_PIN_FAN_RELAY, LOW);
}

void switchOff() {
#ifdef DEBUG_MODE
  if(fan_on) {
   Serial.println("Fan switched OFF");
  }
#endif

  fan_on = false;
  fan_work_timer = just_now;
  digitalWrite(DO_PIN_FAN_RELAY, HIGH);
}
