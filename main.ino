#define SOUND_PIN 11
#define PLAYERS 2

#include "Arduino.h"
extern HardwareSerial Serial;
#include "mario.h"

int buttonPins[PLAYERS] = {12, 13};
int ledPins[PLAYERS] = {10, 9};

class SoundService {
  public:
    void setup() {
      pinMode(SOUND_PIN, OUTPUT);
    }

    void playWin() {
      tone(SOUND_PIN, 4000, 1000);
    }

    void playLose() {
      tone(SOUND_PIN, 3500, 500);
      delay(500);
      tone(SOUND_PIN, 3000, 500);
      delay(500);
      tone(SOUND_PIN, 1500, 500);
      delay(500);
    }


    void playStart() {
      tone(SOUND_PIN, 3000, 250);
    }

    void playMario() {
      sing(SOUND_PIN);
    }
};

class LedSerivce {
  public:
    void setup() {
      for (int i = 0; i < PLAYERS; i ++) {
        pinMode(ledPins[i], OUTPUT);
      } 
    }

    void onForPlayer(int playerIndex) {
      digitalWrite(ledPins[playerIndex], HIGH);
    }

    void onForAll() {
      for (int i = 0; i < PLAYERS; i ++) {
        digitalWrite(ledPins[i], HIGH);
      } 
    }

    void offForAll() {
      for (int i = 0; i < PLAYERS; i ++) {
        digitalWrite(ledPins[i], LOW);
      } 
    }

    void offForPlayer(int playerIndex) {
      digitalWrite(ledPins[playerIndex], LOW);
    }

    void blinkForPlayer(int playerIndex, int delayMs) {
      digitalWrite(ledPins[playerIndex], HIGH);
      delay(delayMs);
      digitalWrite(ledPins[playerIndex], LOW);
    }
};

class ControlService {
  public:
    bool stateMap[PLAYERS] = {false, false};
    bool someIsPressed = false;
    bool bothIsPressed = false;

    void setup() {
      for (int i = 0; i < PLAYERS; i ++) {
        pinMode(buttonPins[i], INPUT_PULLUP);
      }
    }

    void poll() {
      setStates();
    }
  
  private:
    void setStates() {
      bool isPressed = false;
      int pressedCount = 0;

      for (int player = 0; player < PLAYERS; player++) {
        if (!digitalRead(buttonPins[player])) {
          pressedCount++;
          isPressed = true;
          stateMap[player] = true;
        } else {
          stateMap[player] = false;
        }
      }
      bothIsPressed = pressedCount == PLAYERS;
      someIsPressed = isPressed;
    }
};

class ScoreService {
  public:
    int scoreMap[PLAYERS] = {0, 0};

    int getWinner() {
      for (int i = 0; i < PLAYERS; i++) {
        if (scoreMap[i] == 10) {
          return i;
        }
      }
      return -1;
    }

    void reset() {
      for (int i = 0; i < PLAYERS; i++) {
        scoreMap[i] = 0;
      }
    }

    void add(int player) {
      scoreMap[player] += 1;
    }

    void remove(int player) {
      scoreMap[player] -= 1;
    }

    void print() {
      for (int i = 0; i < PLAYERS; i++) {
        Serial.print("Player ");
        Serial.print(i);
        Serial.print(" is ");
        Serial.println(scoreMap[i]);
      };
    }
};

SoundService sound;
LedSerivce led;
ControlService control;
ScoreService score;

void setup() {
  Serial.begin(4600);
  sound.setup();
  led.setup();
  control.setup();
}

void loop() {
  delay(random(1000, 7000));
  control.poll();

  // check cheaters
  if (control.someIsPressed) {
    for (int i = 0; i < PLAYERS; i ++) {
      if (control.stateMap[i]) {
        led.onForPlayer(i);
        sound.playLose();
        led.offForPlayer(i);
        score.remove(i);
      }
    }
    score.print();
    return;
  }

  sound.playStart();

  for (int player = 0; ; player = (player + 1) % PLAYERS) {
    // draw checks
    for (int i = 0; i < 4000; i++) {
      control.poll();
    }

    if (!control.someIsPressed) continue;

    // draw
    if (control.bothIsPressed) {
      led.onForAll();
      delay(800);
      led.offForAll();
      break;
    }
  
    for (int i = 0; i < PLAYERS; i ++) {
      if (control.stateMap[i]) {
        sound.playWin();
        led.blinkForPlayer(i, 800);
        score.add(i);
        score.print();
      }
    }

    // check totally win
    int totalWinner = score.getWinner();
    if (totalWinner != -1) {
      led.onForPlayer(totalWinner);
      sound.playMario();
      led.offForPlayer(totalWinner);
      score.reset();
    }

    break;
  }
}