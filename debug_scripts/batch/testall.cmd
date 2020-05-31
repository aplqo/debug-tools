@echo off

if "%3"=="" set regex="^(.*)\.[^\.]*" else set regex="%3"
if "%4"=="" set inreg=in$ else set inreg="%4"
if "%5"=="" set ansreg=ans$ else set ansreg="%5"

bin\group %1 -test-regex [ %regex% -em 1 ';' ] -indir "%2" -in-regex  [ %inreg% -mm  ';' ] -ansdir %2 -ans-regex [ %ansreg% -mm "; ] -test .\scripts\test.cmd -testargs [ "<input>" "<output>" "<answer>" ] ";"
