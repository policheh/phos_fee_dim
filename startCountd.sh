#!/bin/bash

delay=20

./countd 1 $delay > /tmp/countd1.log &
./countd 2 $delay > /tmp/countd2.log &
./countd 3 $delay > /tmp/countd3.log &
./countd 4 $delay > /tmp/countd4.log &
./countd 5 $delay > /tmp/countd5.log &
./countd 6 $delay > /tmp/countd6.log &
./countd 7 $delay > /tmp/countd7.log &
./countd 8 $delay > /tmp/countd8.log &
./countd 9 $delay > /tmp/countd9.log &
./countd 10 $delay > /tmp/countd10.log &
./countd 11 $delay > /tmp/countd11.log &
./countd 12 $delay > /tmp/countd12.log &
./countd 13 $delay > /tmp/countd13.log &
./countd 14 $delay > /tmp/countd14.log &

