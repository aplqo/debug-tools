param(
    [String]$pro,
    [String]$f
)

./bin/single "$pro" -in "$f.in" -out "$f.out" -test 'powershell ./scripts/test.ps1' -testargs 3 "$f.in" "$f.out" "$f.ans"