echo "Compiling"
g++ main.cpp -o main -pthread

if [ $? -ne 0 ]; then
    echo "ошибка при компиляции"
    exit 1
fi

if [ "$#" -ne 2 ]; then
    echo "Использование $0 <IPv4> <Port>"
    exit 1
fi

echo "Start $1 $2"
./main "$1" "$2"
