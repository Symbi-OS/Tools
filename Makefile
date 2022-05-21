all:
	make -C libs/symlib clean
	make -C libs/symlib
	make -C examples clean
	make -C examples
# git submodule update --init
# make -C libs/symlib
# make -C examples

