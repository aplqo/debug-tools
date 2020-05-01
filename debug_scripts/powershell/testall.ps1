param(
    [String]$pro,
    [String]$data,
    [String]$regex = "^",
    [String]$inreg = '\.in$',
    [String]$ansreg = '\.ans$'
)

./bin/group "$pro" -test-regex "$regex" -indir "$data" -in-regex "$inreg" -ansdir "$data" -ans-regex "$ansreg" -test .\scripts\test.cmd -testargs 3 [input] [output] [answer]