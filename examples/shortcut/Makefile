test_sc: ../../bin/shortcut/sc_lib.so
	time (LD_LIBRARY_PATH=../../bin/shortcut LD_PRELOAD=./$< ./shortcut_test)

test_no_sc: 
	time ./shortcut_test
