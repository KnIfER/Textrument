How to build Textrument
----------------------

There are two components that need to be built separately:

 - `Textrument.exe`:  ( Main Application )
 - `SciLexer.dll` : ( Runtime dependency )
 
 I build them both with Visual Studio Community 2019.
 
 Two vital additions:
----------------------

 - `doLocalConf.xml`:  ( Indicates that user configuration should be stored in the installation path )
 - `User` : ( Folder for user-configurations )


## Build `Textrument.exe`:

 1. Open [`PowerEditor\visual.net\notepadPlus.vcxproj`](PowerEditor/visual.net/notepadPlus.vcxproj).
 
## Build `SciLexer.dll` with boost:

Follow the original instructions to build boost first: 

 1. Download the [Boost source code](https://www.boost.org/users/history/version_1_70_0.html).
 2. Unzip boost.
 3. Build regex of boost. With the version 1.70, launch `bootstrap.bat` under the boost root, `b2.exe` will be generated beside of `bootstrap.bat`. For building boost PCRE lib, go into regex build directory by typing `cd C:\sources\boost_1_70_0\libs\regex\build` then launch `C:\sources\boost_1_70_0\b2.exe toolset=msvc link=static threading=multi runtime-link=static address-model=64 release stage`.
 Note that **address-model=64** is optional if you want to build lib in 64 bits. For 32 bits build, just remove **address-model=64** from the command line.

Then  Open [`PowerEditor\visual.net\notepadPlus.vcxproj`](scintilla/win32/SciLexer.vcxproj). 

Before build, make sure that the macro $(BOOST) is defined correctly in `plugins/plugins_dev.props`. 

If you are lazy, just copy the `SciLexer.dll` kernel from the distribution package of Textrument or Notepad++.  

## Build bundled plugins :

You need to copy the [property file](plugins/plugins_dev_sample.props) as `Project_Root/plugins/plugins_dev.props`.  

A bunch of marco definitions need to be modified according to you environment.   

To modify the props properties, go to View->Property Manager, just right click and edit the Macros. dont forget to save them.  

- scintilla : path to the scintilla module

- PowerEditor : path to the PowerEditor module

- PowerEditor : path to the PowerEditor module

- BOOST : path to the boost root directory. ( required when building SciLexer.dll )

- DLLOU : default plugin output path. (`Project_Root/PowerEditor\bin64\plugins\PluginName\PluginName.dll`)

- DLLOU_ : another plugin output path to test x32 output.

How to update languages
----------------------
I use java scripts to sync language updates patch by patch.
