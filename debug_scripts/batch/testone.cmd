@echo off

bin\single.exe %1 -in "%2.in" -out "%2.out" %3 -test "scripts\test.cmd" -testargs [ "%2.in" "%2.out" "%2.ans" ]