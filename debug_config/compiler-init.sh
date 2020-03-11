function find-init() # $1: path $2: config path $3: compiler $4: extra args
{
    if [ -e "$2/init-$3.sh" ]
    then
        $2/init-$3.sh "$1" $4
    else
        default "$1" $2 $4
    fi
}
function find-config() # $1: path $2:editor $3: compiler $4 extra args
{
    if [ -e "./config/$2/$3" ]
    then
        find-init "$1" "./config/$2/$3" "$3" "$4"
    elif [ -e "./config/compiler-shared/$2-$3" ]
    then
        find-init "$1" "./config/compiler-shared/$2-$3/$3" "$3" "$4"
    elif [ -e "./config/compiler-shared/$3" ]
    then
        find-init "$1" "./config/compiler-shared/$3" "$3" "$4"
    else
        echo "Can't find config for selected compiler!"
        return 1
    fi
    return 0
}
function list-config() # $1: editor
{
    echo "Available config for $1:"
    ls "./config/$1/"
    ls "./config/compiler-shared/$1-*"
    ls "./config/compiler-shared" --ingore="*-*"
}
function read-config() # $1: path $2: editor $3: extra
{
    echo "Enter compiler: "
    read com
    find-config "$1" "$2" "$com" "$3"
    while [ $? -gt 0 ]
    do
         echo "Enter compiler"
         read com
         find-config "$1" "$2" "$com" "$3"
    done
    echo "$com" > "$1/.config/$2"
}
function init() # $1: path $2: editor $3:compiler $4: extra args
{
    if [ -z "$3" ]
    then
        list-config "$2"
        read-config "$1" "$2" "$4"
    else
        echo "$3" > "$1/.config/$2"
        find-config "$1" "$2" "$3" "$4"
    fi
    cp ./config/compiler-deinit.sh $1/.dtors/
}