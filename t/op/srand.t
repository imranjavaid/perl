#!./perl -w

# Test srand.

use strict;
use Test::More tests => 3;

# Generate a load of random numbers.
# int() avoids possible floating point error.
sub mk_rand { map int rand 10000, 1..100; }


# Check that rand() is deterministic.
srand(1138);
my @first_run  = mk_rand;

srand(1138);
my @second_run = mk_rand;

ok( eq_array(\@first_run, \@second_run),  'srand(), same arg, same rands' );


# Check that different seeds provide different random numbers
srand(31337);
@first_run  = mk_rand;

srand(1138);
@second_run = mk_rand;

ok( !eq_array(\@first_run, \@second_run),
                                 'srand(), different arg, different rands' );


# This test checks whether Perl called srand for you.
@first_run  = `$^X -le "print int rand 100 for 1..100"`;
@second_run = `$^X -le "print int rand 100 for 1..100"`;

ok( !eq_array(\@first_run, \@second_run), 'srand() called automatically');
