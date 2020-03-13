function find-init([System.Action]$def, [String]$path, [String]$config, [String]$compiler, [String]$extra) {
    if (Test-Path -Path "$config/init-$compiler.ps1") {
        Invoke-Expression -Command "$config/init-$compiler.ps1 \"$path\" $extra"
    }
    else {
        $def.Invoke("$path", "config", $extra.Split(' '))
    }
}
function find-config( [String]$editor, [String]$compiler) {
    switch ($compiler) {
        { Test-Path -Path "./config/$editor/$_" } { return "./config/$editor/$_" }
        { Test-Path -Path "./config/compiler-shared/$editor-$_" } { return "./config/compiler-shared/$editor-$_" }
        { Test-Path -Path "./config/compiler-shared/$_" } { return "./config/compiler-shared/$_" }
        Default { 
            Write-Error -Message "can't find config for $compiler"
            return $null
        }
    }
}
function list-config([string]$editor) {
    [System.IO.DirectoryInfo[]]$list = $null
    foreach ($i in (Get-ChildItem -Path "./config/compiler-shared/*", "./config/$editor/*" -Directory)) {
        if (($i.Name -like "$editor-*") -or ($i.Name -notlike "*-*")) {
            $list.Add($i)
        }
    }
    Write-Output -InputObject $list
}
function read-config([System.Action]$f, [String]$editor) {
    $com = $null
    do {
        $com = Read-Host -Prompt "Enter compiler"
    } until (!$f.Invoke($editor, $com))
    return $com
}
function init([System.Action]$def, [string]$path, [string]$editor, [String]$compiler, [String[]]$extra) {
    if (!$compiler) {
        list-config $editor
        $compiler = read-config $Function:find-config $editor
        Out-File -InputObject $compiler -FilePath "$path/.config/cmd" -Append:$true -NoNewline
    } 
    Out-File -InputObject "$compiler" -FilePath "$path/.config/$editor"
    $c = find-config "$editor" "$compiler"
    find-init $def "$path" "$c" "$compiler" "$extra"
    Copy-Item -Path ./config/compiler-deinit.ps1 "$path/.dtors"
}