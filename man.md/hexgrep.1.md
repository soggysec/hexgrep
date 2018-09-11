% HEXGREP(1) Hexgrep User Manuals
% Xyne
% 2013-08-22





# Description

hexgrep - search for strings and byte patterns in binary data with wildcards






# Usage

hexgrep [options] -f \<path\> [search tokens]

hexgrep [options] [search tokens] \< \<filepath\>

\<command\> | hexgrep [options] [search tokens]




# Options
-b
:   Convert values greater than 0xff to a big-endian sequence of bytes. The
    default is little-endian.

-c
:   Color output.

-d
:   Print debugging information. Pass multiple times to display more information.

-f \<path\>
:   Search a file. The file will be mmap'd and match sequences may span the full
    file. Without this option, an input stream from STDIN will be searched.

-h
:   Display the help message and exit.

-l \<size\>
:   The maximum length of a match sequence when reading a stream. The allocated
    buffer will be twice this size. The size is given in bytes as an integer
    optionally followed by k, M, G, T, P, E, Z, or Y. Yes, it really will try to
    allocate terabytes or more if you tell it to.

-p \<int\>
:   Pad offsets with zeros up to this width. Default: 6.

-r \<range specifier\>
:   Limit the search range. See "Range Specifier" below for details.







# Values

Values may be specified in decimal, hexadeximal, octal or string format:

Decimal value format
:   -?[0-9]+

Hexadecimal value format
:   -?0[xX][0-9a-fA-F]+

Octal value format
:   -?0[0-7]+

String value format
:   /*

Values that do not begin with "/" are parsed with `strtol` internally. String
values are converted to bytes directly.






# Range Specifier

A range specifier will limit the search to a given range.

Format
:   \<start\>?:\<length\>?:\<end\>?

`<start>`, `<length>` and `<end>` are values as described in "Values". At most 2 of these values may be given. If 3 are given then `<end>` will be ignored. Positive values for `<start>` and `<end>` are interpretted as offsets from the start of the input. Negative values are interpretted as offsets from the end of the input. The sign of `<length>` is ignored and the value will be treated as positive. Negative values are not supported when parsing streams as this would require loading the full stream into memory or saving it to a file. If you need this, dump the stream to a file and then use the `-f` option.

## Examples

10:20:
:   Skip the first 10 bytes of the input and search in the 20 bytes that follow.

5::-5
:   Skip the first and last 5 bytes of the input.

:0x100:-0xf
:   Limit the search to the sequence of 256 bytes that are 15 bytes away from the end of the input.








# Search Token Specifier

Search tokens either specify sequences of bytes to match or numbers of bytes to skip. The latter begin with "#".

Numerical format
:   \#?\<equality\>?\<value\>

The equalities are "=", ">=", ">", "<=", "<", "<>". "=" is the default, "<>" is "greater than or less than", i.e. "not equal".


## Examples

40 50
:   Match a byte equal to 40 followed by a byte equal to 50.

40 \#2 50
:   Match a byte equal to 40 and a byte equal to 50 separated by 2 bytes.

0xE3 \#>=5 0xFF
:   Match a byte equal to 227 followed at least 5 bytes later by a byte equal to 255.

>=0xA
:   Match a byte that is equal to or greater than 10 in value

## Shortcuts

The following shortcuts are supported.

?
:   \#=1

*
:   \#>=0

+
:   \#>=1

