all:
	git submodule update --init
	make -C libs/symlib
	make -C examples

