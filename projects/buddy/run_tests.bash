#!/bin/bash

TEST_DIR=./test-files
TMP_FILE=$TEST_DIR/.tmp

TEST_PREFIX=test_
RESULT_PREFIX=result_

SUCCESSFUL_TESTS=""
FAILED_TESTS=""
UNCHECKED_TESTS=""

VERBOSE=0
VERBOSE_DIFF=0

usage() {
    printf "Usage $0 [-dv]\n" 1>&2
    printf "\td - Output diff of result and expected result on test failure\n"
    printf "\tv - Output result and expected result on test failue\n"
    exit 1
}

while getopts "dv" o; do
    case "${o}" in
        d)
            VERBOSE_DIFF=1
            ;;

        v)
            VERBOSE=1
            ;;

        *)
            usage
            ;;

    esac
done

for F in `find $TEST_DIR -type f -name test_'*' | sort`
do
    echo "-----------------------------------------------------------"
    echo "Running test file:    $F"

    ./buddy -i $F > $TMP_FILE

    RESULT_FILE=`echo $F | sed "s/$TEST_PREFIX/$RESULT_PREFIX/g"`

    echo "Expected result file: $RESULT_FILE"

    if [ -e "$RESULT_FILE" ]; then
        DIFF_OUT=`diff -w $TMP_FILE $RESULT_FILE`

        if [ "$DIFF_OUT" != "" ]; then
            echo "Output from test $F differs"
            FAILED_TESTS+=" $F"

            if [ "$VERBOSE" != "1" ] && [ "$VERBOSE_DIFF" != "1" ]; then
                echo "Please specify the -v or -d options for further details about failure. Re-run with \"$0 -h\" for usage information."
            fi

            echo ""

            if [ "$VERBOSE_DIFF" -eq "1" ]; then
                echo "*** DIFFERENCE: < Test Result; > Expected Result ***"
                echo "$DIFF_OUT"
                echo ""
            fi

            if [ "$VERBOSE" -eq "1" ]; then
                echo "*** Test output ***"
                cat $TMP_FILE
                echo "*** Expected output ***"
                cat $RESULT_FILE
                echo ""
            fi
        else
            echo "Test passed"
            SUCCESSFUL_TESTS+=" $F"
            echo ""
        fi
    else
        echo "No result file for test: $F... Skipping diff"
        UNCHECKED_TESTS+=" $F"
        echo "OUTPUT:"
        cat $TMP_FILE
        echo ""
    fi
done

rm $TMP_FILE

echo "=======================  SUMMARY  ========================="
echo "SUCCESSFUL TESTS"
for F in $SUCCESSFUL_TESTS
do
    echo $F
done

echo ""
echo "FAILED TESTS"
for F in $FAILED_TESTS
do
    echo $F
done

echo ""
echo "UNCHECKED TESTS"
for F in $UNCHECKED_TESTS
do
    echo $F
done

echo ""
