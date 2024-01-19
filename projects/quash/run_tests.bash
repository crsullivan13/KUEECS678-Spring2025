#!/bin/bash

if [ ! -e "$0" ]; then
    echo "This script must be run from its directory"
    exit 1
fi

export TOP_DIR=$PWD
export TEST_DIR=$PWD/test-cases
export SANDBOX_DIR=$PWD/sandbox
export QUASH=$TOP_DIR/quash
export SETUP_DIR=$PWD/test-cases/test-setup

PATH=$PATH:$SETUP_DIR

VALGRIND_CMP_FILE=$TOP_DIR/.valgrind_expected.txt
RESULT_FILE=$TOP_DIR/results.txt

PARALLEL=0
VERBOSE=0
VERBOSE_DIFF=0
TEST_UNCHECKED=0
KEEP_TMP=0
CLEAN_ALL=0
KEEP_SANDBOX=0

usage() {
    printf "Usage: $0 [-cdstuv]\n" 1>&2
    printf "\tc - Clean up all files created during testing\n" 1>&2
    printf "\td - Output differences as they occur\n" 1>&2
    printf "\tp - Allow tests to run in parallel as background jobs"
    printf "\ts - Keep sandbox directory (not valid if \"c\" is specified)\n" 1>&2
    printf "\tt - Leave temporary files intact (not valid if \"c\" is specified)\n" 1>&2
    printf "\tu - Test uncheckable files\n" 1>&2
    printf "\tv - Output raw outputs\n" 1>&2
    exit 1
}

# Removes the extension from a file name
# RETURN: File name without the extension
strip_extension() {
    # $1 - File name to remove the extension from

    echo "$1" | sed 's/\.[^\/]*$//'
}

# Removes the path from a file name
# RETURN: File name without the path
strip_path() {
    # $1 - File name to remove the path from

    echo "$1" | sed 's/[^\/]*\///g'
}

generate_expectation_file() {
    local __exp_file="$1"

    local gen_file="$(strip_extension $__exp_file).gen"

    if [ -e "$gen_file" ]; then
        printf "" > $__exp_file
        while IFS='' read -r line || [[ -n "$line" ]]
        do
            echo "$(eval echo "$line")" >> $__exp_file
        done < $gen_file
    fi
}

note_result() {
    local __noted_script="$1"
    local __result_status="$2"

    flock -x $RESULT_FILE -c "echo $__noted_script $__result_status >> $RESULT_FILE"
}

# Run a script in the test-cases/test-setup directory
run_test_setup_cmd() {
    # $1 - Script to run in the test-cases/test-setup directory

    local __setup_cmd="$1"

    local prev_pwd=$PWD

    cd $SETUP_DIR

    eval $__setup_cmd

    cd $prev_pwd
}

# Run a script in the test-cases/test-setup directory
run_test_setup_script() {
    # $1 - Script to run in the test-cases/test-setup directory

    local __setup_script="$1"

    run_test_setup_cmd 'if [ -e '"$__setup_script"' ]; then bash '"$__setup_script"'; fi'
}

# Check if output is different
output_diff() {
    # $1 - Script name
    # $2 - Output file
    # $3 - Expected output
    # $4 - Difference file
    # $5 - RETURN: Match success status (1 for match; 0 otherwise)

    local __script_name="$1"
    local __out_file="$2"
    local __out_expected="$3"
    local __out_diff="$4"
    local __out_success="$5"

    # Check output against expected output
    diff -w $__out_file $__out_expected &> $__out_diff

    # Check the exit value of diff
    if [ "$?" == "1" ]; then
        # Determine output message
        if [ "$VERBOSE" == "1" ]; then
            echo "$__script_name: Output report"
            cat $__out_file
        elif [ "$VERBOSE_DIFF" == "1" ]; then
            echo "$__script_name: Output difference"
            cat $__out_diff
        else
            echo "$__script_name: Output differs. See $__out_diff for difference"
        fi

        # Note failure status
        note_result "$__script_name" "OUT_FAILED_TESTS"
        eval $__out_success="0"
    else
        # Note success status
        note_result "$__script_name" "OUT_SUCCESSFUL_TESTS"
        eval $__out_success="1"
    fi
}

