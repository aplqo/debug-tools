function deinit() # $1 default $2: editor
{
    typ=$(cat "./.config/$2")
    if [ -e "./.dtors/deinit-$typ.sh" ]
    then
        "./.dtors/deinit-$typ.sh"
        rm -f "./.dtors/deinit-$typ.sh"
    else
        $1
    fi
    rm -f ./.dtors/compiler-deinit.sh
}