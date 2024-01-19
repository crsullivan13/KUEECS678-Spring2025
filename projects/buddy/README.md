> # EECS 678 - The Buddy Allocator

## Introduction
In this assignment, you must complete the buddy allocator. Most of the
implementation has already been done for you.

<b>A full description of the buddy algorithm can be found <a
href="../../buddy-description.pdf">here</a>.</b>

## Installation
To only build the buddy allocator use:
> `$ make`

To generate this documentation in HTML use:

> `$ make doc`

To clean the project use:
> `$ make clean`

## Usage
To run the executable use:
> `$ ./buddy < test-files/test_sample1.txt`
or
> `$ ./buddy -i test-files/test_sample1.txt`

## What to Implement
#### [Allocation]

> `void* buddy_alloc (int size);`

On a memory request, the allocator returns the head of a free-list of the
matching size (i.e., smallest block that satisfies the request). If the
free-list of the matching block size is empty, then a larger block size will be
selected. The selected (large) block is then splitted into two smaller
blocks. Among the two blocks, left block will be used for allocation or be
further splitted while the right block will be added to the appropriate
free-list.

#### [Free]

> `void buddy_free(void *addr);`

Whenever a block is freed, the allocator checks its buddy. If the buddy is free
as well, then the two buddies are combined to form a bigger block. This process
continues until one of the buddies is not free.

How to find buddy and check if the buddy is free?
Suppose we have block B1 of order O, we can compute the buddy using the formula
below.

> `B2 = B1 XOR (1 << O)`
We provide a convenient macro BUDDY_ADDR() for you.

## Testing
Be sure you thoroughly test your program. We will use different test files than
the ones provided to you. We have provided a simple test case to demonstrate how
to write the test cases. We have also provided a testing script `runtests.sh`
which will run all of your test files in the test-files directory. To execute
all of your tests in that directory, simply use either:

> `$ make test`
or
> `$ ./run_tests.sh`

All test files must be located in the test-files directory and have the prefix
"test_" (i.e. test_sample2.txt). The file test_sample2.txt has the following
lines in it:

> `a = alloc(44K)` <br>
> `free(a)`

This test case allocates a 64 kilo-byte block of memory and assigns it to the
variable 'a'. If the 'K' in the size argument is removed, then this call will
only request 44 bytes. This test case then releases the block that is assigned
to 'a' with the free command. Variable names can only be one character long,
alphabetic letters.

Output must match exactly for credit. We have provided some sample output from
our implementation in the test-files directory. All files that you wish to
compare tests against should be located in the test-files directory and must
match the name of its corresponding test file with the prefix "result_" instead
of "test_". These result files should be manually created by hand. Nothing you
add to the code should print to standard output by the time you submit the
project.

## Grading

10% per working test file we provide. We have 12 test files we use for grading
so you may get up to a 120%.


Again, we have provided two test cases as examples of files we will use to grade
your project, however we encourage you to create your own to test and discover
various corner cases you may encounter.