# Check for lines that are different than the expected file
valgrind_diff() {
    # $1 - Name of the script
    # $2 - Valgrind output
    # $3 - Valgrind accepted lines
    # $4 - Diff output
    # $5 - Temporary directory
    # $6 - RETURN: Match success status (1 for match; 0 otherwise)

    local __script_name="$1"
    local __valgrind_output="$2"
    local __valgrind_expected="$3"
    local __diff_out="$4"
    local __tmp_dir="$5"
    local __val_success="$6"

    local valgrind_tmp="$__tmp_dir"/valgrind_diff_cp.txt
    local valgrind_tmp2="$__tmp_dir"/valgrind_diff_final.txt

    # Make a copy of the ideal output into a temporary file
    cp $__valgrind_expected $valgrind_tmp

    # Extract a comparable report into the copied file
    tail -n +5 "$__valgrind_output" |
        sed 's/[1-9][0-9,]*/#/g' |
        sort |
        uniq >> $valgrind_tmp

    # Sort and remove non-unique lines
    sort $valgrind_tmp | uniq > $valgrind_tmp2

    # Do comparison
    diff -w $valgrind_tmp2 "$__valgrind_expected" &> "$__diff_out"

    # Check the return of valgrind_diff
    if [ "$?" == "1" ]; then
        # Determine output message
        if [ "$VERBOSE" == "1" ]; then
            echo ""
            echo "$__script_name: Valgrind report"
            cat $__valgrind_output
        elif [ "$VERBOSE_DIFF" == "1" ]; then
            echo "$__script_name: Valgrind difference"
            cat $__diff_out
        else
            echo "$__script_name: Valgrind found a problem. Report available at " \
                 "\"$__valgrind_output\"."
        fi

        # Note failure status
        note_result "$__script_name" "VALGRIND_FAILED_TESTS"
        eval $__val_success="0"
    else
        # Note success status
        note_result "$__script_name" "VALGRIND_SUCCESSFUL_TESTS"
        eval $__val_success="1"
    fi
}

## Setup testing directory
setup_sandbox() {
    local lorem_ipsum=$SANDBOX_DIR/lorem_ipsum.txt

    rm -rf $SANDBOX_DIR

    mkdir -p $SANDBOX_DIR
    mkdir -p $SANDBOX_DIR/dir1
    mkdir -p $SANDBOX_DIR/dir2
    mkdir -p $SANDBOX_DIR/dir3
    mkdir -p $SANDBOX_DIR/dir3/dir3-1
    mkdir -p $SANDBOX_DIR/dir3/dir3-2

    echo 'Lorem ipsum dolor sit amet, consectetur adipiscing elit. In eget rhoncus
lacus. Quisque tincidunt, tellus non lacinia sodales, orci lorem egestas
libero, sed mattis sem quam in mauris. Praesent varius posuere justo ac
lacinia. Nunc ut turpis ante. Cras ac commodo tellus. Fusce consectetur
interdum nulla nec vulputate. Integer vestibulum maximus magna in euismod.
Phasellus tincidunt ante cursus nulla elementum, et luctus arcu vulputate.' \
         > $lorem_ipsum

    cp $lorem_ipsum $SANDBOX_DIR/dir1
    cp $lorem_ipsum $SANDBOX_DIR/dir3/dir3-1

    echo "TEST FILE 1" > $SANDBOX_DIR/dir2/test1.txt
    echo "TEST FILE 2" > $SANDBOX_DIR/dir2/test2.txt
    echo "TEST FILE 3" > $SANDBOX_DIR/dir2/test3.txt

    cp $VALGRIND_CMP_FILE $SANDBOX_DIR/.valgrind_expected.txt
    cp $VALGRIND_CMP_FILE $SANDBOX_DIR/valgrind_expected.txt
    cp $VALGRIND_CMP_FILE $SANDBOX_DIR/dir1/valgrind_expected.txt
    cp $VALGRIND_CMP_FILE $SANDBOX_DIR/dir3/valgrind_expected.txt
    cp $VALGRIND_CMP_FILE $SANDBOX_DIR/dir3/dir3-1/valgrind_expected.txt
}

