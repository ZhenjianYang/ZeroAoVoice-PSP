ZeroAoVoice-PSP
===============

This projcet is still in progress, its goal is making voice patches for the PSP games *Zero/Ao no Kiseki*

**NOTE:** This projcet is licensed under the GPLv3. You MUST copy,
distribute and/or modify any code or binaries from this projcet under
this license. See
[LICENSE](https://github.com/ZhenjianYang/SoraVoice/blob/master/LICENSE)
for details

**NOTE:** Do NOT upload any offical resources (The entire game or game
resources like voice files, Chinese or Japesene texts, scena files,
etc.) or anything modified from them.


1.Build
-------

You should install psptoolchain and pspsdk fisrt.

You can build them by following the project [psptoolchain](https://github.com/pspdev/psptoolchain) (with gcc 4.9.3), or
download and install a prebuild pspsdk [Minimalist PSPSDK](https://sourceforge.net/projects/minpspw/) (with gcc 4.3.5).

After all is ready. You can build this projcet with entering the root path of it and typing command "make".

Then you will get 4 files under folder "bin" :

- **za_voice.prx**               : main module of this project.

- **EBOOT.BIN**                  : the loader, used to load za_voice.prx and the game's BOOT module.

- **EBOOT.PBP** & **PARAM.SFO**  : just for testing, no need to care about them.


2.How to let it work
--------------------

1.  Donwload [WQSG_UMD](http://www.brewology.com/downloads/download.php?id=11249&mcid=1), and open you PSP game image (.iso) with it.
    Then extract **/PSP_GAME/SYSDIR/EBOOT.BIN**, **PSP_GAME/PARAM.SFO**, and the whole folder **/PSP_GAME/USRDIR/data/scena/**

2.  Open extracted **PARAM.SFO** with [SFOEditor](https://sites.google.com/site/theleecherman/sfoeditor) and add a param **MEMSIZE** with value **1**

3.  Rename extracted **EBOOT.BIN** to **BOOT.BIN**

4.  Drag **PARAM.SFO**, **BOOT.BIN**, and **EBOOT.BIN** built above back to the game image. Now you can close WQSG_UMD.

5.  Extract voice files(*.at9) from the PSVita edition game ("Evo" edition), and convert them to supported formats(describe later).

6.  Put converted voice files to **(memorystick)/PSP/za_voice/(game)/(ext)/**  
    **(game)** should be **zero** (for *Zero no Kiseki*) or **ao** (for *Ao no Kiseki*)   
    **(ext)** is the extention of the voice files   
    **e.g.**  wav format voice files for *Zero no Kiseki* should be put to **(memorystick)/PSP/za_voice/zero/wav/**

7.  Modify extracted script files(*.bin under folder scena) by adding voice instrucions. (Details will be described later)

8.  Add Modified script files to game image. (Details will be described later)

9.  Put **za_voice.prx** to **(memorystick)/PSP/za_voice/**

10. Now you can launch your game. If everything is OK, you can enjoy the voice like the "Evo" edition.

3.Limitations
-------------

1.  **Memory.**

    For tecnical reasons, the main module za_voice.prx runs on user mode. This means it shares the user mode memory with the game.
    As default, the PSP system provides 24MB memory for user mode, and the game will use most of it. Little is left for our module.
    The original PSP model (1000) has no more memory, so there's no way to run our module now. But later models (2000,3000,go) has
    extra 32MB physical memory (for UMD cache), and is possible to enable them for user mode.
    za_voice.prx is able to run on a system if the extra memory is enabled. 

    Here are ways to enable the extra memory in some systems.

    - **PSVita (Henkaku, Adrenaline 4.1)**   
      Enable extra memory by   
      XMB -> ADRENALINE VSH MENU -> RECOVERY MENU -> Advanced -> Advanced configuration ->   
      Set **Force high memory layout** to **Enable**

    - **PPSSPP**   
      Enable extra memory by   
      Settings -> System ->   
      Set **PSP model** to **PSP-2000/3000**

    - **Real PSP 2000/3000/go/E1000**   
      There should be a way to enable the extra memory.   
      Because I don't have any of them, so I'm not sure if it is really possible. 
         
2.  **Supported voice formats.**

    The original voice files in the "Evo" edition are at9 files. You can convert them to wav files with at9tool.  
    And then convert them to supported formats list below:

    - **wav**  
        None compressive format. Lowest CPU pressure.  
        But it costs more I/O time because the data is such big.

    - **ogg**  
        Software decoding.  
        You can convert wav to ogg with [oggenc2](http://www.rarewares.org/ogg-oggenc.php).

    - **at3**  
        Hardware decoding.  
        You can convert wav to at3 with at3tool.   
        at3tool can only convert a wav with sample rate 44100Hz, but the original voice files' sample rate
        is 48000Hz. So you must change the wav files' sample rate by a resample software before converting them
        to at3.

    When the game lanching, za_voice.prx will check voice folders in turn, the first one found will be seleted as
    the format. The turn is:  
    -  **at3** -> **ogg** -> **wav**

    **Note for PPSSPP users**   
      It seems there's a bug with PPSSPP.   
      za_voice.prx uses PSP system's APIs sceAudioSRCCh* to play sound files.  
      These APIs supports multiple sample rates : 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11050, 8000  
      But in PPSSPP, no matter which sample rate is set, it will play the sound file as 44100Hz.  
      So you must resample all voice files to 44100Hz if you are using PPSSPP.  
      (oggenc2 supports resampling during conversion) 


4.About the script files
------------------------

1. **Voice instrucions's format**

    The original voice instrucions in "Evo" edition have format like **#123456789V**.   
    za_voice.prx uses formallar format :  **#123456789v**.  
    The only different is the last letter is lowercase v, because *Ao no Kiseki* has some scene voice and the
    voice instrucions are also uppercase V.

2. **Add voice instrucions to script files**

    This is really not easy work, follow these 3 steps:  
    1. **Decompile the script files**  
    2. **Add voice instrucions to decompiled scripts**
    3. **Re-compile the scripts**

    About how to decompile/recompile script files, pelease refer to [this projcet](https://github.com/ZhenjianYang/EDDecompiler)   
    And in [this project](https://github.com/ZhenjianYang/SoraVoice), there may be some tools could help during step 2.

3. **Add modified script files to game image**

    The game doesn't access its resources by path, but use the resources' offset and size which are defined in /PSP_GAME/USRDIR/data.lst .
    So you must fix data.lst after you adding script files to the game image.  
    And for every script file, there's a buckup in a package file /PSP_GAME/USRDIR/data/cclm/map4/xxxxxx.mc1 (xxxxxx stands for the script's name)  
    If this backup exists, the game will read this backup instead of the one lays under /PSP_GAME/USRDIR/data/scena/  
    So if you don't want to modify the backup one, delete it from xxxxxx.mc1. (you can just change the backup's name in xxxxxx.mc1, so the game won't find it)  

    [tools/AddScnsToIso](https://github.com/ZhenjianYang/ZeroAoVoice-PSP/tree/master/tools/AddScnsToIso) in this projcet is a tool which can:
    - Add script files to the end of the game image (iso)  
    - Fix script files' offset and size in data.lst
    - Delete all script files' backup in xxxxxx.mc1 (by change their names in xxxxxx.mc1)

External libraries used in this project
---------------------------------------

-   [PSPSDK](https://github.com/pspdev/pspsdk), licensed under the
    [BSD 3-clause "New" or "Revised" License](https://github.com/pspdev/pspsdk/blob/master/LICENSE).

-   [libVorbis & libOgg](https://www.xiph.org/), licensed under the
    [BSD-like license](https://www.xiph.org/licenses/bsd/).

------------------------------------------------------------------------

------------------------------------------------------------------------

ZeroAoVoice-PSP
===============

这是进行中的工程，目标为制作PSP游戏《零·碧之轨迹》系列的语音补丁

**注意：** 本项目基于GPLv3开源协议，对本项目的任何代码或二进制文件的复制、修改、分发需遵循此协议。
具体细节请参见[LICENSE](https://github.com/ZhenjianYang/SoraVoice/blob/master/LICENSE)文件

**注意：** 请勿在此项目中上传任何官方或基于官方修改的资源（游戏本体，或其资源文件如语音文件、中日文本、脚本文件等）


本项目使用的外部库
--------------

-   [PSPSDK](https://github.com/pspdev/pspsdk), 基于
    [BSD 3-clause "New" or "Revised" License](https://github.com/pspdev/pspsdk/blob/master/LICENSE).

-   [libVorbis & libOgg](https://www.xiph.org/), 基于
    [BSD-like license](https://www.xiph.org/licenses/bsd/).




