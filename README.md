# Tapper clone for Playstation 1

As a toy project to learn the Playstation 1 SDK, I've developed a clone of the [Tapper](https://en.wikipedia.org/wiki/Tapper) arcade. The clone is far from being complete and I'm not interested in completing the project (such as implementing all the levels!). I am just interested in working out all the relevant parts from the SDK (audio, video, memory card management, ...).

## TL;DR;

1. Install the [PysQ SDK](http://www.psxdev.net/downloads.html)
2. Run PSPATH [RegEdit script](https://github.com/apiraino/psx_tapper/blob/master/tools/pspath_winxp.reg)
3. `git clone https://github.com/apiraino/psx_tapper tapper`
4. `cd tapper`
5. (customize Makefile with correct paths) and `make`

## Requirements

* Windows 95/98/XP (not tested on 7/8/10)
* Sony PsyQ SDK (better 4.6)
* GNU Make 3.81

## A bit of history

### What is a Playstation 1, PS1, PSX or PSone?
Obligatory link to [Wikipedia](https://en.wikipedia.org/wiki/PlayStation_(console)). All those are commercial and _street_ names for the first generation of Sony console. Technically speaking _PSX_ doesn't belong to the pack since it was a Japan-only release of a hybrid hardware (a [Playstation2 console + a DVR](https://en.wikipedia.org/wiki/PSX_(video_game_console))).

I suggest using any classic "brick" model (from `SCPH-1001` to `SCPH-900x` models, see [models](https://en.wikipedia.org/wiki/PlayStation_models)). The console must be modded in order to run unsigned software.

[PSone](https://en.wikipedia.org/wiki/PlayStation_(console)#PSone), on the other hand, is the second, cost reduced, release of the console. The lack of a serial port makes difficult to connect the console to your PC; keep in mind this if you want to develop software for the PS1. PSone units can theoretically used too, but you need a heavily modded unit (the serial port is still soldered on the mainboard but there's no external connector).

### What is Tapper?
It's a 1983 Midway arcade about a barman trying to serve booze to as many customers as possible. Game mechanics are simple, basically it's a static single screen with a few moving sprites and a collision detection logic.

This reimplementation is completely from scratch concerning code, while original graphical assets have been ripped off thanks to M.A.M.E. (`F4` key in game).

A sprite sheet is an image file where any in-game image asset is stored at precise coordinates. You can find the first two original sprite sheets [here](https://github.com/apiraino/psx_tapper/blob/master/pics/screengrabs) (files `sheet0.bmp` and `sprite1.bmp`).

See [Workflow for graphics](#workflow-for-graphics) for some tips about graphics.

There's no sound or music in this clone at this time (see [TODO](https://github.com/apiraino/psx_tapper/blob/master/TODO.txt)), I've haven't tackled yet the API.

### Documentation
At that time there was no StackOverflow :-) and developers relied on heavy tomes sent by Sony along the hardware devkit. The two books that you will need are:
* LIBREF46.PDF (Run-time library Reference)
* LIBOVR46.PDF (Run-Time Library Overview)

Scroll down to [References](#references) to find a link to download them. Of course there's quite some code snippets from demos and leaked game sources to learn by trial and errors (apply your google-fu).

### Programming the PS1: a brief hardware overview
Specs of the console are everywhere, I'll just point out here some information you'll learn by programming the machine.

* The memory layout is detailed in many tutorials, for example from [Orion_'s website](http://onorisoft.free.fr/psx/tutorial/tuto.htm).

* You have at your disposal a single memory page of 1024x512 pixels for 16bit for a total of 1mb of VRAM, but you can't use all this "wealth".

  Speaking of PAL resolutions, the first 320x256 pixels are used by the currently active frame buffer (what you see on the TV). At \[0,256\] ("under" the previous buffer) you have another 320x256 back buffer that is being drawn while the other one is shown. These two buffers swap at every frame.
  
  This leaves you with half of that megabyte of VRAM to store your graphic assets. You may increase your game resolution up to 640x512 but the more VRAM you steal for your frame buffers, the less remains for quick access of image assets. Assets that are not in VRAM must be loaded from CD and that kills performances. Conventional RAM (2mb) is used for code, sound and libraries.

* The way graphics work in the PS1 is roughly like a treadmill: you have a structure called _Ordering Tables_ (basically a linked list of graphical instructions) that feeds the GPU. After VSYNC you init an Ordering Table item, execute instructions (move sprites, manage collisions and so on) and finally send the new OT to the GPU. Buffers are then swapped:
```
    /* code using PSXLIB for clarity */
    while (1)
    {
        /* init a new OT */
        ot = System_InitFrame();

        /* your code that moves sprites etc. */

        /* send OT to the GPU */
        System_DrawFrame(ot, NULL, NULL);
    }
```
Be careful doing too much in the time frame at your disposal. Refresh frequencies (PAL or NTSC) allows you 30ms/40ms between frames. If you crunch too much code or your code is not properly optimized, the OT won't be sent in time to keep the refresh rate above 24fps and your game will be sluggish. Not a problem for such a trivial game like this, but sound familiar, huh?

You may have heard of the [Net Yaroze](https://en.wikipedia.org/wiki/Net_Yaroze) console: it was a special PS1 targeted at bedroom/indie coders willing to approach the Sony console for little cost. It has the same specs as the grey PS1 and some peculiar features:
 * It's a collector item, therefore extremely pricey; on the other hand you can grab a modded grey PS1 for a few bucks off eBay
 * It's region free
 * Much of the RAM memory of the Net Yaroze is occupied by debug libraries, actually reducing the available memory for your application.
 * The Net Yaroze looks cool but from a programmer's standpoint is worse. The only tangible advantage over a grey unit is the serial cable that easily connects the console to a PC for deploying your code. This link functionality, however, can be obtained with little effort buying a [PSXSERIAL](https://psx0.wordpress.com/2013/08/21/psx-serial-cable-now-on-sale/) cable or with a Xplorer (+ Caetla firmware) cartridges (a bit more complicated since these debug devices are less common).

Here's some more [FAQ](http://www.psxdev.net/help/common_questions.html).

## Setup your environment, install the SDK
Development for the PS1 flourished in the mid Nineties, the supported platform of choice started with DOS + Win3.1 and ended with Windows95/98.

Today there are a couple of ways to develop software for the PS1; my preferred is the original one, using the [Psy-Q SDK](http://www.psxdev.net/downloads.html) and a text editor.

Psy-Q was developed by Psygnosis and ported to a bunch of the most important console platforms at that time (Sega MegaDrive, 32X, Saturn, and so on).

Today you can run Psy-Q in XP/7 (also virtualized). You need to extract the zip downloaded (preferably in `c:\psyq`) and run the RegEdit script you find in the [tools](https://github.com/apiraino/psx_tapper/blob/master/tools) directory to set paths and env variables (choose the XP or the 7 flavour, there's just a different header). I didn't test with Win8/10.

Theoretically you should be ready to go. Run `make` and `ccpsx`, see if their output is ok.

## Get to know your tools

### Workflow for coding and debugging
Back in the day modern IDE were not so common, there were basic instruments for writing code and debugging.

Choose your preferred code editor, that will do.

Debugging is a bit more complicated. At the time there were ISA add-on cards that basically were a PS1 console into your PC. After uploading your game into the "card", you could debug through a serial port. These cards are nowadays collectors' items therefore pricey. Look for a Sony `DTL-H2000`, `DTL-H2500` for example. `DTL-Hxxxx` identifies development and debugging hardware, `SCPH-xxxx` identifies consumer products.

I've ended up debugging with simple `printf()` statements from the emulator and overlay text to show CPU usage, objects count, stuff like that. I don't know how far you can go this way with complex projects though.

Tools used by the programmer:

* **Psymake 1.14**: this is a custom `make` done by SN Systems and distributed with the SDK. Personally I don't like it, it shows its age. The Makefile syntax is also slightly different from what the standard is (GNU make).
* **GNU Make 3.81**: use this if you can. Convert Makefiles from projects using Psymake.
* **ccpsx**: the C compiler for R3000A CPU
* **bin2h**:  tool to convert an image data into a C header. This is a trick often used back in the day (also in Amigaland) to embed assets into the code.
* **WinBin2Src**: same thing as above, but it outputs also in R3000A asm
* **pxfinder.obj**: (optional) a little tool to help you find the coordinates on screen when you need to place sprites. Link the object to your stuff at compile time and you'll have a crosshair showing the current [x,y] coordinates (see [Workflow for graphics](#workflow-for-graphics)).
* **MMGMNEW.OBJ**: this is the memory library with updated version of memory management function (malloc, free, etc.). **Use them instead of the stock functions** (see [Fun facts](#fun-facts)).

--> __Remember to write C89 code__.

### Workflow for graphics
Playstation 1 image data is compressed in a proprietary format called TIM. Do some googling to [learn what is it](https://web.archive.org/web/20171214120206/https://wiki.qhimm.com/view/PSX/TIM_format). TIM files have basically three components: image data, palette data and [x,y] coordinates to place the TIM file into the VRAM when loaded.

The way to work with images is:
- stick to BMP files and create your sprite sheets. Avoid compressed formats like JPG.
- when your sprite is finished convert the BMP to TIM. TIM files are great at reducing space.
- use TIMTOOL to place TIM files into the frame buffer. Position in memory will be saved into the TIM file itself.
- sprites need to be manually placed on the canvas, at least their starting position. Finding the starting coordinates of a sprite is a tedious job because:
 - you stick in your code the sprite starting position somewhere
   - compile and run
 - not satisfied, change the position
   - compile and run
 - rinse and repeat until you're satisfied

 In order to ease this pain I wrote **pxfinder**, a tiny library that shows the current cursor position. You link it in your code and it should shrink the time wasted for sprite positioning.

### Use efficiently a sprite map
- Have a look at the original [sprite map](https://github.com/apiraino/psx_tapper/blob/master/pics/screengrabs/sheet0.bmp): sprites are divided in two (e.g. top and bottom half of the bartender); often you only need to load the part that is actually changing. Try to isolate the "moving" parts of your sprites and only change that in your code when you react to events.
- Positioning sprites in the sheet at "smart" [x,y] coordinates allows you to retrieve them with less code (see [the drunkard frame struct](https://github.com/apiraino/psx_tapper/blob/master/drunkard.c)).

Here's some tools (see [References](#references) or google them):

* **TIM2VIEW**: a viewer for TIM files
* **TIMUTIL**: a converter to and from TIM <--> BMP, plain RGB, PICT format
* **TIMTOOL**: a very important tool! This lets you place TIM files in a map representing the PS1 memory. This is very useful to understand how VRAM is used. Position in memory is then saved into the TIM file.
* **bmp2tim**: converts back BMP -> TIM
* **GIMP / Photoshop 4.0 / PaintShopPro 4**: if you want to do pixel art there are probably better tools, but I used GIMP to prepare the sprite sheets. Also Photoshop 4 can be used (which is free as in free beer).
* create_tapper_tim.py or .sh, make_cursor.sh: just an example of scripts you can code yourself to create TIM files.

### Workflow for 3D graphics
TODO

### Workflow for audio
TODO

### Workflow for the release
In the [makecd](https://github.com/apiraino/psx_tapper/blob/master/tools/makecd) directory you'll find some tools and create a file your PS1 will be able to load.

You don't need to prepare a CD (i.e. an ISO9660 file): often a simple PS-X executable will suffice, especially if you create small applications or POC for learning.

In order to create an executable use **cpe2psx**. From the Readme: _"What it does is a simple conversion from the CPE format into something the PlayStation can understand out of the box, that is a PS-X EXE file."_ PS-EXE files can be directly run into the console/emulator without the hassle of making a CD release. Data (for example images and sounds) **must** be embedded as C/ASM data array (created with tools such as `bin2h`). PS-EXE files cannot load external data. PS-X executable are great if you want to code a demo, I suggest creating your first executable this way.

PS-EXE files can injected into RAM of the console/emulator with PSXSERIAL.

If you want to release more rich stuff with a lot of audio and image assets, at some point you'll need to stop embedding stuff as C files and create a "fat" PS-EXE, you'll fill up the available heap; you should have a directory tree with all the resources and assets and you load them as needed.

Making a PS1 CD only differs from the standard procedure (i.e. `mkisofs`) in that you need to embed a region license file (leaked from the official SDK since the dawn of time). I think that's just a formal step to create a valid PSX CD since a modded PSX can run games from any region, therefore it doesn't really matter which license file you will embed.

Needless to say, any CD you will produce *will not* run on a unmodded console. A CD has also the advantage that you can run your game on your neighbours' consoles, while PS-EXE files can only be loaded with PSXSERIAL.

Tools to make a CD:
* **PSXISOMaker**: a GUI too to guide you in the process
* **mkpsxiso**: some as above but using a CLI tool that I've put together a while ago recompiling for Win32 the Linux version of this tool.
* **edit.bat**: a script to automate `start notepad ISOROOT\system.cnf` where you need to specify the main executable that will be searched and launched from the CD as first thing. Usually it's `MAIN.PSX`, no need to change it.
* **makecd.bat**: a script that uses `mkpsxiso` to automate the procedure.

## Additional libraries
Using the bare SDK for PS1 is incredibly useful to understand how things really work under the hood but at some point you really want to abstract a bit, especially when creating 3D primitives and pipelining data towards the GPU. Most third-party developers back in the day used their own libraries that abstracted the common tasks from the SDK.

I use a very useful thin layer ([PSXLIB](http://onorisoft.free.fr/psx/tutorial/tuto.htm)) by a French hero called _Orion_\_ that wraps the most basic system calls and helps carrying out common tasks concerning sound, sprites, background, data access, even user dialogs (!) and so on. Read the headers to see what you have available.

Third-party developers back in the day were heroes anyway because they optimized critical code writing ASM for the R3000A CPU.

There's also an effort to reimplement an open-source version of a PS1 SDK for Linux (see [References](#references)) but it is limited to 2D stuff and the project it's not very active.

## Compile
After cloning the repo, check paths in the `Makefile`, then run `make`. You should end up with some stuff in the `ISOROOT` directory. If you just want to use the PS-EXE file pick `MAIN.PSX` and use it with the console/emulator. If you want to make a CD run `makecd.bat`, you should see two new files appearing: `psxiso.cue` and `psxiso.bin`.

## Run and debug (emulator and hardware)
Third-party developers used additional hardware (Sony `DTL-Hxxx` boards and units) to test and debug their software before sending the "gold" CD to Sony QA.

Today, thanks to modchips, we can use a plain modded PS1 (that you can get anywhere at a reasonable price) attached to your PC through a serial cable (PS1 serial plug on one end, USB on the other). Having the schematics, you can build your own or buy one like [this](http://www.psxdev.net/forum/viewtopic.php?f=24&t=410).

You can run your software (either a `PS-EXE` or a `.bin/.cue` file) on an emulator:
* [ePSXe](http://www.epsxe.com/) (faster but less accurate)
* [no$psx](http://problemkaputt.de/psx.htm) (more accurate, incredibly detailed and documented)

These commands can be saved as desktop links:
```
# run your PS-EXE or BIN on ePSXe
C:\psyq\emulators\ePSXe190\ePSXe.exe -nogui MAIN.EXE
C:\psyq\emulators\epsxe190\ePSXe.exe -nogui -loadbin psxiso.bin

# run your PS-EXE or BIN on no$psx
C:\psyq\emulators\nopsx\NO$PSX.EXE "C:\psyq\emulators\nopsx\MAIN.EXE"
C:\psyq\emulators\nopsx\NO$PSX.EXE "C:\psyq\emulators\nopsx\psxiso.bin"
```

or on real hardware using the above mentioned serial cable and a small software ([PSXSERIAL](http://www.psxdev.net/forum/viewtopic.php?f=69&t=378&hilit=psxserial)) running on the PS1 that receives data from the cable, injects in memory your game and soft reboots the console.

**Note**: as far as I know you can directly upload only `PS-EXE` files into the console.

Upload your game on the PS1 with this command:
```
C:\psyq\tools\psxserial-012\PSXSERIAL.exe MAIN.EXE COM4
```
where `COM4` is the serial port you mapped when you installed PSXSERIAL.

**Note**: I've successfully transferred data from a virtualized XP instance running from Virtualbox to the console: you have to share into Virtualbox the USB device that is installed on the host PC.

## Fun facts
The compiler (`ccpsx`) was developed by SN Systems Software Ltd ([they're still in business!](http://www.snsystems.com/)).

The core C library is implemented following closely the GNU one (therefore you have `malloc()`, `srand()`, etc.). Comparing the standard GNU C library, some of the core functions are missing, others are implemented specifically for the platform.

During the lifespan of the PS1, Sony released updates to their SDK. You will find __three__  malloc/calloc/realloc/free/etc functions (f.e. _malloc_, _malloc2_ and _malloc3_) because earlier versions were buggy and leaked memory. Always link MMGMNEW.OBJ and __use the most up to date ones!__

## References
* Help and support (a goldmine, really): [PSX dev forum](http://www.psxdev.net/forum)
* Downloads (PDF Manuals, SDK, IDE, tools, etc.): [PSX dev download section](http://www.psxdev.net/downloads.html)
* Linux SDK: [PSXSDK](http://unhaut.x10host.com/psxsdk/) by Giuseppe Gatta
* [Sources for the pxfinder tool](https://github.com/apiraino/psx_pxfinder)

## Disclaimer
This repo includes tools freely available in order to save some time to get started. Credit is due (and a **big** thank you) to respective owners.

Technically speaking, some of the code in this repo is still copyrighted by Sony. However so many moons have passed since the PS1 was a profitable platform that I highly doubt anyone will be sued.

The Playstation 1 was some serious piece of hardware (kudos to the engineers) and this is just for learning purposes. We are just a bunch of retrogaming enthusiasts.
