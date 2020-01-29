@echo off

inreg="in$"
ansreg="ans$"

if not %4 == "" inreg=%4
if not %5 == "" ansreg=%5

bin\group %1 -test-regex "%3" -indir "%2" -in-regex "%inreg" -ansdir "%2" -ans-regex "%ansreg" -test '.\scripts\test.cmd' -testargs 3 [input] [output] [answer]
