@echo off

bin\single-%type%.exe -program %1 -in "%2.in" -ans "$2.ans" %3 -test "C:\Windows\System32\fc.exe" -test-args [  "{output}" "{answer}" ] -autodiff [ -diff "{input}.diff" -files [ "{output}" "{answer}" ] ]