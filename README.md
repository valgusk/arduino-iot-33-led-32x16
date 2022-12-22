# `Arduino Nano IOT 33` project for `DFR0471` RGB LED Matrix Panel

## What is does
- renders 12bpp color data to mentioned led matrix.
- receives said data from TCP socket(port 80)

# 12bpp color data
- 4 bits per channel means that e.g. max RED value is 15
- It takes array of 768 bytes(start of `buffer`). Each byte contains two color channels
- color channel sequence:
    ```
    |                  1 byte                      |                   1 byte                    |
    |      4bit           |         4bit           |        4bit          |
    row0_pix0_red(4bit),   row0_pix0_green(4bit),   row0_pix0_blue(4bit),
    row0_pix1_red(4bit),   row0_pix1_green(4bit),   row0_pix1_blue(4bit),
    ...,
    row15_pix31_red(4bit), row15_pix31_green(4bit), row15_pix31_blue(4bit)
    ```
# how data transfer is done
- client connects with TCP stream socket
- for each frame
  - client writes 768 bytes to socket (`16`rows * `32`colums * (4`red` + 4`green` + 4`blue`)) / 8`bits per byte`
  - client waits for 1 byte from the socket which the microcontroller sends when data is received (to avoid buffer overflows, as client is probably faster)
- client closes the TCP socket

# how render in the panel is done
- for each row pair (0+8, 1+9, ..., 7+15)
  - we set current row(`setLine(line_pair - 1)`) - 1 is substracted to account for technical order of panel rows.
  - we disable panel (`outputOff()`)
  - for each column
    - we set rgb value for a two pixels, one for each row in the pair (`setVal(...)`) and shift to next pixel (`clock()`).
      _here the rgb value is actually 1bit, that is - ON/OFF. But we do it frequently enough to make it seem like it is PWM-ed.
      for example if stored value is 13, it will get turned on 13 of 15 frames, thus getting more or less expected percieved brightness (this is the reason for nano not being really performant enough)_
  - we toggle LAT(to send data for row I think) (`latch()`)
  - we turn the panel back on (`outputOn()`)

# Notes
- `WiFi.begin("xxxxxxxx", "xxxxxxxx")` must be changed to whatever your `SSID` | `password` are
- assigning static IP to controller in router is recommended. No discovery functionality is built in for pefromance reasons.
- almost all my knowledge on how the panel operates is experimental. Something might not be correct
- only one TCP client is allowed at the same time
- same pins as in project must be wired, because direct write through registers was necessary for performance
- socket on connecting client has to be closed after use, otherwise nano will never know that connection is lost. If it was not closed, microcontroller restart is needed to make it receive data again.
- latest WifiNINA firmware is highly recommended
- average-good wifi coverage is highly recommended, otherwise animations will flicker visibly
- Windows seems to compile project to a less performant binary for me. Might be circumstance, but trying Linux is viable if render seems flickery
- 16x32 panel is hardcoded. Should be easy to change for bigger panels probably, but I don't think the performance of nano will be enough for it.
