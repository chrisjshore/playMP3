# README

Simple console MP3 player.

## Dependencies

1. miniaudio

miniaudio can be installed from AUR on Arch based systems.

``` bash
yay -S miniaudio-git
```

## Building

Change into cloned directory and run make

## Usage

Pass an audio file to the program, press the spacebar to pause and resume, press q to quit.  The program will automatically quit when the audio file is done playing.

``` bash
./play ~/Music/someaudiofile.mp3
```