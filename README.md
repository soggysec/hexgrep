# Origin

Original Version: 2013.9.6
Forked from https://xyne.archlinux.ca/projects/hexgrep/src/

# Updates

Added -R option which will allow you to recursively search files.

# About
hexgrep is a command-line tool for searching for byte sequences in binary data. The tool implements its own simplified query language that supports inequalities and wild cards. Values may be given in decimal, hexadecimal, octal or string format, and the formats may be freely mixed. Multibyte values are supported and their byte translation can be controlled with the endianness flag.

The man page and the following examples should hopefully clarify how this works.

# Examples
Simple Sequences
Find all occurrences of the string “pacman” in the pacman executable:

```shell
$ hexgrep /pacman -f /usr/bin/pacman
007b2e 70 61 63 6d 61 6e
007c27 70 61 63 6d 61 6e
013a6d 70 61 63 6d 61 6e
013a7e 70 61 63 6d 61 6e
013a94 70 61 63 6d 61 6e
013e56 70 61 63 6d 61 6e
0148a7 70 61 63 6d 61 6e
0148b4 70 61 63 6d 61 6e
014989 70 61 63 6d 61 6e
```
Same thing using byte values:

```shell
$ hexgrep 0x70 0x61 0x63 0x6D 0x61 0x6E -f /usr/bin/pacman
007b2e 70 61 63 6d 61 6e
007c27 70 61 63 6d 61 6e
013a6d 70 61 63 6d 61 6e
013a7e 70 61 63 6d 61 6e
013a94 70 61 63 6d 61 6e
013e56 70 61 63 6d 61 6e
0148a7 70 61 63 6d 61 6e
0148b4 70 61 63 6d 61 6e
014989 70 61 63 6d 61 6e
```
Again, with mixed values:

```shell
$ hexgrep /pac 0x6D 97 0x6E -f /usr/bin/pacman
007b2e 70 61 63 6d 61 6e
007c27 70 61 63 6d 61 6e
013a6d 70 61 63 6d 61 6e
013a7e 70 61 63 6d 61 6e
013a94 70 61 63 6d 61 6e
013e56 70 61 63 6d 61 6e
0148a7 70 61 63 6d 61 6e
0148b4 70 61 63 6d 61 6e
014989 70 61 63 6d 61 6e
```
And finally with a single, multibyte hexadecimal integer (note the endianness flag) :

```shell
$ /hexgrep 0x7061636d616e -b -f /usr/bin/pacman
007b2e 70 61 63 6d 61 6e
007c27 70 61 63 6d 61 6e
013a6d 70 61 63 6d 61 6e
013a7e 70 61 63 6d 61 6e
013a94 70 61 63 6d 61 6e
013e56 70 61 63 6d 61 6e
0148a7 70 61 63 6d 61 6e
0148b4 70 61 63 6d 61 6e
014989 70 61 63 6d 61 6e
```
Negative Matches
Find all instances of “man” not prefixed with “pac”:

```shell
hexgrep '<>/pac' /man -f /usr/bin/pacman
007c74 08 48 b9 6d 61 6e
008785 63 48 ba 6d 61 6e
013ab7 43 6f 6d 6d 61 6e
013ad9 63 6f 6d 6d 61 6e
013d6b 63 6f 6d 6d 61 6e
013f2c 43 6f 6d 6d 61 6e
013f4c 43 6f 6d 6d 61 6e
013f54 63 6f 6d 6d 61 6e
015ccf 50 61 63 6d 61 6e
015d1f 50 61 63 6d 61 6e
017de0 67 65 20 6d 61 6e
```
Wildcards
Find all occurences of “p” followed by at most 20 bytes before “man”.

