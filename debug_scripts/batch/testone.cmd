@echo off

if "%2"=="" set conf=.\config\default.yaml else set conf=%2

bin\single-%type%.exe %conf% %1.in %1.ans