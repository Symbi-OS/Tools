all:
	make -C libs/symlib clean
	make -C libs/symlib -j10
	make -C libs/symredislib clean
	make -C libs/symredislib -j10
	make -C examples clean
	make -C examples -j10
# git submodule update --init
# make -C libs/symlib
# make -C examples

