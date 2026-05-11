#!/bin/sh

#./md2mdoc ./test/test.md ./test/test.1 && man ./test/test.1
#./md2mdoc ./test/drist.md ./test/test.1 && man -P more ./test/test.1
#./md2mdoc ./test/drist.md | mandoc | less
#./md2mdoc ./doc/md2mdoc.md | mandoc | less

fail=0
cntr=1

for test_in in ./test/syntax/*.md; do
    ./md2mdoc $test_in > test.out
    res=$(diff -u ./test/syntax/$(basename ${test_in%.md}.mdoc) test.out)
    if [ $? -gt 0 ]; then
        printf "[%02d] FAIL : %s\n" $cntr $(basename ${test_in%.md}.mdoc)
        echo "$res"
        fail=$(( $fail + 1 ))
    else
        printf "[%02d] PASS : %s\n" $cntr $(basename ${test_in%.md})
    fi
    cntr=$(( $cntr + 1 ))
done
printf "1..%d\n" $(( $cntr - 1 ))
rm test.out

exit $fail
