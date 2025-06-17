// demo reel 100
// twinkle fox
#include <FastLED.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(7, 6, 10, 11, 12, 13);  // RS, E, D4, D5, D6, D7


#define PIN_LIGHT 3  // Arduino pin that connects to WS2812B
#define NUM_PIXELS 840  // The number of LEDs (pixels) on WS2812B
#define DELAY_INTERVAL 30 // 250ms pause between each pixel


unsigned long lastPulseUpdate = 0;
const unsigned long PULSE_UPDATE_INTERVAL = 5000;  // 每3秒更新一次
int cachedBPM1 = 0;
int cachedBPM2 = 0;



#define BRIGHTNESS 180     // 亮度设定
#define LED_TYPE WS2812B   // 灯带型号
#define COLOR_ORDER GRB    // 灯带颜色顺序
// CRGB leds[NUM_PIXELS];
CRGBArray<NUM_PIXELS> leds;

// float breath = 0;          // 呼吸值
// bool breathDirection = true;

#define LED_WIDTH 30
#define LED_HEIGHT 28


// pulse sensor
#include <PulseSensorPlayground.h>
#define PULSE_SENSOR_COUNT 2

const int PULSE_INPUT0 = A0;
const int PULSE_INPUT1 = A15;

const int THRESHOLD0 = 550;
const int THRESHOLD1 = 550;

const unsigned long TIMEOUT = 3000;  // 超时时间：1 秒无输入归零

PulseSensorPlayground pulseSensor(PULSE_SENSOR_COUNT);

unsigned long lastBeatTime[2] = {0, 0};
unsigned long lastValidBeat[2] = {0, 0};  // 记录上次有效心跳时间
unsigned long beatInterval[2] = {0, 0};
int bpmValues[2] = {0, 0};

bool resonanceTriggered = false;
unsigned long resonanceStartTime = 0;
const unsigned long RESONANCE_DURATION = 10000; // 10 秒
const unsigned long RESONANCE_ANIM_TIME = 3000; // 融合动画时长


unsigned long similarityStartTime = 0;
const unsigned long SIMILARITY_REQUIRED = 3000;
bool hasFlashed = false;


// twinkle fox default 
#define TWINKLE_SPEED 3
#define TWINKLE_DENSITY 5
#define SECONDS_PER_PALETTE  40
#define VOLTS          12
#define MAX_MA       4000
CRGB gBackgroundColor = CRGB::Black; 
#define AUTO_SELECT_BACKGROUND_COLOR 0
#define COOL_LIKE_INCANDESCENT 1

CRGBPalette16 gCurrentPalette;
CRGBPalette16 gTargetPalette;


void setup() {

  delay( 3000 ); //safety startup delay
  // FastLED.setMaxPowerInVoltsAndMilliamps( VOLTS, MAX_MA);
  
  // 设置传感器 0
  pulseSensor.analogInput(PULSE_INPUT0, 0);
  pulseSensor.setThreshold(THRESHOLD0, 0);

  // 设置传感器 1
  pulseSensor.analogInput(PULSE_INPUT1, 1);
  pulseSensor.setThreshold(THRESHOLD1, 1);


  pulseSensor.begin();

   

  FastLED.addLeds<LED_TYPE, PIN_LIGHT, COLOR_ORDER>(leds, NUM_PIXELS);
  FastLED.setBrightness(BRIGHTNESS);

  // turn on all pixels to red at the same time for two seconds

  FastLED.clear();
  FastLED.show();


  lcd.begin(16, 2);     // 初始化为 16 列 2 行
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("");

  chooseNextColorPalette(gTargetPalette);


}