# Run a test for correctness on the quash program
run_test_checked() {
    # $1 - Script to test
    # $2 - Expected results of test
    # $3 - Accepted lines from Valgrind
    # $4 - Test results directory

    local __script="$1"
    local __script_expected="$2"
    local __valgrind_expected="$3"
    local __res_dir="$4"

    local script_name="$(strip_extension $(strip_path $__script))"
    local tmp_dir="$__res_dir"/.tmp
    local test_valgrind_cmp_file=$tmp_dir/valgrind_cmp.txt
    local valgrind_diff_out_file=$tmp_dir/vgdiff.txt
    local test_stdout_file="$__res_dir"/stdout.txt
    local test_stderr_file="$__res_dir"/stderr.txt
    local test_stdout_diff="$__res_dir"/stdout-diff.txt
    local test_valgrind_file="$__res_dir"/valgrind.txt

    local out_success=1
    local val_success=1

    # Make standard output file available to test_setup files
    export OUTPUT=$test_stdout_file

    # Create output directories
    mkdir -p "$__res_dir"
    mkdir -p $tmp_dir

    # Create temporary directory structure to run tests in
    export SANDBOX_DIR="$__res_dir"/sandbox

    # Generate sandbox
    setup_sandbox
    cd $SANDBOX_DIR

    generate_expectation_file "$__script_expected"

    echo "Running test: $script_name"

    # Run setup script for this test
    run_test_setup_script $script_name.set

    # Run the test
    valgrind --leak-check=full --track-origins=yes $QUASH \
             < "$__script"                                \
             > $OUTPUT                                    \
             2> $test_stderr_file

    # Run cleanup script to alter variable outputs and cleanup files created by
    # the setup script
    run_test_setup_script $script_name.cln

    # Extract Valgrind report
    grep "^==[0-9]*==" $test_stderr_file > $test_valgrind_file

    # Check diff of output
    output_diff $script_name $test_stdout_file $__script_expected \
                $test_stdout_diff out_success

    # Check if there are any bad lines in the output
    valgrind_diff $script_name $test_valgrind_file $__valgrind_expected \
                  $valgrind_diff_out_file $tmp_dir val_success

    # Note overall status
    if [ "$out_success" == "1" ] && [ "$val_success" == "1" ]; then
        note_result "$script_name" "SUCCESSFUL_TESTS"
    else
        note_result "$script_name" "FAILED_TESTS"
    fi

    # Clean up directories
    if [ "$CLEAN_ALL" == "1" ]; then
        if [ -e "$(strip_extension $__script_expected).gen" ]; then
            rm -f $__script_expected
        fi

        rm -rf $__res_dir
    else
        if [ "$KEEP_TMP" == "0" ]; then
            rm -rf $tmp_dir
        fi

        if [ "$KEEP_SANDBOX" == "0" ]; then
            rm -rf $SANDBOX_DIR
        fi
    fi

    cd $TOP_DIR
}

# Run a tests on the quash program
run_test_unchecked() {
    # $1 - Script to test
    # $2 - Accepted lines from Valgrind
    # $3 - Test results directory

    local __script="$1"
    local __valgrind_expected="$2"
    local __res_dir="$3"

    local script_name="$(strip_extension $(strip_path $__script))"
    local tmp_dir="$__res_dir"/.tmp
    local test_valgrind_cmp_file=$tmp_dir/valgrind_cmp.txt
    local valgrind_diff_out_file=$tmp_dir/vgdiff.txt
    local test_stdout_file="$__res_dir"/stdout.txt
    local test_stderr_file="$__res_dir"/stderr.txt
    local test_valgrind_file="$__res_dir"/valgrind.txt

    local val_success=1

    # Make standard output file available to test_setup files
    export OUTPUT=$test_stdout_file

    # Create temporary directory structure to run tests in
    export SANDBOX_DIR="$__res_dir"/sandbox

    # Create output directories
    mkdir -p "$__res_dir"
    mkdir -p $tmp_dir

    # Create temporary directory structure to run tests in
    export SANDBOX_DIR="$__res_dir"/sandbox

    # Generate sandbox
    setup_sandbox
    cd $SANDBOX_DIR

    echo "Running test: $script_name"

    # Run setup script for this test
    run_test_setup_script $script_name.set

    # Run the test
    valgrind --leak-check=full --track-origins=yes $QUASH \
             < "$__script"                                \
             > $test_stdout_file                          \
             2> $test_stderr_file

    # Run cleanup script to alter variable outputs and cleanup files created by
    # the setup script
    run_test_setup_script $script_name.cln

    # Extract Valgrind report
    grep "^==[0-9]*==" $test_stderr_file > $test_valgrind_file

    if [ "$VERBOSE" == "1" ]; then
        echo "$script_name: Output"
        cat $test_std_out_file
    else
        echo "$script_name: Output unchecked. See $test_stdout_diff for output"
    fi

    note_result "$script_name" "UNCHECKED_TESTS"

    # Check if there are any bad lines in the output
    valgrind_diff $script_name $test_valgrind_file $__valgrind_expected \
                  $valgrind_diff_out_file $tmp_dir val_success

    note_result "$script_name" "FAILED_TESTS"

    # Clean up directories
    if [ "$CLEAN_ALL" == "1" ]; then
        rm -rf $__res_dir
    else
        if [ "$KEEP_TMP" == "0" ]; then
            rm -rf $tmp_dir
        fi

        if [ "$KEEP_SANDBOX" == "0" ]; then
            rm -rf $SANDBOX_DIR
        fi
    fi

    cd $TOP_DIR
}

