param(
    [String]$path,
    [String]$editor,
    [String]$compiler,
    [String]$extra
)

function list-editor() {
    Get-ChildItem -Path ./config/, ./config/editor-shared/* -Exclude "compiler-shared", "editor-shared" -Directory
}
function find-editor([String]$editor) {
    if (Test-Path -Path "./config/$editor") {
        return "./config/$editor"
    }
    elseif (Test-Path -Path "./.config/editor-shared/$editor") {
        return "./.config/editor-shared/$editor"
    }
    Write-Error -Message "Can't find config for $editor"
    return $null
}
function read-editor() {
    $ret = $null
    do {
        $ret = Read-Host -Prompt "Enter editor"
    }until(find-editor $ret)
    return ret
}

if (!$editor) {
    list-editor
    $editor = read-editor
    Out-File -InputObject "$editor" -FilePath "$path/.config/cmd" -NoNewline -Append:$true
}
Out-File -InputObject "$editor" -FilePath "$path/.config/editor"
$c = find-editor "$editor"
Invoke-Expression -Command "$c/init-$compiler.ps1 $path \"$compiler\" \"$extra\""
Copy-Item -Path ./config-deinit.ps1 "$path/.dtors"