```shell
$ hexgrep /p '#<=20'  /man -f /usr/bin/pacman
007b2e 70 61 63 6d 61 6e
007c27 70 61 63 6d 61 6e
007c64 70 61 63 c7 40 10 75 70 67 2f c6 40 14 00 48 89 08 48 b9 6d 61 6e
007c6b 70 67 2f c6 40 14 00 48 89 08 48 b9 6d 61 6e
008783 70 61 63 48 ba 6d 61 6e
013a6d 70 61 63 6d 61 6e
013a7e 70 61 63 6d 61 6e
013a94 70 61 63 6d 61 6e
013e56 70 61 63 6d 61 6e
0148a7 70 61 63 6d 61 6e
0148b4 70 61 63 6d 61 6e
014989 70 61 63 6d 61 6e
017ddb 70 61 63 6b 61 67 65 20 6d 61 6e
```
Adding the “-c” flag to the above will color the matched bytes for “p” and “man”:

```shell
$ hexgrep /p '#<=20'  /man -f /usr/bin/pacman -c
007b2e 70 61 63 6d 61 6e
007c27 70 61 63 6d 61 6e
007c64 70 61 63 c7 40 10 75 70 67 2f c6 40 14 00 48 89 08 48 b9 6d 61 6e
007c6b 70 67 2f c6 40 14 00 48 89 08 48 b9 6d 61 6e
008783 70 61 63 48 ba 6d 61 6e
013a6d 70 61 63 6d 61 6e
013a7e 70 61 63 6d 61 6e
013a94 70 61 63 6d 61 6e
013e56 70 61 63 6d 61 6e
0148a7 70 61 63 6d 61 6e
0148b4 70 61 63 6d 61 6e
014989 70 61 63 6d 61 6e
017ddb 70 61 63 6b 61 67 65 20 6d 61 6e
```

## Higan Cheat Codes
Here’s a real usage example with cheat-code hacking. The multi-system emulator higan can export a game’s working memory to a file (the command is in the hotkey menu). The file, work.ram, contains in-game memory data.

Let’s say that you want to create cheat codes to increase a character’s HP and MP. The current values are 63/63 and 24/24. To locate the memory offsets in file, you need to find the sequence of values 63, 63, 24, 24 (assuming they are in the order that they appear in the menu), without knowing how many bytes are between them. Let’s assume they are less than 10 bytes appart. Here’s how you do it with hexgrep:

```shell
$ hexgrep 63 #<10 63 #<10 24 #<10 24 -f work.ram -c
001609 3f 00 3f 00 18 00 18
00a0d3 3f 21 3f 23 3f 1e 3f 46 7f 5c 7f 5e 7f 18 18
00a0d5 3f 23 3f 1e 3f 46 7f 5c 7f 5e 7f 18 18
00a0d7 3f 1e 3f 46 7f 5c 7f 5e 7f 18 18
0156cd 3f 3f ff 18 18
```
You can see a number of hits, but the first is is interesting because it has regular spacing between each matched byte. If you know that the max values for HP and MP are 9999, then you know they are represented by 2 bytes each, which fits the first match.

If you get lots of hits for a search, change the values in game (e.g. by taking hits), dump the memory again, search for the new values, and look for common offsets with the previous hits.

Once you have the right offsets, you can create cheat codes using Higan’s built-in cheat code manager. I’m not going to give a tutorial here, but basically there are ROM-hacking codes and RAM-hacking codes. The latter are what we use for values found in the dumped memory. In this case, the codes follow the format 7exxxx:yy where xxxx is an offset and yy is the value. So, to boost MP up to 255/255, you would use

```shell
7e160d:ff
7e160f:ff
```
After confirming that these codes work, there are two possibilities for boosting MP up to 9999/9999:

```shell
7e160c:27
7e160d:0f
7e160e:27
7e160f:0f
and

7e160d:0f
7e160e:27
7e160f:0f
7e1610:27
```

The ambiguity arises from only having a single-byte value with 00 on either size. After you determine which one is correct, you can do the same for HP, and then use hexgrep to locate any other value in the game and then change it.

# TODO
optimize as much as possible
maybe add “-q” option to limit output to offsets
maybe add support for alternation, e.g. (0xff 0x80) would match either value
HEXGREP(1)
Description
hexgrep - search for strings and byte patterns in binary data with wildcards

