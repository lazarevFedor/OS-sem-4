#!/bin/bash

# Компиляция программ
gcc -o lab9_1 lab9_1.c -lcap
gcc -o lab9_2 lab9_2.c -lcap
sudo setcap cap_setfcap=ep ./lab9_2