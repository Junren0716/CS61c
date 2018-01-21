echo
echo '          *** CS 61C, Summer 2015: Project 4 ***'
echo
echo 'If your tests fail, you can find the output from each test run in a file'
echo 'test/out/<N>.txt, and the expected output in test/ref/<N>.txt. Each of'
echo 'these files contains the values produced by every layer. If something'
echo 'goes wrong, check your implementation of the layer right before the bug'
echo 'manifested itself.'
echo

FINAL_OUTPUT="ALL TESTS PASSED"

for i in {1..20}; do
  echo -n "RUNNING TEST $i... "
  ../cnn test $i 2>/dev/null | grep LAYER > out/$i.txt
  python compare_layers.py out/$i.txt ref/$i.txt

  if [ "$?" -ne 0 ]; then
    FINAL_OUTPUT='SOME TESTS FAILED -- SEE ERROR MESSAGES FOR DETAILS!'
  fi
done

for i in 100 400 600 1200; do
    echo -n "PARALLEL TEST $i... "
    ../cnn partest $i 2>/dev/null | grep PAR > out/par$i.txt
    python compare_output.py out/par$i.txt ref/par$i.txt

    if [ "$?" -ne 0 ]; then
        FINAL_OUTPUT='SOME PARALLEL TESTS FAILED -- SEE ERROR MESSAGES FOR DETAILS!'
    fi
done

echo
echo "$FINAL_OUTPUT"
echo
