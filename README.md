#  InfiniTime-borkymods

Howdy! This repo is a fork of the InfiniTime firmware for PineTime incorporating a suite of modifications by me (borky)

## Added features include:

- Swipe up on metronome app replaces dial with counter, for more precision [tempo-counter](https://github.com/borkymcgee/InfiniTime-borkymods/tree/tempo-counter)
- CasioStyleG7710 shows number of undismissed notifications [casio-notify-number](https://github.com/borkymcgee/InfiniTime-borkymods/tree/casio-notify-number)
  - Previously only showed an 'i' if there were unread notifications, now shows the number of undismissed notifications
- weather and temperature added to CasioStyleG7710 [casio-weather](https://github.com/borkymcgee/InfiniTime-borkymods/tree/casio-weather)
  - also added PineTimeStyle-style settings menu to enable/disable them
  - choose temperature units [temp-units](https://github.com/borkymcgee/InfiniTime-borkymods/tree/temp-units)
    - (this one is a branch off of casio-weather, so also includes those features)
    - unified setting for preferred unit
    - PTS and CSG display the correct temperature in that unit
    - ability to toggle that unit in the settings of PTS and CSG
- artist/track/album of current media added to CasioStyleG7710 [casio-media](https://github.com/borkymcgee/InfiniTime-borkymods/tree/casio-media)
  - also added PineTimeStyle-style settings menu to select which of those to show, if any
- touchscreen lock [touch-lock](https://github.com/borkymcgee/InfiniTime-borkymods/tree/touch-lock)
  - double pressing the button disables or re-enables the touch screen, anywhere in InfiniTime
  - an indicator of the current state of the lock on the casio and PineTimeStyle watchfaces
- altered music app to be entirely swipe-based [swipe-only-music](https://github.com/borkymcgee/InfiniTime-borkymods/tree/swipe-only-music)
  - (up/down for volume, left/right for forward-back, tap for pause)
  - removed graphical buttons
- swiping left on the watchface opens the music app [swipe-left-music-shortcut](https://github.com/borkymcgee/InfiniTime-borkymods/tree/swipe-left-music-shortcut)
- swiping up opens the timer app first, and then the app list [swipe-up-timer-shortcut](https://github.com/borkymcgee/InfiniTime-borkymods/tree/swipe-up-timer-shortcut)
- ability to swipe left or right on the timer app while setting to increase/decrease the timer by 5 minutes [timer-swipe-jump5](https://github.com/borkymcgee/InfiniTime-borkymods/tree/timer-swipe-jump5)
- ability to swipe left or right on notifications to dismiss them [ambi-notifications](https://github.com/borkymcgee/InfiniTime-borkymods/tree/ambi-notifications)
- custom version text :3 [custom-version-text](https://github.com/borkymcgee/InfiniTime-borkymods/tree/custom-version-text)

## Structure of this fork:
- main branch: all of the mods merged together
- clean branch: a pristine clone of the original repo to branch new features off of
- all other branches: identical to the clean branch but with one mod in each branch

- casio-plus: deprecated branch that predates, and combines, several of the other mods. Will probably be deleted in the near future


# [InfiniTime](https://github.com/InfiniTimeOrg/InfiniTime)

![InfiniTime logo](doc/logo/infinitime-logo-small.jpg "InfiniTime Logo")

Fast open-source firmware for the [PineTime smartwatch](https://www.pine64.org/pinetime/) with many features, written in modern C++.

## New to InfiniTime?

- [Getting started with InfiniTime](doc/gettingStarted/gettingStarted-1.0.md)
- [Updating the software](doc/gettingStarted/updating-software.md)
- [About the firmware and bootloader](doc/gettingStarted/about-software.md)
- [PineTimeStyle Watch face](https://wiki.pine64.org/wiki/PineTimeStyle)
  - [Weather integration](https://wiki.pine64.org/wiki/Infinitime-Weather)

### Companion apps

- [Gadgetbridge](https://gadgetbridge.org/) (Android)
- [AmazFish](https://openrepos.net/content/piggz/amazfish/) (SailfishOS)
- [Siglo](https://github.com/alexr4535/siglo) (Linux)
- [InfiniLink](https://github.com/InfiniTimeOrg/InfiniLink) (iOS) **[Looking for a new maintainer]**
- [ITD](https://gitea.elara.ws/Elara6331/itd) (Linux)
- [WatchMate](https://github.com/azymohliad/watchmate) (Linux)

***Note** : We removed mentions to NRFConnect as this app is closed source and recent versions do not work anymore with InfiniTime (the last version known to work is 4.24.3). If you used NRFConnect in the past, we recommend you switch to [Gadgetbridge](https://gadgetbridge.org/).*

## Development

- [InfiniTime Vision](doc/InfiniTimeVision.md)
- [Rough structure of the code](doc/code/Intro.md)
- [How to implement an application](doc/code/Apps.md)
- [Generate the fonts and symbols](src/displayapp/fonts/README.md)
- [Tips on designing an app UI](doc/ui_guidelines.md)
- [Bootloader, OTA and DFU](bootloader/README.md)
- [External resources](doc/ExternalResources.md)

### Contributing

- [How to contribute?](CONTRIBUTING.md)
- [Coding conventions](doc/coding-convention.md)

### Build, flash and debug

- [InfiniTime simulator](https://github.com/InfiniTimeOrg/InfiniSim)
- [Build the project](doc/buildAndProgram.md)
- [Build the project with Docker](doc/buildWithDocker.md)
- [Build the project with VSCode](doc/buildWithVScode.md)
- [Flash the firmware using OpenOCD and STLinkV2](doc/openOCD.md)
- [Flash the firmware using SWD interface](doc/SWD.md)
- [Flash the firmware using JLink](doc/jlink.md)
- [Flash the firmware using GDB](doc/gdb.md)
- [Stub using NRF52-DK](doc/PinetimeStubWithNrf52DK.md)

### API

- [BLE implementation and API](doc/ble.md)

### Architecture and technical topics

- [Memory analysis](doc/MemoryAnalysis.md)

### Project management

- [Maintainer's guide](doc/maintainer-guide.md)
- [Versioning](doc/versioning.md)
- [Project branches](doc/branches.md)
- [Files included in the release notes](doc/filesInReleaseNotes.md)
- [Files needed by the factory](doc/files-needed-by-factory.md)

## Licenses

This project is released under the GNU General Public License version 3 or, at your option, any later version.

It integrates the following projects:

- RTOS : **[FreeRTOS](https://freertos.org)** under the MIT license
- UI : **[LittleVGL/LVGL](https://lvgl.io/)** under the MIT license
- BLE stack : **[NimBLE](https://github.com/apache/mynewt-nimble)** under the Apache 2.0 license
- Font : **[Jetbrains Mono](https://www.jetbrains.com/fr-fr/lp/mono/)** under the Apache 2.0 license

## Credits

I’m not working alone on this project. First, many people create PR for this project. Then, there is the whole #pinetime community : a lot of people all around the world who are hacking, searching, experimenting and programming the Pinetime. We exchange our ideas, experiments and code in the chat rooms and forums.

Here are some people I would like to highlight:

- [Atc1441](https://github.com/atc1441/) : He works on an Arduino based firmware for the Pinetime and many other smartwatches based on similar hardware. He was of great help when I was implementing support for the BMA421 motion sensor and I²C driver.
- [Koen](https://github.com/bosmoment) : He’s working on a firmware based on RiotOS. He integrated similar libs as me : NimBLE, LittleVGL,… His help was invaluable too!
- [Lup Yuen Lee](https://github.com/lupyuen) : He is everywhere: he works on a Rust firmware, builds a MCUBoot based bootloader for the Pinetime, designs a Flutter based companion app for smartphones and writes a lot of articles about the Pinetime!
