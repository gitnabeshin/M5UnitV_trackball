
#include <M5Stack.h>

#define MOTOR_A1        17
#define MOTOR_A2        16
#define MOTOR_B1        5
#define MOTOR_B2        26
//21, 22 for UART
//25 speaker

HardwareSerial VSerial(1);

typedef struct {
    int16_t dx;         //object position x
    uint32_t area;      //object detected area
} v_response_t;

void myPrintln( String str ) {
    // Lcdに書くとシリアル通信が遅くなる
    //   M5.Lcd.setCursor(0, 30);
    //   M5.Lcd.println(str);
    Serial.printf("[%s]\n", str);
}

void handleForward() {
    myPrintln(" FORWARD");
    digitalWrite(MOTOR_A1, LOW);   //LEFT
    digitalWrite(MOTOR_A2, HIGH);
    digitalWrite(MOTOR_B1, LOW);   //RIGHT
    digitalWrite(MOTOR_B2, HIGH);
    delay(20);
    digitalWrite(MOTOR_A1, LOW);  //LEFT
    digitalWrite(MOTOR_A2, LOW);
    digitalWrite(MOTOR_B1, LOW);  //RIGHT
    digitalWrite(MOTOR_B2, LOW);
}

void handleStop() {
    myPrintln(" STOP");
    digitalWrite(MOTOR_A1, LOW);  //LEFT
    digitalWrite(MOTOR_A2, LOW);
    digitalWrite(MOTOR_B1, LOW);  //RIGHT
    digitalWrite(MOTOR_B2, LOW);
    delay(20);
}

void handleLeft() {
    myPrintln(" LEFT");
    digitalWrite(MOTOR_A1, HIGH); //RIGHT
    digitalWrite(MOTOR_A2, LOW);
    digitalWrite(MOTOR_B1, LOW);  //LEFT
    digitalWrite(MOTOR_B2, HIGH);
    delay(30);
    digitalWrite(MOTOR_A1, LOW);  //RIGHT
    digitalWrite(MOTOR_A2, LOW);
    digitalWrite(MOTOR_B1, LOW);  //LEFT
    digitalWrite(MOTOR_B2, LOW);
    delay(10);
}

void handleRight() {
    myPrintln(" RIGHT");
    digitalWrite(MOTOR_A1, LOW);   //RIGHT
    digitalWrite(MOTOR_A2, HIGH);
    digitalWrite(MOTOR_B1, HIGH);  //LEFT
    digitalWrite(MOTOR_B2, LOW);
    delay(30);
    digitalWrite(MOTOR_A1, LOW);  //RIGHT
    digitalWrite(MOTOR_A2, LOW);
    digitalWrite(MOTOR_B1, LOW);  //LEFT
    digitalWrite(MOTOR_B2, LOW);
    delay(10);
}

void handleBackward() {
    myPrintln(" BACK");
    digitalWrite(MOTOR_A1, HIGH); //RIGHT
    digitalWrite(MOTOR_A2, LOW);
    digitalWrite(MOTOR_B1, HIGH); //LEFT
    digitalWrite(MOTOR_B2, LOW);
    delay(20);
    digitalWrite(MOTOR_A1, LOW);  //RIGHT
    digitalWrite(MOTOR_A2, LOW);
    digitalWrite(MOTOR_B1, LOW);  //LEFT
    digitalWrite(MOTOR_B2, LOW);
}

void setup() {
    M5.begin();

    VSerial.begin(115200, SERIAL_8N1, 21, 22);
    // Serial2.begin(115200, SERIAL_8N1, 21, 22);

    dacWrite(25, 0); // Speaker OFF

    pinMode(MOTOR_A1, OUTPUT);
    pinMode(MOTOR_A2, OUTPUT);
    pinMode(MOTOR_B1, OUTPUT);
    pinMode(MOTOR_B2, OUTPUT);

    Serial.printf("Setup OK. \n");
}

enum {
    kNoTarget = 0,
    kLeft,          //1
    kRight,         //2
    kStraight,      //3
    kTooClose       //4
};

const uint16_t kThreshold = 30; // If target is in range ±kThreshold, the car will go straight
v_response_t v_data;    // Data read back from V
uint8_t state = 0;  // Car's movement status

void loop() {

    VSerial.write(0xAF);

    if(VSerial.available()) {
        uint8_t buffer[5];
        VSerial.readBytes(buffer, 5);
        // 先頭2byteがdx
        v_data.dx = (buffer[0] << 8) | buffer[1];
        // 残り3byteがarea
        v_data.area = (buffer[2] << 16) | (buffer[3] << 8) | buffer[4];

        //QVGA:320*240の幅dx
        if(v_data.dx > -160 && v_data.dx < 160) {
            if(v_data.area > 30000)
            {
                state = kTooClose;  // Back
            }
            else if(v_data.dx > -kThreshold && v_data.dx < kThreshold)
            {
                state = kStraight;  // Go straight
            }
            else if(v_data.dx <= -kThreshold)
            {
                // UnitVの上下がサンプルとは違って逆になる @sensor.set_vflip(1)
                state = kRight; // Go right
            }
            else if(v_data.dx >= kThreshold)
            {
                // UnitVの上下がサンプルとは違って逆になる 2sensor.set_vflip(1)
                state = kLeft;  // Go left
            }
            else
            {
                state = kNoTarget;  // Rotate to look for
            }
            M5.Lcd.fillScreen(GREEN);
        } else {
            state = kNoTarget;  // Rotate to look for
            M5.Lcd.fillScreen(RED);
        }

        Serial.printf("%d, %d, %d \n", v_data.dx, v_data.area, state);
    }

    //The speed and time here may need to be modified according to the actual situation
    switch(state) {
        case kNoTarget:
            handleRight();
            break;
        case kLeft:
            handleLeft();
            break;
        case kRight:
            handleRight();
            break;
        case kStraight:
            handleForward();
            break;
        case kTooClose:
            handleBackward();
            break;
        default:
            handleStop();
            break;
    }

    // これを入れないとシリアル読み込み出来ない。
    delay(10);
}
