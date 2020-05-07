if "%3"=="true" ( copy NUL "%1.cpp" )

for /l %%x in ( 1 , 1 ,%2 ) do (
    copy NUL "%1%%x.in"
    copy NUL "%1%%x.ans"
    %edit% "%1%%x.in"
    %edit% "%1%%x.ans"
)