void draw_pulse(int bpm1, int bpm2){
  int col = 14;
  int row = 15;

  FastLED.clear();

  if (bpm1==0&&bpm2==0){
    // CRGB c = CHSV(180, 255, 255);  // Default 蓝色
    // drawBreathCircle(14, 14, 3 + sin(millis() * 0.005) * 1.5, c);

    EVERY_N_SECONDS( SECONDS_PER_PALETTE ) { 
    chooseNextColorPalette( gTargetPalette ); 
    }
    
    EVERY_N_MILLISECONDS( 100 ) {
      nblendPaletteTowardPalette( gCurrentPalette, gTargetPalette, 8);
    }

    drawTwinkles( leds);


  }else{
    if (bpm1!=0&&bpm2!=0){
      CRGB bgColor = CHSV(100-20, 150, 45);

      for (int i = 0; i < NUM_PIXELS; i++) {
        leds[i] = bgColor;
      }

      CRGB c1 = bpmToColor(bpm1);  // 你可以自己实现这个函数，映射不同 bpm 到颜色
      float radius1 = map(bpm1, 40, 130, 2, 4);
      drawBreathCircle(14, 21, radius1 + sin(millis()*bpm1 *0.01 * 0.005) * 1.5, c1);

      CRGB c2 = bpmToColor(bpm2);  // 你可以自己实现这个函数，映射不同 bpm 到颜色
      float radius2 = map(bpm2, 40, 130, 2, 4);
      drawBreathCircle(14, 7, radius2 + sin(millis()*bpm2 *0.01 * 0.005) * 1.5, c2);
    }
    else{
      CRGB c;
      float radius = 1;
      int _bpm = 40;

      if (bpm1!=0){
        c = bpmToColor(bpm1);  // 你可以自己实现这个函数，映射不同 bpm 到颜色
        radius = map(bpm1, 40, 130, 2, 4);
        _bpm = bpm1;
        // drawBreathCircle(14, 14, radius + sin(millis() * 0.005) * 1.5, c1);
      }
      if (bpm2!=0){
        c = bpmToColor(bpm2);  // 你可以自己实现这个函数，映射不同 bpm 到颜色
        radius = map(bpm2, 40, 130, 2, 4);
        _bpm = bpm2;
        
      }
      // draw background color
      CHSV centerHSV = rgb2hsv_approximate(c);
      uint8_t baseHue = centerHSV.h;

      CRGB bgColor = CHSV(baseHue-20, 150, 45);

      for (int i = 0; i < NUM_PIXELS; i++) {
        leds[i] = bgColor;
      }

      drawBreathCircle(14, 14, radius + sin(millis()*_bpm*0.01 * 0.005) * 1.5, c);


    }

  }
  FastLED.show();
}


