#! /bin/sh
# $Id$
# Test scripts for hdp (dumper)


# Definitions of commands and variables
CMD='./hdp'
RM='rm -f'
DIFF=diff
errors=0


# Definitions of functions/shorthands
#

# Print message with formats according to message level ($1)
MESG()
{
    level=$1
    shift
    case $level in
	0)
	    echo '#############################'
	    echo $*
	    echo '#############################'
	    ;;
	3)
	    echo '============================='
	    echo $*
	    echo '============================='
	    ;;
	6)
	    echo "*** $* ***"
	    ;;
	*)
	    echo "MESG(): Unknown level ($level)"
	    exit 1
	    ;;
    esac

}


# Run the test to produce an output file which is then
# compared with the expected ($1) output.
# Note that this can be used to produce the expected
# output files by replace "$output" with "$expected"
# in the run-the-test commands.
TEST()
{
    # parse the arguments
    output=tmp.out
    expected=testfiles/$1
    shift
    # print a id banner
    MESG 6 $@
    # run the test
    MESG 0 "Expected output for '$CMD $@'" > $output
    $CMD "$@" >> $output
    if ($DIFF $expected $output)
    then
	# no action
	true
    else
	echo "   <<< FAILED >>>"
	errors=`expr $errors + 1`
    fi
}


# Test command list
MESG 3 Test command list
TEST list-1.out list tdata.hdf
TEST list-2.out list -l tdata.hdf
TEST list-3.out list -d tdata.hdf
TEST list-4.out list -e tdata.hdf
TEST list-5.out list -t 720 tdata.hdf
TEST list-6.out list -d -t "Numeric Data Group" tdata.hdf
TEST list-7.out list -g tdata.hdf
TEST list-8.out list -a tdata.hdf
TEST list-9.out list -a Example6.hdf
TEST list-10.out list -n Example6.hdf

# Test command dumpsds
MESG 3 Test command dumpsds
TEST dumpsds-1.out dumpsds swf32.hdf
TEST dumpsds-2.out dumpsds -i 2 swf32.hdf
TEST dumpsds-3.out dumpsds -i 1,3 swf32.hdf
# Next command should fail with error message: "SD with name Time: not found"
TEST dumpsds-4.out dumpsds -n Time swf32.hdf
TEST dumpsds-5.out dumpsds -n fakeDim0,Data-Set-2 swf32.hdf
TEST dumpsds-6.out dumpsds -r 3,2 swf32.hdf
TEST dumpsds-7.out dumpsds -r 3,2 -d swf32.hdf

# Test command dumprig
MESG 3 Test command dumprig
TEST dumprig-1.out dumprig tdf24.hdf
TEST dumprig-2.out dumprig -i 1,2 tdf24.hdf
TEST dumprig-3.out dumprig -i 1,3 tdf24.hdf		# '-i 3' is invalid
TEST dumprig-4.out dumprig -m 24 tdf24.hdf
TEST dumprig-5.out dumprig -r 3,4 tdf24.hdf
TEST dumprig-6.out dumprig -r 3,4 -d tdf24.hdf

# Test command dumpvd
MESG 3 Test command dumpvd
TEST dumpvd-1.out dumpvd tvset.hdf
TEST dumpvd-2.out dumpvd -i 1,3,5 tvset.hdf
TEST dumpvd-3.out dumpvd -r 1238,1239,1251,1252 tvset.hdf
TEST dumpvd-4.out dumpvd -n "Multi-Order Vdata" tvset.hdf
TEST dumpvd-5.out dumpvd -n "Mixed Vdata","Integer Vdata" tvset.hdf
TEST dumpvd-6.out dumpvd -c "Test object","No class specified" tvset.hdf
TEST dumpvd-7.out dumpvd -f B tvset.hdf
TEST dumpvd-8.out dumpvd -f "STATION_NAME","FLOATS" tvset.hdf
TEST dumpvd-9.out dumpvd -f "STATION_NAME","FLOATS" -d tvset.hdf
TEST dumpvd-10.out dumpvd tvattr.hdf

# Test command dumpvg
MESG 3 Test command dumpvg
TEST dumpvg-1.out dumpvg tvset.hdf
TEST dumpvg-2.out dumpvg -i 0,1 tvset.hdf
TEST dumpvg-3.out dumpvg -r 3 tvset.hdf
TEST dumpvg-4.out dumpvg -n "Simple Vgroup" tvset.hdf
TEST dumpvg-5.out dumpvg -c "Test object" tvset.hdf
TEST dumpvg-6.out dumpvg -i 1,3,5 tdata.hdf
TEST dumpvg-7.out dumpvg -r 32,39 tdata.hdf
TEST dumpvg-8.out dumpvg -n nsamp,tdata.hdf tdata.hdf
TEST dumpvg-9.out dumpvg -c CDF0.0 tdata.hdf
TEST dumpvg-10.out dumpvg -c Dim0.0,Var0.0 tdata.hdf
TEST dumpvg-11.out dumpvg -c Dim0.0,Var0.0 -d tdata.hdf
TEST dumpvg-12.out dumpvg tvattr.hdf

# Test command dumpgr
MESG 3 Test command dumpgr
TEST dumpgr-1.out dumpgr grtdfui82.hdf
TEST dumpgr-2.out dumpgr -i 0,1,3 grtdfui82.hdf
TEST dumpgr-3.out dumpgr -i 0 grtdfui82.hdf
TEST dumpgr-4.out dumpgr -n Image_array_5 grtdfui82.hdf
TEST dumpgr-5.out dumpgr -r 6,2,3 grtdfui82.hdf
TEST dumpgr-6.out dumpgr -r 6 -d  grtdfui82.hdf
TEST dumpgr-7.out dumpgr -o my.dat grtdfui82.hdf
$RM my.dat
TEST dumpgr-8.out dumpgr -o mybin.dat  -b grtdfui82.hdf
$RM mybin.dat
TEST dumpgr-9.out dumpgr grtdfui83.hdf
TEST dumpgr-10.out dumpgr grtdfui84.hdf
TEST dumpgr-11.out dumpgr grtdfui162.hdf
TEST dumpgr-12.out dumpgr grtdfi322.hdf

# Result
if [ $errors -eq 0 ]
then
    MESG 0 "All tests passed"
else
    MESG 0 "Tests failed: $errors"
fi
exit $errors
