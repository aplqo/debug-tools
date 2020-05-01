param(
    [String]$in,
    [String]$out,
    [String]$ans
)
./bin/diff -files 2 "$out" "$ans" -diff "$in.diff" -test fc -testargs 2 "$out" "$ans"
return $?