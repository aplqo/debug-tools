[CmdletBinding()]
param(
    [String]$in,
    [String]$out,
    [String]$ans
)
./bin/diff -no-version -files [ "$out" "$ans" ] -diff "$in.diff" -test fc -testargs [ "$out" "$ans" ]
exit $LASTEXITCODE