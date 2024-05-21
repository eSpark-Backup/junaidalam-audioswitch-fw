# Audio Switch Application
#### Client Name: Brad Fairman

The Audio switch board focuses on allowing a single S/PDIF input to be routed to 4 different S/PDIF outputs via switching and volume control. The incoming stream is optical TOSLINK uncompressed PCM.

## Firmware Requirements
Electronics boots up and waits for user input all twelve LED lights are on (pushbutton LEDS).  The user needs to press one of the fours demo pushbuttons to start the audio stream from the TV,  Once they push (Demo 2) button the audio stream will play to output 2 on the electronics.  The volume buttons for output 2 are now active and can be used.  The stream will play for 30 seconds and then timeout.  If Demo 2 is pressed again during the timer period the stream is stopped, meaning demo 2 pushbutton starts the stream or pauses the stream. If any other demo button is pressed (buttons 1,3,4) then the corresponding output is enabled and the stream plays. Below are the button sequences:

- Demo 1
     - START / STOP : J1
     - Volume Up    : J2
     - Volume Down  : J3
- Demo 2
     - START / STOP : J4
     - Volume Up    : J5
     - Volume Down  : J6
- Demo 3
     - START / STOP : J7
     - Volume Up    : J8
     - Volume Down  : J9
- Demo 4
     - START / STOP : J10
     - Volume Up    : J11
     - Volume Down  : J12
