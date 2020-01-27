@echo off
rem $1: input $2 output $3 answer

bin\diff -files 2 "%2" "%3" -diff "%1.diff" -test diff -testargs 6 "%2" "%3" -a -w --color -u