Usage
hexgrep options -f <path> [search tokens]

hexgrep [options] [search tokens] < <filepath>

<command> | hexgrep [options] [search tokens]

# Options

```shell
-b Convert values greater than 0xff to a big-endian sequence of bytes. The default is little-endian.
-c Color output.
-d Print debugging information. Pass multiple times to display more information.
-f <path> Search a file. The file will be mmap’d and match sequences may span the full file. Without this option, an input stream from STDIN will be searched.
-h Display the help message and exit.
-l <size> The maximum length of a match sequence when reading a stream. The allocated buffer will be twice this size. The size is given in bytes as an integer optionally followed by k, M, G, T, P, E, Z, or Y. Yes, it really will try to allocate terabytes or more if you tell it to.
-p <int> Pad offsets with zeros up to this width. Default: 6.
-r <range specifier> Limit the search range. See “Range Specifier” below for details.
Values Values may be specified in decimal, hexadeximal, octal or string format:

Decimal value format
-?[0-9]+
Hexadecimal value format
-?0[xX][0-9a-fA-F]+
Octal value format
-?0[0-7]+
String value format
/*
```
Values that do not begin with “/” are parsed with strtol internally. String values are converted to bytes directly.

Range Specifier
A range specifier will limit the search to a given range.

# Format
<start>?:<length>?:<end>?
<start>, <length> and <end> are values as described in “Values”. At most 2 of these values may be given. If 3 are given then <end> will be ignored. Positive values for <start> and <end> are interpretted as offsets from the start of the input. Negative values are interpretted as offsets from the end of the input. The sign of <length> is ignored and the value will be treated as positive. Negative values are not supported when parsing streams as this would require loading the full stream into memory or saving it to a file. If you need this, dump the stream to a file and then use the -f option.

## Examples
10:20:
Skip the first 10 bytes of the input and search in the 20 bytes that follow.
5::-5
Skip the first and last 5 bytes of the input.
:0x100:-0xf
Limit the search to the sequence of 256 bytes that are 15 bytes away from the end of the input.
Search Token Specifier
Search tokens either specify sequences of bytes to match or numbers of bytes to skip. The latter begin with “#”.

## Numerical format
#?<equality>?<value>
The equalities are “=”, “>=”, “>”, “<=”, “<”, “<>”. “=” is the default, “<>” is “greater than or less than”, i.e. “not equal”.

Examples
40 50
Match a byte equal to 40 followed by a byte equal to 50.
40 #2 50
Match a byte equal to 40 and a byte equal to 50 separated by 2 bytes.
0xE3 #>=5 0xFF
Match a byte equal to 227 followed at least 5 bytes later by a byte equal to 255.
>=0xA
Match a byte that is equal to or greater than 10 in value
Shortcuts
The following shortcuts are supported.

?
#=1
*
#>=0
+
#>=1
# Help Message
```shell
$ hexgrep -h

Usage
  hexgrep [options] [search tokens]

Options
  -b
      Convert values greater than >0xff to a big-endian sequence of bytes. The
      default is little-endian.

  -c
      Color output.

  -d
      Print debugging information. Pass multiple times to display more information.

  -f <path>
      Search a file. The file will be mmap'd and match sequences may span the full
      file. Without this option, an input stream from STDIN will be searched.

  -h
      Display this help message and exit.

  -l <length>
    The maximum length of a match sequence when reading a stream. The allocated
    buffer will be three times. The size is given in bytes as an integer
    optionally followed by k, M, G, T, P, E, Z, or Y. Yes, it really will try to
    allocate terabytes or more if you tell it to. Default: STREAM_MATCH_LENGTH bytes.

  -p <int>
      Pad offsets with zeros up to this width. Default: 6.

  -r <range specifier>
      Limit the search range. See "Range Specifier" in the manual.

See the man page for details.
```