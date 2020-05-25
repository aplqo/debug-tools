param(
    [String]$pro,
    [String]$f
)

./bin/single "$pro" -in "$f.in" -out "$f.out" -test 'powershell ./scripts/test.ps1' -testargs [ "$f.in" "$f.out" "$f.ans" ]