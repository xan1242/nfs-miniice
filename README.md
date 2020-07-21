# MiniICE
This is a small utility designed to manipulate the NIS (Non-Interactive Scenes) in Black Box's NFS games.
Currently only works with NFS Carbon.

If you can find the equivalent functions and pointers for CAnimScene stuff, you can make it work in other NFS games very easily!

I made this back in 2018. after GetRPM, couldn't work on it much since. Feel free to have fun with this!

## Requirements for building
- IUP Library for UI: https://sourceforge.net/projects/iup/
Extract the contents in the IUP folder (readme file is in there with instructions what it should look like before compiling)

# Features
- Timeline scrubbing
- Camera track changing
- Play/Pause of an ICE scene/NIS
- Very hacky code

# Usage
- Place the MiniICE.asi file in the scripts folder (provided you installed an ASI loader beforehand).
- Preferably run the game in windowed mode to avoid issues.
- After the game starts, another window should start with it.
- Write your NIS name in the "Override NIS" box if you wish to play a different NIS, if you don't want to anymore, remove all text from the box
- Start a race (this is mandatory to initiate a NIS playback)
- To change a camera track, write the number in the box and click "Restart" to initiate the change
