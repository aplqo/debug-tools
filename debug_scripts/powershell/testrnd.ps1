[CmdletBinding()]
param(
    [String]$tmpdir,
    [String]$genarg,
    [String]$time
)
[String]$program
[String]$generator
[String]$standard

& ./bin/random-$env:type -program $program -tmpdir "$tmpdir" -generator $generator -gen-args [ '{input}' ] -standard $standard -std-args [ '{input}' '{answer}' ] -times $time -stop-on-error  -test fc -test-args [ "{output}" "{answer}" ] 