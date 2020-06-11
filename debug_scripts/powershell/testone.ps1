param(
    [String]$pro,
    [String]$f
)

./bin/single "$pro" -in "$f.in" -out "$f.out" -ans "$f.ans" -test 'powershell ./scripts/test.ps1' -testargs [ "<input>" "<output>" "<answer>" ]