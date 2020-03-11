function read-typ()
{
    typ=$(cat "./.config/$1")
    if [ -e "./.dtors/deinit-$typ.sh" ]
    then
        "./.dtors/deinit-$typ.sh"
        rm -f "./.dtors/deinit-$typ.sh"
    else
        default
    fi
    rm -f ./.dtors/compiler-deinit.sh
}