@echo off

if "%4"=="" set inreg=in$ else set inreg="%4"
if "%5"=="" set ansreg=ans$ else set ansreg="%5"

bin\group %1 -test-regex "%3" -indir "%2" -in-regex %inreg% -ansdir %2 -ans-regex %ansreg% -test .\scripts\test.cmd -testargs [ "<input>" "<output>" "<answer>" ]
