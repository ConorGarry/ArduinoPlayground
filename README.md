### Instructions

## IDE
You can use whatever you want, I'd recommend downloading the Arduino IDE as it has some good examples with it,
but for actual dev work I find VSCode with PlatformIO add-on better to work with.

- [Official Arduino IDE](https://www.arduino.cc/en/software)
- [Visual Studio Code](https://code.visualstudio.com/)
  - [PlatformIO extension](https://platformio.org/)
  - [Wowki VSCode extension](https://docs.wokwi.com/vscode/getting-started)

[FastLED Library](https://fastled.io/) is the C++ library we're using to control the LEDs, you can install it through the Arduino IDE or PlatformIO.

## Simulation
We can simulate the lighting setup using [wokwi.com](https://www.wokwi.com). It's free to start an account and you can create multiple projects.
Chrome is recommended over other browsers as apparently it yields the best performance. It is quite resource-heavy so I'd recommend closing any applications and tabs that are not being used to free up memory and processing power for the simulator.

### Wokwi VSCode Extension
We can now use the Wokwi VSCode extension to simulate the lights, which is much more convenient than using the website. It requires a license key but it's free, follow the [steps](https://docs.wokwi.com/vscode/getting-started#installation) here to get up and running. The license expires after 30 days but it's free to renew it.

## Getting Started
You don't need a Github but I would recommend it, it's free and it lets you subscribe to changes in the project so you'll get an email whenever there's a change, but if you don't want to start one or use your own, then that's totally fine.

If using Github, clone the project and work away, create branches and pull requests if you want, or just commit directly, we can fix any issues with reverts if needed.

To subscribe to changes, click on the `Watch` button with the eye symbol, and select `All Activity`.  You'll get email notifications when new changes have been made.

If not using Github account, you can download a zip of the codebase and open it in VSCode, and make your changes. When external changes come in, you'll either need to download a new zip, or just manually copy paste from Github in the browser.

If you don't want to bother with any of this, you can directly copy paste code from the Github directly into Wokwi in the browser and make your changes there, then send on the code however you want, e.g. link to you Wokwi project, zip and add to whatsapp, or add a comment in Github and paste it there.

There are two files you need to run the project, currently they are `main_parallel.cpp`, which is the code of the project, and `diagram_parallel.cpp` which is the schema for the simulation (which won't be changed that often).

## Steps
1. Create a [new project in Wokwi](https://wokwi.com/projects/new) using Arduino Mega.
2. Copy all the code from [src/main_parallel.cpp](https://github.com/ConorGarry/ArduinoPlayground/blob/main/src/main_parallel.cpp) and paste it into `sketch.ino` (ino is arduino bespoke format, but it's C++ under the hood).
3. Copy all the code from [diagram_parallel.json](https://github.com/ConorGarry/ArduinoPlayground/blob/main/diagram_parallel.json) into the `diagram.json` tab in Wokwi.
4. Press the play icon in the simulator. You should see something like this:

![Kapture 2024-04-26 at 09 15 08](https://github.com/ConorGarry/ArduinoPlayground/assets/6222596/f462a347-72d3-42c8-bbd7-b6267ed3380b)

_Note: Due to the scale of the project, you'll need to zoom in and out a lot to see a close-up for the schema and to flick the switches._

At this point, you'll see it's running the default mode (0) which is the `rainbowChase()` function.

## Switches (be trippin')
There are 5 on/off state switches for selecing the 32 (5^2) different modes.
The simulator uses slide switches, which have simple persisted on/off states, on = 1, off = 0, the real-life model will use same principle.

<img width="330" alt="Screenshot 2024-04-26 at 17 59 34" src="https://github.com/ConorGarry/ArduinoPlayground/assets/6222596/6ab3674e-ffe4-40d1-9913-c9ce0abdef77">  

_5 Switches in sequence, representing 5 binary digits._


<img width="79" alt="Screenshot 2024-04-26 at 17 59 40" src="https://github.com/ConorGarry/ArduinoPlayground/assets/6222596/9655d0b6-bd6e-4abe-8392-2d4061ac535c">  

_Switch in *off* position, or `0` value_


<img width="69" alt="Screenshot 2024-04-26 at 17 59 49" src="https://github.com/ConorGarry/ArduinoPlayground/assets/6222596/4c7a6031-4fad-43d2-b564-185ec64131cd">  

_Switch in *on* position, or `1` value_


Using this pattern, we can use the switches to represent a 5 digit binary number. So to select between the first four (because at time of writing this there are four patterns), the switches would be set accordingly:

|Binary Value| Mode |
|-|-|
|`00000`|`0`|
|`00001`|`1`|
|`00010`|`2`|
|`00011`|`3`|
|`00100`|`4`|

- The switches are only toggleable when the simulator is running, they won't change when it is paused.
- If the simulator has been stopped, the state will be persisted, i.e. next time you press play, it will resume with the most recent pattern you had selected.
- The switches are quite small, so you'll be zooming in and out a lot, get used to whatever shortcut you use for that, be it keyboard or touchpad.

## Contributing Changes
First of all, have a read of the code, specifically the `switch` statement in the `selectMode()` function. This is what is run on every loop. It reads in the switch states and converts it to a number between `0` and `31` inclusive. A change of a switch will trigger this to select a different number, and subsequently a different function.

```cpp
switch (mode) {
    case 0:
      rainbowChase();
      break;
    case 1:
      multipleTrails();
      break;
    case 2:
      multipleTrails(CRGB::Red);
      break;
    case 3:
      multipleTrails(CRGB::Green);
      break;
    case 4:
      multipleTrails(CRGB::Blue);
      break;
    // Add the rest of the case n..31 as needed.
    default:
      break;
  }
```

To add a new function, you'll need to add a new case to the `switch` statement, and then create the function. Let's say you want to create `rgbChase()` which will chase the RGB colours in sequence.

```cpp
void rgbChase() {
  // Your code here
}
```

```cpp
switch (mode) {
    // ... cases 0-4 removed for brevity.
    case 5:
      rgbChase();
      break;
    default:
      break;
  }
```

Now when you select `000101` (`5`) on the switches, it will run the `rgbChase()` function.

- Functions have to be declared before they are used, so if you're adding a new function, make sure it's above the `selectMode` statement.
- For readability, it's best to keep the functions in the same order as the cases in the `switch` statement.
- Given that there will eventually be 32 functions, I'll probably look to moving them to a different file and using imports, which will clean up this main file. (Though I'm not sure if wokwi supports multiple files yet).

## Implementing a Function/Pattern
_"Pattern" and "function" are interchangeable terms, "Pattern" is what we'd refer to in real-life, "function" is a programming way of defining a body of work, in this case, a "pattern"._

### Code Overview
Before adding your pattern, take a look at what the code is already doing in the setup. Here's an overview:

```c
#define RUNS 30
#define NUM_LEDS 2160
#define NUM_LEDS_PER_STRIP 72
#define NUM_LEDS_PER_SEGMENT 360
```
This simply defines the constants for the number of LEDs in the setup. Pretty self-explanatory. They will come in handy when you're writing your pattern.


```c
// Define the data pins for the LED strips
#define DP_1 7
#define DP_2 8
#define DP_3 9
#define DP_4 10
#define DP_5 11
#define DP_6 12
```
This defines the data pins (hence **DP**) for the LED strips. Due to limited resources on the micro controllers, we can't simply have one long array for every LED, so we have to use **parallel output** technique where we split them in separate segments and control them in parallel. In our case, seeing as we have **30** runs, we have **6** segments of **5** runs each.
The pins are in sequence, so `DP_1` is the first segment, `DP_2` is the second, etc.

Note that this is based on Arduino, which is not the microcontroller we're using, but it will be the same principle, only the number values may differ in the final implementation. It'll still be six data pins in sequence.


```cpp
CRGB leds1[NUM_LEDS_PER_SEGMENT];
CRGB leds2[NUM_LEDS_PER_SEGMENT];
CRGB leds3[NUM_LEDS_PER_SEGMENT];
CRGB leds4[NUM_LEDS_PER_SEGMENT];
CRGB leds5[NUM_LEDS_PER_SEGMENT];
CRGB leds6[NUM_LEDS_PER_SEGMENT];
```
This defines the arrays for the LEDs in each segment. Each segment has 360 LEDs, so we have an array of 360 LEDs for each segment. This is where you'll be writing your patterns to.
CRGB is a struct from the FastLED library, which is used to define the colour of the LEDs. It has three values, `r`, `g`, and `b`, which are the red, green, and blue values respectively. Each value is between 0 and 255, so a colour is defined by three numbers. So `leds1` for example, is an array of 360 LEDs, each with a colour defined by three numbers contained in the CRGB.

```cpp
void setup() {
  // Initialize the LED strip
  FastLED.addLeds<LED_TYPE, DP_1, COLOR_ORDER>(leds1, NUM_LEDS_PER_SEGMENT);
  FastLED.addLeds<LED_TYPE, DP_2, COLOR_ORDER>(leds2, NUM_LEDS_PER_SEGMENT);
  FastLED.addLeds<LED_TYPE, DP_3, COLOR_ORDER>(leds3, NUM_LEDS_PER_SEGMENT);
  FastLED.addLeds<LED_TYPE, DP_4, COLOR_ORDER>(leds4, NUM_LEDS_PER_SEGMENT);
  FastLED.addLeds<LED_TYPE, DP_5, COLOR_ORDER>(leds5, NUM_LEDS_PER_SEGMENT);
  FastLED.addLeds<LED_TYPE, DP_6, COLOR_ORDER>(leds6, NUM_LEDS_PER_SEGMENT);
  FastLED.setMaxRefreshRate(0);
  FastLED.clear();
}
```
The `setup()` function is only run once when the program starts. It initializes the LED strips and sets the refresh rate to 0, which means it will run as fast as possible. It also clears the LEDs, so they are all off when the program starts.

It also adds the leds to the FastLED library, which is what controls the LEDs (saving us **a lot** of complexity). The `addLeds` function takes the type of LED, the data pin, the colour order, the array of LEDs, and the number of LEDs in the array. The `LED_TYPE` and `COLOR_ORDER` are defined elsewhere in the code, but you don't need to worry about them, they're just settings for the FastLED library.

```cpp
void loop() {
  FastLED.show();
  selectPattern();
}
```
The loop function is run continuously, it shows the LEDs (i.e. updates them with the new colours) and then runs the `selectPattern()` function, which is the `switch` statement we talked about earlier.  

---

As a simple demonstration, here's a non-animated pattern that lights up the different segments in red, green, blue, yellow, purple, and white.

```cpp
void rgbypwSegments() {
  // Iterate all segments and light up the LEDs with different colours.
  for (int i = 0; i < NUM_LEDS_PER_SEGMENT; i++) {
    leds1[i] = CRGB::Red;
    leds2[i] = CRGB::Green;
    leds3[i] = CRGB::Blue;
    leds4[i] = CRGB::Yellow;
    leds5[i] = CRGB::Purple;
    leds6[i] = CRGB::White;
  }
  FastLED.show();
}
```
Then placed in the `switch` statement:

```cpp
switch (mode) {
    // ... cases 0-4 removed for brevity.
    case 5:
      rgbypwSegments();
      break;
    default:
      break;
  }
```

Setting the switches to `000101` will now light up the segments in the different colours.
This is static, so it won't animate, but it's a good starting point to get used to the codebase and how to access the different segments.

<img width="837" alt="Screenshot 2024-04-27 at 10 26 40" src="https://github.com/ConorGarry/ArduinoPlayground/assets/6222596/45f50572-b8f7-4c31-b442-de2e5795af01">


## Resources
- [FastLed documentation](http://fastled.io/docs/)
- [FastLED basics YouTube Tutorial Series](https://www.youtube.com/watch?v=4Ut4UK7612M&list=PLgXkGn3BBAGi5dTOCuEwrLuFtfz0kGFTC) <- I **highly** recommend this! He's a great tutor, explains everything very well.
