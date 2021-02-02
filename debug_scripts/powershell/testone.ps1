param(
    [String]$f,
    [String]$conf = ".\config\default.yaml"
)

& ./bin/single-$env:type $conf "$f.in" "$f.ans"