void loop() {
  Serial.begin(250000); 

  // pulse sensor 
  unsigned long now = millis();

  for (int i = 0; i < PULSE_SENSOR_COUNT; i++) {
    if (pulseSensor.sawStartOfBeat(i)) {
      beatInterval[i] = now - lastBeatTime[i];
      lastBeatTime[i] = now;
      lastValidBeat[i] = now;

      int bpm = 60000 / beatInterval[i];
      // int bpm = pulseSensor.getBeatsPerMinute();
      if (bpm >= 40 && bpm <= 130) {
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

    // show bpm on lcd
    if (!resonanceTriggered){
      
      lcd.setCursor(0, 0);
      lcd.print("USER1   USER2");
      lcd.setCursor(0, 1);
      if (bpmValues[0] > 0) {
        lcd.print(bpmValues[0]);
        if (bpmValues[0] < 100) lcd.print(" "); // 保持对齐
      } else {
        lcd.print("-- ");
      }

      lcd.setCursor(9, 1);
      
      if (bpmValues[1] > 0) {
        lcd.print(bpmValues[1]);
        if (bpmValues[1] < 100) lcd.print(" ");
      } else {
        lcd.print("-- ");
      }
    }

  }
  
  
  // 是否两个 BPM 都有效
  bool valid1 = bpmValues[0] >= 40 && bpmValues[0] <= 130;
  bool valid2 = bpmValues[1] >= 40 && bpmValues[1] <= 130;
  int diff = abs(bpmValues[0] - bpmValues[1]);

  // 共鸣模式尚未触发，检查是否满足“持续相似”条件
  if (!resonanceTriggered) {
    if (valid1 && valid2 && diff <= 5) {
      if (similarityStartTime == 0) {
        similarityStartTime = now;  // 记录相似开始时间
      } else if (now - similarityStartTime >= SIMILARITY_REQUIRED) {
        resonanceTriggered = true;
        resonanceStartTime = now;
        hasFlashed = false;  // 允许下次共鸣重新闪烁
      }
    } else {
      similarityStartTime = 0;  // 不再相似，重置
    }
  }

  // 共鸣模式中
  if (resonanceTriggered) {
  
    lcd.setCursor(0, 1);
    if (now - resonanceStartTime <= RESONANCE_DURATION) {
      drawResonance(bpmValues[0], bpmValues[1]);
      lcd.print("   Resonance! ");
    } else {
      lcd.clear();
      resonanceTriggered = false;
      similarityStartTime = 0;  // 清除延时触发器
      draw_pulse(bpmValues[0], bpmValues[1]);
    }
  } else {
      // 每 3 秒更新一次 cached BPM 用于绘制
      if ((bpmValues[0]!=0&&cachedBPM1==0)||(bpmValues[1]!=0&&cachedBPM2==0)){
        lastPulseUpdate = now;
        cachedBPM1 = bpmValues[0];
        cachedBPM2 = bpmValues[1];
        draw_pulse(cachedBPM1, cachedBPM2);
      }
      else if (now - lastPulseUpdate >= PULSE_UPDATE_INTERVAL ) {
        lastPulseUpdate = now;
        cachedBPM1 = bpmValues[0];
        cachedBPM2 = bpmValues[1];
        draw_pulse(cachedBPM1, cachedBPM2);
      } else {
        // 不刷新 BPM，只重绘上次的效果
        draw_pulse(cachedBPM1, cachedBPM2);
      }
  }



  delay(15);  // 减少 CPU 占用
}


void drawBreathCircle(int cx, int cy, float radius, CRGB centerColor) {
  // CHSV centerHSV;
  // rgb2hsv_approximate(centerColor, centerHSV);
  CHSV centerHSV = rgb2hsv_approximate(centerColor);
  uint8_t baseHue = centerHSV.h;

  // CRGB outerColor = CHSV(baseHue + 20, 130, 200);  // 外圈亮
  // CRGB bgColor    = CHSV(baseHue, 80, 30);         // 全屏背景更淡
  // CRGB bgColor = CHSV(baseHue-20, 150, 45);

  // for (int i = 0; i < NUM_PIXELS; i++) {
  //   leds[i] = bgColor;
  // }

  for (int x = 0; x < LED_WIDTH; x++) {
    for (int y = 0; y < LED_HEIGHT; y++) {
      float dx = x - cx;
      float dy = y - cy;
      float distToCenter = sqrt(dx * dx + dy * dy);

      float diff = distToCenter - radius;

      // 中心区域（最亮）
      if (diff <= -1.5) {
        leds[XY(x, y)] += centerColor;
      }
      // 中间渐变过渡区域（heatmap 样式）
      else if (diff > -1.5 && diff < 2.0) {
        float gradient = 1.0 - constrain((diff + 1.5) / 3.5, 0.0, 1.0);

        // 可改为不同色环：中间色 + 外圈色
        // CHSV outer = CHSV(hue(centerColor) + 20, 200, 200);  // 更亮/更偏黄的外环
        CHSV outer = CHSV(baseHue + 20, 200, 200);

        CRGB blendColor = blend(centerColor, outer, gradient * 255);

        blendColor.nscale8(255 * gradient);
        leds[XY(x, y)] += blendColor;
      }
      // 外部光晕（淡色+大范围扩散）
      else if (diff >= 2.0 && diff < 4.5) {
        float glow = 1.0 - constrain((diff - 2.0) / 2.5, 0.0, 1.0);

        // CHSV outerGlow = CHSV(hue(centerColor), 100, 150); // 柔光
        CHSV outerGlow = CHSV(baseHue, 100, 150);

        CRGB glowColor = outerGlow;
        glowColor.nscale8(130 * glow);
        leds[XY(x, y)] += glowColor;
      }
    }
  }
}



CRGB bpmToColor(int bpm) {
  int h;
  if (bpm <= 65) h = map(bpm, 40, 65, 200, 160);     // 蓝到绿
  else if (bpm <= 90) h = map(bpm, 66, 90, 160, 120); // 绿到橙
  else if (bpm <= 115) h = map(bpm, 91, 115, 120, 80);
  else h = map(bpm, 116, 130, 80, 40);                // 橙到红
  return CHSV(h, 255, 255);
}

int XY(int x, int y) {
  if (y % 2 == 0) {
    // 偶数行：从左到右
    return y * LED_WIDTH + x;
  } else {
    // 奇数行：从右到左
    return y * LED_WIDTH + (LED_WIDTH - 1 - x);
  }
}

void drawResonance(int bpm1, int bpm2) {
  if (!hasFlashed) {
    // 白色闪烁效果
    // for (int i = 0; i < NUM_PIXELS; i++) {
    //   leds[i] = CRGB::White;
    // }
    // FastLED.show();
    // delay(100);
    hasFlashed = true;
  }

  FastLED.clear();

  unsigned long now = millis();
  unsigned long elapsed = now - resonanceStartTime;

  float t = now * 0.001;
  float freq = 0.25;

  float baseRadius = map((bpm1 + bpm2) / 2, 50, 130, 2, 4);
  float radius = baseRadius + sin(TWO_PI * freq * t) * 1.5;

  // 用户各自颜色
  CRGB c1 = bpmToColor(bpm1);
  CRGB c2 = bpmToColor(bpm2);
  CRGB resonanceColor = CHSV(0, 255, 255);  // 红色融合

  if (elapsed < RESONANCE_ANIM_TIME) {
    // 融合动画阶段：3秒缓慢靠近
    float progress = elapsed / float(RESONANCE_ANIM_TIME);
    int y1 = (int)(7 + (14 - 7) * progress);   // 从上方向下移动
    int y2 = (int)(21 - (21 - 14) * progress); // 从下方向上移动

    drawBreathCircle(14, y1, radius, c2);
    drawBreathCircle(14, y2, radius, c1);
  } else {
    // 完全融合阶段：中间红色呼吸圈
    // drawBreathCircle(14, 14, radius, resonanceColor);
    CRGB c1 = bpmToColor(bpm1);
    CRGB c2 = bpmToColor(bpm2);
    drawResonanceFusionEffect(c1, c2);

  }

  FastLED.show();
}



void drawTwinkles( CRGBSet& L)
{
  // "PRNG16" is the pseudorandom number generator
  // It MUST be reset to the same starting value each time
  // this function is called, so that the sequence of 'random'
  // numbers that it generates is (paradoxically) stable.
  uint16_t PRNG16 = 11337;
  
  uint32_t clock32 = millis();

  // Set up the background color, "bg".
  // if AUTO_SELECT_BACKGROUND_COLOR == 1, and the first two colors of
  // the current palette are identical, then a deeply faded version of
  // that color is used for the background color
  CRGB bg;
  if( (AUTO_SELECT_BACKGROUND_COLOR == 1) &&
      (gCurrentPalette[0] == gCurrentPalette[1] )) {
    bg = gCurrentPalette[0];
    uint8_t bglight = bg.getAverageLight();
    if( bglight > 64) {
      bg.nscale8_video( 16); // very bright, so scale to 1/16th
    } else if( bglight > 16) {
      bg.nscale8_video( 64); // not that bright, so scale to 1/4th
    } else {
      bg.nscale8_video( 86); // dim, scale to 1/3rd.
    }
  } else {
    bg = gBackgroundColor; // just use the explicitly defined background color
  }

  uint8_t backgroundBrightness = bg.getAverageLight();
  
  for( CRGB& pixel: L) {
    PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
    uint16_t myclockoffset16= PRNG16; // use that number as clock offset
    PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
    // use that number as clock speed adjustment factor (in 8ths, from 8/8ths to 23/8ths)
    uint8_t myspeedmultiplierQ5_3 =  ((((PRNG16 & 0xFF)>>4) + (PRNG16 & 0x0F)) & 0x0F) + 0x08;
    uint32_t myclock30 = (uint32_t)((clock32 * myspeedmultiplierQ5_3) >> 3) + myclockoffset16;
    uint8_t  myunique8 = PRNG16 >> 8; // get 'salt' value for this pixel

    // We now have the adjusted 'clock' for this pixel, now we call
    // the function that computes what color the pixel should be based
    // on the "brightness = f( time )" idea.
    CRGB c = computeOneTwinkle( myclock30, myunique8);

    uint8_t cbright = c.getAverageLight();
    int16_t deltabright = cbright - backgroundBrightness;
    if( deltabright >= 32 || (!bg)) {
      // If the new pixel is significantly brighter than the background color, 
      // use the new color.
      pixel = c;
    } else if( deltabright > 0 ) {
      // If the new pixel is just slightly brighter than the background color,
      // mix a blend of the new color and the background color
      pixel = blend( bg, c, deltabright * 8);
    } else { 
      // if the new pixel is not at all brighter than the background color,
      // just use the background color.
      pixel = bg;
    }
  }
}


//  This function takes a time in pseudo-milliseconds,
//  figures out brightness = f( time ), and also hue = f( time )
//  The 'low digits' of the millisecond time are used as 
//  input to the brightness wave function.  
//  The 'high digits' are used to select a color, so that the color
//  does not change over the course of the fade-in, fade-out
//  of one cycle of the brightness wave function.
//  The 'high digits' are also used to determine whether this pixel
//  should light at all during this cycle, based on the TWINKLE_DENSITY.
CRGB computeOneTwinkle( uint32_t ms, uint8_t salt)
{
  uint16_t ticks = ms >> (8-TWINKLE_SPEED);
  uint8_t fastcycle8 = ticks;
  uint16_t slowcycle16 = (ticks >> 8) + salt;
  slowcycle16 += sin8( slowcycle16);
  slowcycle16 =  (slowcycle16 * 2053) + 1384;
  uint8_t slowcycle8 = (slowcycle16 & 0xFF) + (slowcycle16 >> 8);
  
  uint8_t bright = 0;
  if( ((slowcycle8 & 0x0E)/2) < TWINKLE_DENSITY) {
    bright = attackDecayWave8( fastcycle8);
  }

  uint8_t hue = slowcycle8 - salt;
  CRGB c;
  if( bright > 0) {
    c = ColorFromPalette( gCurrentPalette, hue, bright, NOBLEND);
    if( COOL_LIKE_INCANDESCENT == 1 ) {
      coolLikeIncandescent( c, fastcycle8);
    }
  } else {
    c = CRGB::Black;
  }
  return c;
}


// This function is like 'triwave8', which produces a 
// symmetrical up-and-down triangle sawtooth waveform, except that this
// function produces a triangle wave with a faster attack and a slower decay:
//
//     / \ 
//    /     \ 
//   /         \ 
//  /             \ 
//

uint8_t attackDecayWave8( uint8_t i)
{
  if( i < 86) {
    return i * 3;
  } else {
    i -= 86;
    return 255 - (i + (i/2));
  }
}

// This function takes a pixel, and if its in the 'fading down'
// part of the cycle, it adjusts the color a little bit like the 
// way that incandescent bulbs fade toward 'red' as they dim.
void coolLikeIncandescent( CRGB& c, uint8_t phase)
{
  if( phase < 128) return;

  uint8_t cooling = (phase - 128) >> 4;
  c.g = qsub8( c.g, cooling);
  c.b = qsub8( c.b, cooling * 2);
}

// A mostly red palette with green accents and white trim.
// "CRGB::Gray" is used as white to keep the brightness more uniform.
const TProgmemRGBPalette16 RedGreenWhite_p FL_PROGMEM =
{  CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, 
   CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, 
   CRGB::Red, CRGB::Red, CRGB::Gray, CRGB::Gray, 
   CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Green };

// A mostly (dark) green palette with red berries.
#define Holly_Green 0x00580c
#define Holly_Red   0xB00402
const TProgmemRGBPalette16 Holly_p FL_PROGMEM =
{  Holly_Green, Holly_Green, Holly_Green, Holly_Green, 
   Holly_Green, Holly_Green, Holly_Green, Holly_Green, 
   Holly_Green, Holly_Green, Holly_Green, Holly_Green, 
   Holly_Green, Holly_Green, Holly_Green, Holly_Red 
};

// A red and white striped palette
// "CRGB::Gray" is used as white to keep the brightness more uniform.
const TProgmemRGBPalette16 RedWhite_p FL_PROGMEM =
{  CRGB::Red,  CRGB::Red,  CRGB::Red,  CRGB::Red, 
   CRGB::Gray, CRGB::Gray, CRGB::Gray, CRGB::Gray,
   CRGB::Red,  CRGB::Red,  CRGB::Red,  CRGB::Red, 
   CRGB::Gray, CRGB::Gray, CRGB::Gray, CRGB::Gray };

// A mostly blue palette with white accents.
// "CRGB::Gray" is used as white to keep the brightness more uniform.
const TProgmemRGBPalette16 BlueWhite_p FL_PROGMEM =
{  CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, 
   CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, 
   CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, 
   CRGB::Blue, CRGB::Gray, CRGB::Gray, CRGB::Gray };

// A pure "fairy light" palette with some brightness variations
#define HALFFAIRY ((CRGB::FairyLight & 0xFEFEFE) / 2)
#define QUARTERFAIRY ((CRGB::FairyLight & 0xFCFCFC) / 4)
const TProgmemRGBPalette16 FairyLight_p FL_PROGMEM =
{  CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, 
   HALFFAIRY,        HALFFAIRY,        CRGB::FairyLight, CRGB::FairyLight, 
   QUARTERFAIRY,     QUARTERFAIRY,     CRGB::FairyLight, CRGB::FairyLight, 
   CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight };

// A palette of soft snowflakes with the occasional bright one
const TProgmemRGBPalette16 Snow_p FL_PROGMEM =
{  0x304048, 0x304048, 0x304048, 0x304048,
   0x304048, 0x304048, 0x304048, 0x304048,
   0x304048, 0x304048, 0x304048, 0x304048,
   0x304048, 0x304048, 0x304048, 0xE0F0FF };

// A palette reminiscent of large 'old-school' C9-size tree lights
// in the five classic colors: red, orange, green, blue, and white.
#define C9_Red    0xB80400
#define C9_Orange 0x902C02
#define C9_Green  0x046002
#define C9_Blue   0x070758
#define C9_White  0x606820
const TProgmemRGBPalette16 RetroC9_p FL_PROGMEM =
{  C9_Red,    C9_Orange, C9_Red,    C9_Orange,
   C9_Orange, C9_Red,    C9_Orange, C9_Red,
   C9_Green,  C9_Green,  C9_Green,  C9_Green,
   C9_Blue,   C9_Blue,   C9_Blue,
   C9_White
};

// A cold, icy pale blue palette
#define Ice_Blue1 0x0C1040
#define Ice_Blue2 0x182080
#define Ice_Blue3 0x5080C0
const TProgmemRGBPalette16 Ice_p FL_PROGMEM =
{
  Ice_Blue1, Ice_Blue1, Ice_Blue1, Ice_Blue1,
  Ice_Blue1, Ice_Blue1, Ice_Blue1, Ice_Blue1,
  Ice_Blue1, Ice_Blue1, Ice_Blue1, Ice_Blue1,
  Ice_Blue2, Ice_Blue2, Ice_Blue2, Ice_Blue3
};


// Add or remove palette names from this list to control which color
// palettes are used, and in what order.
const TProgmemRGBPalette16* ActivePaletteList[] = {
  &RetroC9_p,
  &BlueWhite_p,
  &RainbowColors_p,
  &FairyLight_p,
  &RedGreenWhite_p,
  &PartyColors_p,
  &RedWhite_p,
  &Snow_p,
  &Holly_p,
  &Ice_p  
};


// Advance to the next color palette in the list (above).
void chooseNextColorPalette( CRGBPalette16& pal)
{
  const uint8_t numberOfPalettes = sizeof(ActivePaletteList) / sizeof(ActivePaletteList[0]);
  static uint8_t whichPalette = -1; 
  whichPalette = addmod8( whichPalette, 1, numberOfPalettes);

  pal = *(ActivePaletteList[whichPalette]);
}


float dist(float x1, float y1, float x2, float y2) {
  return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}


void drawResonanceFusionEffect(CRGB color1, CRGB color2) {
  float t = millis() * 0.003;
  float pulse = sin(TWO_PI * 0.25 * t) * 1.5 + 6.5;  // 更稳定一点

  for (int x = 0; x < LED_WIDTH; x++) {
    for (int y = 0; y < LED_HEIGHT; y++) {
      float dxL = x - 10;
      float dxR = x - 18;
      float dy  = y - 14;

      float dL = sqrt(dxL * dxL + dy * dy);
      float dR = sqrt(dxR * dxR + dy * dy);

      // 缩小衰减区域 & 提高衰减密度，让形状更集中
      float strengthL = constrain(1.2 - abs(dL - pulse) * 0.4, 0.0, 1.0);
      float strengthR = constrain(1.2 - abs(dR - pulse) * 0.4, 0.0, 1.0);

      CRGB blendL = color1; blendL.nscale8(255 * strengthL);
      CRGB blendR = color2; blendR.nscale8(255 * strengthR);

      // 保留鲜明的色彩
      CRGB fused = blend(blendL, blendR, 128);

      // 增强中心区域亮度
      float centerDist = sqrt((x - 14) * (x - 14) + (y - 14) * (y - 14));
      if (centerDist < 5) {
        uint8_t glowStrength = 80 - centerDist * 10;
        CRGB glow = CHSV(0, 0, glowStrength);
        fused += glow;
      }

      // 提高基础亮度，使得效果更“炸”
      fused.nscale8(230);
      leds[XY(x, y)] += fused;
    }
  }
}









