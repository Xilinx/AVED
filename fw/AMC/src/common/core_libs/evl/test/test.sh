
DEF="-DDEBUG"
OUT="evl.o"
APP="evl_testSuite"
INC="-I../../../../common/include -I../"

/usr/bin/gcc -c ../evl.c ${DEF} ${INC} -o $OUT
/usr/bin/gcc evl_test.c $OUT ${DEF} ${INC} -o $APP

if [ -f "$APP" ]; then
    echo "$APP built successfully"
    echo "Running test suite..."
    echo "*******************************************************************************"
    ./$APP
    echo "*******************************************************************************"
    echo "...done"
    rm $OUT
    rm $APP
else
    echo "$APP not built"
    if [ -f "$OUT" ]; then
        rm $OUT
    fi
fi

