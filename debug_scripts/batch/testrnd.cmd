@echo off

bin\random %1 -tmpdir "%5" -generator "%2" -genargs %3 -times %4 -stop-on-error -test "bin\diff" -testargs 14 -quiet -limit 0 -files 1 [input] -diff [differ] -test fc -testargs 2 [output] [answer]