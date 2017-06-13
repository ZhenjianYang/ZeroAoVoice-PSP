# ZeroAoVoice-PSP

This projcet is still in progress, its goal is making voice patches for 
the PSP games *Zero no Kiseki* & *Ao no Kiseki*

**NOTE:** This projcet is licensed under the GPLv3. You MUST copy,
distribute and/or modify any code or binaries from this projcet under
this license. See
[LICENSE](https://github.com/ZhenjianYang/SoraVoice/blob/master/LICENSE)
for details

**NOTE:** Do NOT upload any offical resources (The entire game or game
resources like voice files, Chinese or Japesene texts, scena files,
etc.) or anything modified from them.


## 1.Build

You should install psptoolchain and pspsdk fisrt.

You can build them by following the project [psptoolchain](https://github.com/pspdev/psptoolchain) (with gcc 4.9.3), or
download and install a prebuild pspsdk [Minimalist PSPSDK](https://sourceforge.net/projects/minpspw/) (with gcc 4.3.5).

After all is ready. You can build this projcet with entering the root path of it and typing command `make`.

Then you will get 4 files under folder "bin" :

- **za_voice.prx**               : main module of this project.

- **EBOOT.BIN**                  : the loader, used to load za_voice.prx and the game's BOOT module.

- **EBOOT.PBP** & **PARAM.SFO**  : just for testing, no need to care about them.


## 2.How to let it work

1.  Donwload [WQSG_UMD](http://www.brewology.com/downloads/download.php?id=11249&mcid=1), and open you PSP game image (.iso) with it.
    Then extract **/PSP_GAME/SYSDIR/EBOOT.BIN**, **PSP_GAME/PARAM.SFO**, and the whole folder **/PSP_GAME/USRDIR/data/scena/**

2.  Open extracted **PARAM.SFO** with [SFOEditor](https://sites.google.com/site/theleecherman/sfoeditor) and add a param **MEMSIZE**
    with value **1**

3.  Decrypt **EBOOT.BIN** with **PRXdecrypter 2.7a** (Runs on a PSP or PSVita) or **PPSSPP** (Check on **_Setting_** -> **_Tools_** ->
    **_Developer tools_** -> **_Dump decrypted EBOOT.BIN on game boot_**, and run the game. Then you will get decrypted EBOOT.BIN under
    **(memorystick)/PSP/SYSTEM/DUMP** with a name like **(game-id).bin)**.   
    (**This step is NOT required if you run the game with PPSSPP**)

4.  Rename decrypted **EBOOT.BIN** to **BOOT.BIN**

5.  Drag **PARAM.SFO**, **BOOT.BIN**, and **EBOOT.BIN** built above back to the game image. Now you can close WQSG_UMD.

6.  Extract voice files(*.at9) from the PSVita edition game ("Evo" edition), and convert them to supported formats(describe later).   
    You should keep the voice files' names not changed (except the extention).  
    **e.g.**  **v1234567.at9** should be converted to **v1234567.wav**, **v1234567.ogg**, etc.

7.  Put converted voice files to **(memorystick)/PSP/za_voice/(game)/(ext)/**  
    **(game)** should be **zero** (for *Zero no Kiseki*) or **ao** (for *Ao no Kiseki*)   
    **(ext)** is the extention of the voice files   
    **e.g.**  wav format voice files for *Zero no Kiseki* should be put to **(memorystick)/PSP/za_voice/zero/wav/**.

	***OR*** (**This way is recommended**)   
	Pack converted voice files with [tools/PackVoiceFiles](https://github.com/ZhenjianYang/ZeroAoVoice-PSP/tree/master/tools/PackVoiceFiles),
	rename the packed file to **voice.pak** and put it under **(memorystick)/PSP/za_voice/(game)/**

8.  Modify extracted script files(*.bin under folder scena) by adding voice instrucions. (Details will be described later)

9.  Add Modified script files to game image. (Details will be described later)

10.  Put **za_voice.prx** to **(memorystick)/PSP/za_voice/**

11. Now you can launch your game. If everything is OK, you can enjoy the voice like the "Evo" edition.

## 3.Limitations

1.  **Memory**

    For technical reasons, the main module za_voice.prx runs on user mode. This means it shares the user mode memory with the game.
    As default, the PSP system provides 24MB memory for user mode, and the game will use most of it. Little is left for our module.
    The original PSP model (1000) has no more memory, so there's no way to run our module now. But later models (2000,3000,go) has
    extra 32MB physical memory (for UMD cache), and is possible to enable them for user mode.
    za_voice.prx is able to run on a system if the extra memory is enabled. 

    Here are ways to enable the extra memory in some systems.

    - **PSVita (Henkaku, Adrenaline 4.1+)**   
      Enable extra memory by   
      **_XMB_** -> **_ADRENALINE VSH MENU_** -> **_RECOVERY MENU_** -> **_Advanced_** -> **_Advanced configuration_** ->   
      Set **_Force high memory layout_** to **_Enable_**

    - **PPSSPP (Latest version)**   
      Enable extra memory by   
      **_Settings_** -> **_System_** ->   
      Set **_PSP model_** to **_PSP-2000/3000_**

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

    - **at3** (recommended)    
        Hardware decoding.  
        You can convert wav to at3 with at3tool.   
        at3tool can only convert a wav with sample rate 44100Hz, but the original voice files' sample rate
        is 48000Hz. So you must change the wav files' sample rate by a resample software before converting them
        to at3.

    When the game lanching, za_voice.prx will check voice folders/pack in turn, the first one found will be seleted as
    the format. The turn is:  
      `pak -> at3 -> ogg -> wav`

    **Note for PPSSPP users:**
      ~~~~
      It seems there's a bug with PPSSPP.   
      za_voice.prx uses PSP system's APIs sceAudioSRCCh* to play sound files.  
      These APIs supports multiple sample rates : 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11050, 8000  
      But in PPSSPP, no matter which sample rate is set, it will play the sound file as 44100Hz.  
      So you must resample all voice files to 44100Hz if you are using PPSSPP.  
      (oggenc2 supports resampling during conversion) 
      ~~~~

## 4.Settings

1.  **Configuration file**   

The configuration file for za_voice.prx is **(memorystick)/PSP/za_voice/za_voice.ini**.   
List settings here:

- **Volume**   
    Voice's volume, 0~100, default is 100.

- **AutoPlay**   
    Auto play the dialog.   
	Default is 2.   
	- **0** : Off
	- **1** : Enable if the dialog has voice
	- **2** : Enable for all dialogs   
	Waiting time of **AutoPlay** could be set with next 3 settings.

- **WaitTimePerChar**   
    Default is 60.   

- **WaitTimeDialog**   
    Default is 800.   
	Then with default settings, waiting time for a non-voice dialog with 20 characters will be 60*20+800=2000.

- **WaitTimeDialogWithVoice**   
    Default is 500.   
	Then with default settings, waiting time for a dialog with woice will be 500 + *length of voice*.

**NOTE:** The time unit of above 3 settings is millisecond. But inside the module, we use *Frame* as time unit.
And we assume:    
    `30 Frames = 1 second = 1000 milliseconds`

- **SkipVoice**   
    When the dialog is closed. Whether stop the voice or not.    
	Default is 1.   
	- **0** : Off (Do not stop)
	- **1** : On (Stop)

- **DisableDialogTextSE**   
    Whether disable texts' se (sounds like du du du) when playing voice.    
	Default is 1.   
	- **0** : Off (Do not disable se)
	- **1** : On (Disable se)

- **DisableDialogSwitchSE**   
    Whether disable dialog swiching/closing se when playing voice.    
	Default is 1.   
	- **0** : Off (Do not disable se)
	- **1** : On (Disable se)

- **DisableOriginalVoice**   
    For *Ao no Kiseki*, there are two kinds of voices beside battle voice: reaction voices and scene voices.   
	This setting could disable the scene voices.   
	(*Zero no Kiseki* has only reaction voices, so this setting has no effict with *Zero no Kiseki*)   
	Default is 1.   
	- **0** : Off (Do not disable original scena voices)
	- **1** : On (Disable original scena voices)

- **ShowInfo**   
    Some settings(describe later) could be changed during the game playing. When these settings changed
	whether show the infomation about it.   
	Default is 1.
	- **0** : Off (Do not show)
	- **1** : On (Show the infomation)
	
- **PPSSPP**   
	Default is 0.   
    If you are using PPSSPP and want **ShowInfo** work, then set this setting to 1, otherwise keep it 0.   
	**NOTE:** Never set it to 1 if you run your game in a real PSP or PSVita.

2.  **Change settings during game playing**

**When there's a dialog in the screen**, some settings mentioned above could be changed with hotkeys.   
List them here:   

- **SQUARE** + **RIGHT**   
	Swich setting **AutoPlay**  

- **SQUARE** + **UP**/**DOWN**   
	**Volume** +/- 1   

- **SQUARE** + **TRIANGLE** + **UP**/**DOWN**   
	**Volume** +/- 5   

**NOTE:** Sometimes hotkeys are not available even if a dialog is showing.

## 5.About the script files

1. **Voice instrucions's format**

    The original voice instrucions in "Evo" edition have format like `#123456789V`.   
    za_voice.prx uses similar format :  `#123456789v`.  
    The only different is the last letter is lowercase v, because *Ao no Kiseki* has some scene voice and the
    voice instrucions are also uppercase V.

2. **Add voice instrucions to script files**

    This is really not easy work, follow these 3 steps:  
    1. **Decompile the script files**  
    2. **Add voice instrucions to decompiled scripts**
    3. **Re-compile the scripts**

    About how to decompile/recompile script files, pelease refer to [this projcet](https://github.com/ZhenjianYang/EDDecompiler)   
    And in [this project](https://github.com/ZhenjianYang/SoraVoice), there may be some tools useful for step 2.

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

## External libraries used in this project

-   [PSPSDK](https://github.com/pspdev/pspsdk), licensed under the
    [BSD 3-clause "New" or "Revised" License](https://github.com/pspdev/pspsdk/blob/master/LICENSE).

-   [libVorbis & libOgg](https://www.xiph.org/), licensed under the
    [BSD-like license](https://www.xiph.org/licenses/bsd/).

------------------------------------------------------------------------

------------------------------------------------------------------------

# ZeroAoVoice-PSP

这是进行中的工程，目标为制作PSP游戏《零·碧之轨迹》系列的语音补丁

**注意：** 本项目基于GPLv3开源协议，对本项目的任何代码或二进制文件的复制、修改、分发需遵循此协议。
具体细节请参见[LICENSE](https://github.com/ZhenjianYang/SoraVoice/blob/master/LICENSE)文件

**注意：** 请勿在此项目中上传任何官方或基于官方修改的资源（游戏本体，或其资源文件如语音文件、中日文本、脚本文件等）

## 4.设置

1.  **配置文件**    

配置文件路径为 **(memorystick)/PSP/za_voice/za_voice.ini**.   
罗列配置项如下:

- **Volume**   
    语音音量, 0~100, 默认值为最大值100。

- **AutoPlay**   
    对话框自动播放   
	默认值2   
	- **0** : 关闭
	- **1** : 有语音时开启
	- **2** : 总是开启   
	自动播放的等待时间有接下来3个选项控制。

- **WaitTimePerChar**   
    默认值60   

- **WaitTimeDialog**   
    默认值800.   
	则在默认配置下, 1个20字的无语音对话框的等待时间为60*20+800=2000.

- **WaitTimeDialogWithVoice**   
    默认值500.   
	则在默认配置下, 1个有语音对话框的等待时间为 500 + *语音长度*.

**注意:** 以上3个配置使用的时间单位为毫秒，但是在内部使用的时间单位为*帧*，  
	我们假设： `30 帧 = 1 秒 = 1000 毫秒`

- **SkipVoice**   
    对话框关闭时，是否终止语音    
	默认值1.   
	- **0** : 不终止语音
	- **1** : 终止语音

- **DisableDialogTextSE**   
    有语音时，是否禁用对话框文字显示音效(嘟嘟声)。     
	默认值1.   
	- **0** : 不禁用
	- **1** : 禁用

- **DisableDialogSwitchSE**   
    有语音时，是否禁用对话框关闭/切换音效。     
	默认值1.   
	- **0** : 不禁用
	- **1** : 禁用

- **DisableOriginalVoice**   
   对于《碧之轨迹》，除战斗语音外，还有部分语气语音和剧情语音。  
	本项目可设置是否禁用原有的剧情语音。   
	(《零之轨迹》无剧情语音，故本配置项对《零之轨迹》无效)   
	默认值1.   
	- **0** : 不禁用原有剧情语音
	- **1** : 禁用原有剧情语音

- **ShowInfo**   
    部分设置(稍后说明)可以在游戏进行过程中修改，当这些配置被修改时，  
	是否显示信息。   
	默认值1.
	- **0** : 不显示
	- **1** : 显示
	
- **PPSSPP**   
	默认值0.   
    如果您在使用PPSSPP并且希望上述的**ShowInfo**能正常工作的话，将本选项设置为1, 否则让它保持为0。   
	**注意:** 当使用实机或PSVita运行时，绝对不要将本选项设为1.

2.  **在游戏进行中修改配置**

**当屏幕中有对话框显示时**, 部分配置可通过快捷键修改：   
罗列如下:   

- **方块** + **右**   
	切换配置项 **AutoPlay**  

- **方块** + **上**/**下**   
	**Volume** +/- 1   

- **方块** + **三角** + **上**/**下**    
	**Volume** +/- 5 

## 本项目使用的外部库

-   [PSPSDK](https://github.com/pspdev/pspsdk), 基于
    [BSD 3-clause "New" or "Revised" License](https://github.com/pspdev/pspsdk/blob/master/LICENSE).

-   [libVorbis & libOgg](https://www.xiph.org/), 基于
    [BSD-like license](https://www.xiph.org/licenses/bsd/).




