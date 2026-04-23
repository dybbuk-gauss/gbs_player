# gbs_player

A program for playing Game Boy sound (.gbs) files.

modified for windows compatibility
from Original repository : https://github.com/frestr/gbs-player 

## Compile

To compile, do

```
cmake --build build --config Release
```

## Usage

Run the program with your .gbs file as an argument:

```
$ ./player 
Usage: ./player <gbs-file> <track number>
```

### Controls

* `n`: next song
* `p`: previous song
* `q`: quit player

## Example

```
$ ./player DMG-TRA-0.gb
GBS version:      1
Song count:       17
First song:       1
Load address:     0x64e5
Init address:     0x64e5
Play address:     0x6553
Stack pointer:    0xcfff
Timer modulo:     0x0
Timer control:    0x0
Title:            Tetris v1.0
Author:           Hirokazu Tanaka
Copyright:        1989 Nintendo
Playing song no. 1
```
