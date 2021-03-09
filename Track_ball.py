# -----------------------------------
#  閾値テスト
# -----------------------------------

import sensor
import image
import time
import utime
from machine import UART
from Maix import GPIO
from fpioa_manager import *
from modules import ws2812

# RGB LED設定
class_ws2812 = ws2812(8,100)
BRIGHTNESS = 0x10

#fm.register(34,fm.fpioa.UART1_TX)
#fm.register(35,fm.fpioa.UART1_RX)
fm.register(35, fm.fpioa.UART1_TX, force=True)
fm.register(34, fm.fpioa.UART1_RX, force=True)
#uart_out = UART(UART.UART1, 115200, 8, None, 1, timeout=1000, read_buf_len=4096)
uart_out = UART(UART.UART1, 115200, 8, 0, 0, timeout=1000, read_buf_len=4096)


sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.set_vflip(1)
sensor.run(1)
sensor.skip_frames()

while False:
    uart_out.write('TEST\n')
    utime.sleep_ms(100)

# RGB LEDを点滅
def BLINK_RGB_LED(r,g,b):
    a = class_ws2812.set_led(0,(r,g,b))
    a = class_ws2812.display()
    time.sleep(0.3)
    a = class_ws2812.set_led(0,(0,0,0))
    a = class_ws2812.display()
    time.sleep(0.3)
    a = class_ws2812.set_led(0,(r,g,b))
    a = class_ws2812.display()

def RGB_LED_ON(r,g,b):
    a = class_ws2812.set_led(0,(r,g,b))
    a = class_ws2812.display()

def RGB_LED_OFF():
    a = class_ws2812.set_led(0,(0,0,0))
    a = class_ws2812.display()

# 起動インジケータとしてgreenで点滅、最後に点灯
BLINK_RGB_LED(0,BRIGHTNESS,0)
BLINK_RGB_LED(0,BRIGHTNESS,0)
BLINK_RGB_LED(0,BRIGHTNESS,0)

# orange
target_lab_threshold = (0, 100, 4, 127, 15, 127)

# red
#target_lab_threshold = (0, 100, 31, 127, 19, 127)

# blue
#target_lab_threshold = (0, 100, 35, 127, -128, 127)

while True:
    img=sensor.snapshot()

    blobs = img.find_blobs([target_lab_threshold], x_stride = 2, y_stride = 2, pixels_threshold = 100, merge = True, margin = 20)
    if blobs:
        max_area = 0
        target = blobs[0]
        dx = 0
        for b in blobs:
            if b.area() > max_area:
                max_area = b.area()
                target = b
        # シリアル通信相手がいる場合に検出領域情報を送る
        if uart_out.read(4096):
            # bounding box area
            area = target.area()
            # dx[-120 to 120, 0 is muddle.] x:target[5], y:target[6]  QVGA:320*240
            dx = 160 - target[5]
            hexlist = [(dx >> 8) & 0xFF, dx & 0xFF, (area >> 16) & 0xFF, (area >> 8) & 0xFF, area & 0xFF]
            uart_out.write(bytes(hexlist))
        else:
            pass

        print( 160 - target[5] , target.area())
        tmp=img.draw_rectangle(target[0:4])
        tmp=img.draw_cross(target[5], target[6])
        c=img.get_pixel(target[5], target[6])
        # rgb orange
        RGB_LED_ON(255, 165, 0)

    else:
        # 検出なし rgb led off
        RGB_LED_OFF()
        if uart_out.read(4096):
            hexlist = [0x80, 0x00, 0x00, 0x00, 0x00]
            uart_out.write(bytes(hexlist))
        else:
            pass
