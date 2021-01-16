param(
    [String]$pro,
    [String]$f
)

& ./bin/single-$env:type -program "$pro" -in "$f.in" -ans "$f.ans" -test 'fc.exe' -test-args [ "{output}" "{answer}" ] -autodiff [ -diff '{input}.diff' -files [ '{output}' '{answer}' ] ]