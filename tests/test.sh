#!/bin/bash
set -e

# TODO: systematic approach

self="$(readlink -f "$0")"
dir="${self%/*}"
pdir="${dir%/*}"
cd -- "$pdir"
make
mkdir -p tmp

function test()
{
  i="$1"
  shift
  expected="$1"
  shift
  input="$1"
  shift
  echo "test $i"
  echo "hexgrep $@"
  ./hexgrep "$@" -f "$dir/$input" > tmp/output.txt
  diff "$dir/$expected" tmp/output.txt
  echo "file search succeeded"
  ./hexgrep "$@" < "$dir/$input" > tmp/output.txt
  diff "$dir/$expected" tmp/output.txt
  echo "stream search succeeded"
  echo
}

i=0

i=$(($i + 1))
test $i output_789.txt 123.dat /789

i=$(($i + 1))
test $i output_789.txt 123.dat /7 /8 /9

i=$(($i + 1))
test $i output_789.txt 123.dat 0x373839 -b

i=$(($i + 1))
test $i output_789.txt 123.dat 0x393837

i=$(($i + 1))
test $i output_789.txt 123.dat /789 -l 5



i=$(($i + 1))
test $i output_GT9.txt 123.dat '>/9'

i=$(($i + 1))
test $i output_GT9.txt 123.dat '>071'



i=$(($i + 1))
test $i output_LT5_star_GTc.txt 123.dat '</5' '*' '>=/c'



i=$(($i + 1))
test $i output_LT5_star_GTc_r.txt 123.dat '</5' '*' '>=/c' -r 0x20:0x10:



i=$(($i + 1))
test $i output_NEabcd.txt 123.dat '<>/abcd'



echo "all $i tests passed"