## Parse options
while getopts "cdpstuv" o; do
    case "${o}" in
        c)
            CLEAN_ALL=1
            ;;

        d)
            VERBOSE_DIFF=1
            ;;

        p)
            PARALLEL=1
            ;;

        s)
            KEEP_SANDBOX=1
            ;;

        t)
            KEEP_TMP=1
            ;;

        u)
            TEST_UNCHECKED=1
            ;;

        v)
            VERBOSE=1
            ;;

        *)
            usage
            ;;
    esac
done

make

rm -f $RESULT_FILE

run_test_setup_cmd make

## Run through all of the test files
for F in `find $TEST_DIR -name *.qsh -type f | sort`
do
    FRONT="$(strip_extension $F)"

    rm -rf $FRONT

    if [ -e "$FRONT.exp" ] || [ -e "$FRONT.gen" ]; then
        if [ "$PARALLEL" == "1" ]; then
            run_test_checked $F $FRONT.exp $VALGRIND_CMP_FILE $FRONT &
        else
            run_test_checked $F $FRONT.exp $VALGRIND_CMP_FILE $FRONT
        fi
    elif [ "$TEST_UNCHECKED" == "1" ]; then
        if [ "$PARALLEL" == "1" ]; then
            run_test_unchecked $F $VALGRIND_CMP_FILE $FRONT &
        else
            run_test_unchecked $F $VALGRIND_CMP_FILE $FRONT
        fi
    else
        echo "Skipping uncheckable test: $F. Re-run with the \"-u\" option to run unchecked tests."
        note_result "$(strip_path $FRONT)" "UNCHECKED_TESTS"
    fi
done

wait $(jobs -p)

## Output test summary
# Output successful tests
SUCCESSFUL_TESTS=`awk '/ SUCCESSFUL_TESTS/ { print $1 }' $RESULT_FILE | sort`
if [ "$SUCCESSFUL_TESTS" != "" ]; then
    echo ""
    echo "---------------------------------------------------------"
    echo "SUCCESSFUL TESTS:"
    echo "$SUCCESSFUL_TESTS"
fi

# Output failed tests
FAILED_TESTS=`awk '/ FAILED_TESTS/ { print $1 }' $RESULT_FILE | sort`
if [ "$FAILED_TESTS" != "" ]; then
    echo ""
    echo "---------------------------------------------------------"
    echo "FAILED TESTS:"
    echo "$FAILED_TESTS"

    OUT_FAILED_TESTS=`awk '/ OUT_FAILED_TESTS/ { print $1 }' $RESULT_FILE | sort`
    if [ "$OUT_FAILED_TESTS" != "" ]; then
        echo ""
        echo "The following scripts had non-whitespace differences from their expected output:"
        echo "$OUT_FAILED_TESTS"
    fi

    VALGRIND_FAILED_TESTS=`awk '/ VALGRIND_FAILED_TESTS/ { print $1 }' $RESULT_FILE | sort`
    if [ "$VALGRIND_FAILED_TESTS" != "" ]; then
        echo ""
        echo "The following scripts had unacceptable Valgrind errors:"
        echo "$VALGRIND_FAILED_TESTS"
    fi
fi

# Output unchecked tests
UNCHECKED_TESTS=`awk '/ UNCHECKED_TESTS/ { print $1 }' $RESULT_FILE | sort`
if [ "$UNCHECKED_TESTS" != "" ]; then
    echo ""
    echo "---------------------------------------------------------"
    echo "UNCHECKED TESTS"
    echo "$UNCHECKED_TESTS"
fi

echo ""

if [ "$CLEAN_ALL" == "1" ]; then
    run_test_setup_cmd 'make clean'
    rm -f $RESULT_FILE
fi
