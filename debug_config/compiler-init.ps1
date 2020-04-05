function find-init([String]$def, [String]$path, [String]$config, [String]$compiler, [String]$extra) {
    if (Test-Path -Path "$config/init-$compiler.ps1") {
        Invoke-Expression -Command "$config/init-$compiler.ps1 \"$path\" $extra"
    }
    else {
        & $def "$path" "$config" $extra.Split(' ')
    }
}
function find-config( [String]$editor, [String]$compiler) {
    switch ("$compiler") {
        { Test-Path -path "./config/$editor/$_" } { return "./config/$editor/$_"; break }
        { Test-Path -Path "./config/compiler-shared/$editor-$_" } { return "./config/compiler-shared/$editor-$_"  ; break }
        { Test-Path -Path "./config/compiler-shared/$_" } { return "./config/compiler-shared/$_"; break }
        Default { 
            Write-Error -Message "can't find config for $compiler"
            return $null
            break
        }
    }
}
function list-config([string]$editor) {
    [System.Collections.ArrayList]$list = @()
    foreach ($i in (Get-ChildItem -Path "./config/compiler-shared/*", "./config/$editor/*" -Directory)) {
        if (($i.Name -like "$editor-*") -or ($i.Name -notlike "*-*")) {
            $list.Add($i) > $null
        }
    }
    Write-Output -InputObject $list
}
function read-config([String]$f, [String]$editor) {
    [String]$com
    do {
        $com = Read-Host -Prompt "Enter compiler"
    } until(& $f $editor $com )
    return $com
}
function init([String]$def, [string]$path, [string]$editor, [String]$compiler, [String[]]$extra) {
    if (!$compiler) {
        list-config "$editor"
        $compiler = read-config -f find-config -editor "$editor"
        Out-File -InputObject $compiler -FilePath "$path/.config/cmd" -Append:$true -NoNewline
    } 
    Out-File -InputObject "$compiler" -FilePath "$path/.config/$editor"
    $c = find-config "$editor" "$compiler"
    find-init $def "$path" "$c" "$compiler" "$extra"
    Copy-Item -Path ./config/compiler-deinit.ps1 "$path/.dtors"
}