param(
    [String]$path,
    [String]$editor,
    [String]$compiler,
    [String]$extra
)

function list-editor() {
    [System.Collections.ArrayList]$lst=@()
    foreach($i in (Get-ChildItem -Path ./config/ -Exclude "compiler-shared", "editor-shared" -Directory)){
        if(Test-Path -Path ./config/$($i.Name)/* -Include "init*.ps1"){
            $lst.Add($i) > $null
        }
    }
    foreach($i in (Get-ChildItem -Path ./config/editor-shared/ -Directory)){
        if(Test-Path -Path ./config/editor-shared/$($i.Name)/* -Include "init*.ps1"){
            $lst.Add($i) > $null
        }
    }
    Write-Output -InputObject $lst.ToArray()
}
function find-editor([String]$editor) {
    if (Test-Path -Path "./config/$editor/init-$editor.ps1") {
        return "./config/$editor"
    }
    elseif (Test-Path -Path "./config/editor-shared/$editor/init-$editor.ps1") {
        return "./config/editor-shared/$editor"
    }
    Write-Error -Message "Can't find config for $editor"
    return $null
}
function read-editor() {
    $ret = $null
    do {
        $ret = Read-Host -Prompt "Enter editor"
    }until(find-editor $ret)
    return $ret
}

if (!$editor) {
    list-editor
    $editor = read-editor
    Out-File -InputObject "$editor" -FilePath "$path/.config/cmd" -NoNewline -Append:$true
}
Out-File -InputObject "$editor" -FilePath "$path/.config/editor"
$c = find-editor "$editor"
Invoke-Expression -Command "$c/init-$editor.ps1 ""$path"" ""$compiler"" ""$extra"""

Copy-Item -Path ./config/config-deinit.ps1 "$path/.dtors"