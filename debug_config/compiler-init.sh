function find-init() # $1: default $2: path $3: config path $4: compiler $5: extra args
{
    if [ -e "$3/init-$4.sh" ]
    then
        command "$4/init-$4.sh" "$2" $5
    else
        $1 "$2" $3 $5
    fi
}
function find-config() # $1:editor $2: compiler
{
    find_result=""
    if [ -e "./config/$1/$2" ]
    then
        find_result="./config/$1/$2"
    elif [ -e "./config/compiler-shared/$1-$2" ]
    then
        find_result="./config/compiler-shared/$1-$2"
    elif [ -e "./config/compiler-shared/$2" ]
    then
        find_result="./config/compiler-shared/$2"
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
function read-config() # $1: f $2: editor 
{
    compiler=""
    echo "Enter compiler: "
    read com
    $f "$2" "$com"
    while [ $? -gt 0 ]
    do
         echo "Enter compiler"
         read com
         $f "$2" "$com"
    done
}
function init() # $1: def $2: path $3: editor $4 compiler $5 extra
{
    local comp=""
    if [ -z "$4" ]
    then
        list-config "$3"
        read-config "$2" "$2" "$4"
        $comp=$compiler
        echo " $compiler" -n >> "$2/.config/cmd"
        unset compiler
    else
        comp="$4"
    fi
    echo "$comp" > "$1/.config/$3"
    find-config "$3" "$comp"
    find-init "$1" "$2" "$find_result" "$comp" "$5" 
    unset find_result
    cp ./config/compiler-deinit.sh "$2/.dtors/"
}