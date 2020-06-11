@echo off

bin\single.exe %1 -in "%2.in" -out "%2.out" -ans "$2.ans" %3 -test "scripts\test.cmd" -testargs [ "<input>" "<output>" "<answer>" ]