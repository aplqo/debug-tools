@echo off

bin\random-%type% -program %1 -tmpdir "%5" -generator "%2" -gen-args %3 -times %4 -stop-on-error -test "C:\Windows\System32\fc.exe" -test-args [ "{output}" "{answer}" ]