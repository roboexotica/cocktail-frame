# cocktail-frame
This project is an attempt to revive [Roboexotica](http://roboexotica.at/)'s cocktail frame for its 25<sup>th</sup> anniversary in Q4 2023.
Originally, it was built some 10+ years ago by Ryan Finnigan in a self-inflicted multi-day hackathon at [Metalab](https://metalab.at/) for that year's Roboexotica. The cocktail frame was last exhibited in 2015.

## Hardware
The cocktail frame consists of a welded steel frame, big enough to enclose a DIN A1 poster, and hosts all the electronics necessary to pour out a ~2 cl shot after pressing a button. As the original dispensing system is no longer available, the team behind the revival decided to use an electric pump to deliver the liquid, together with an electro-magnetic valve to stop undesired leakage. The frame's lightshow capabilities will be reused and possibly enhanced.

### Components:
* a steel frame, to house everything
* an [Arduino Uno](https://store.arduino.cc/products/arduino-uno-rev3)
* a [SparkFun EL Escudo Dos](https://www.sparkfun.com/products/retired/10878)
* several EL wires (electro-luminescence wires)
* a 12V DC to high voltage AC inverter for the EL wires
* a 12V electric pump
* a 12V solenoid valve
* a 2x relay module
* an LED button
* a power supply 100-240 VAC to 12V DC
* wires and silicone tubes 

## Installation
This project can either be built with the [~~Arduino IDE*~~](https://www.arduino.cc/en/software) or with [PlatformIO](https://docs.platformio.org/en/latest/core/installation/).
Follow all the required steps on either website to complete the IDE installation.

### Arduino
*Arduino doesn't work atm, sry

Check out or download the repository. Arduino should know what to do with the _.ino_ file.
It needs the library `marcoschwartz/LiquidCrystal_I2C@^1.1.4` to run. There should be an Arduino version in the Arduino IDE's library manager. 

### PlatformIO
Personally, I'm using [CLion](https://www.jetbrains.com/clion/) as IDE with the [PlatformIO for CLion](https://plugins.jetbrains.com/plugin/13922-platformio-for-clion) plugin. After executing _Tools/PlatformIO/Re-Init_, all the necessary library and build files will be downloaded or created. 

However, there's a small problem with selecting the necessary build configuration in CLion because this setting is stored in the user's _workspace.xml_ file, which is autogenerated upon project creation and is usually not published because it holds information about which files are opened by users. Unfortunately, the setting can't be changed in the IDE but rather in the file itself. 

To fix it, open _workspace.xml_ and change `PROFILE_NAME` and `CONFIG_NAME` in `CMakeSettings` to your environment defined in _platform.ini_, in our case `"uno"`. Reloading the project may be necessary for the changes to take effect.

```xml
<component name="CMakeSettings">
  <configurations>
    <configuration PROFILE_NAME="uno" ENABLED="true" CONFIG_NAME="uno" />
  </configurations>
</component>
```

## Build & Upload
### Arduino
Select _"Arduino Uno"_ as the board, click _"Verify"_ to compile the code, and click _"Upload"_ to upload it to your device.

### CLion & Platform.IO
The project can be built via the _"hammer"_ icon or with _"Tools/PlatformIO/Build"_ or _"Tools/PlatformIO/Build Production"_.

To upload the code to your device, select _"PlatformIO Upload | uno"_ in the configuration dropdown in Clion and click the _"Run"_ icon right of it.

## Todos
* Add some images
* Mount hardware neatly
* Add an extravagant lightshow
* Decide if we want to add a coin acceptor, Wi-Fi, GPS, AI, Diesel engine or a second cocktail choice.

## Contributors
* https://github.com/amalettomat
* https://github.com/reinbert

P.S.: There's also the _Roboexotica Stammtisch_ each second Monday after a full moon at [Metalab](https://metalab.at/). Please email [roboexotica@reinbert.net](mailto:roboexotica@reinbert.net) for the exact date.