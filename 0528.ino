#include <PulseSensorPlayground.h>

#define PULSE_SENSOR_COUNT 2

const int PULSE_INPUT0 = A0;
const int PULSE_INPUT1 = A1;

const int THRESHOLD0 = 550;
const int THRESHOLD1 = 550;

const unsigned long TIMEOUT = 5000;  // 超时时间：5 秒无输入归零

PulseSensorPlayground pulseSensor(PULSE_SENSOR_COUNT);

unsigned long lastBeatTime[2] = {0, 0};
unsigned long lastValidBeat[2] = {0, 0};  // 记录上次有效心跳时间
unsigned long beatInterval[2] = {0, 0};
int bpmValues[2] = {0, 0};

void setup() {
  Serial.begin(9600);  // 和 Processing 端一致

  // 设置传感器 0
  pulseSensor.analogInput(PULSE_INPUT0, 0);
  pulseSensor.setThreshold(THRESHOLD0, 0);

  // 设置传感器 1
  pulseSensor.analogInput(PULSE_INPUT1, 1);
  pulseSensor.setThreshold(THRESHOLD1, 1);

  pulseSensor.begin();
}

void loop() {
  unsigned long now = millis();

  for (int i = 0; i < PULSE_SENSOR_COUNT; i++) {
    if (pulseSensor.sawStartOfBeat(i)) {
      beatInterval[i] = now - lastBeatTime[i];
      lastBeatTime[i] = now;
      lastValidBeat[i] = now;

      int bpm = 60000 / beatInterval[i];
      if (bpm >= 40 && bpm <= 180) {
        bpmValues[i] = bpm;
      }
    }

    // 超过 TIMEOUT 时间未检测到新心跳，自动归零
    if (now - lastValidBeat[i] > TIMEOUT) {
      bpmValues[i] = 0;
    }
  }

  // 每 100ms 输出一次数据
  static unsigned long lastPrint = 0;
  if (now - lastPrint >= 100) {
    lastPrint = now;
    Serial.print(bpmValues[0]);
    Serial.print(",");
    Serial.println(bpmValues[1]);
  }

  delay(10);  // 减少 CPU 占用
}
