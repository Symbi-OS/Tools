all: main

# This is what the programmer intened to do.
# Main dynamically links with expected.
# =================================
run_main:
	LD_LIBRARY_PATH=$(PWD) ./main
# debug version
# LD_LIBRARY_PATH=$(PWD) LD_DEBUG=libs ./main

main: libExpected.so
	gcc main.c -L./ -lExpected -o $@

libExpected.so: expected.c
	gcc -shared -fPIC -o $@ $<
# =================================



# This is our trickery. A hijacking lib takes over the call
# to expected()
# =================================
run_hijack: main libHijack.so libHelper.so
	LD_LIBRARY_PATH=$(PWD) LD_PRELOAD=./libHijack.so ./main

libHijack.so: hijack.c libHelper.so
	gcc -shared -fPIC -o $@ -L./ -lHelper $<

libHelper.so: helper.c
	gcc -shared -fPIC -o $@ $<
# =================================

run_hijack_env: main libHijack.so libHelper.so
	MY_ENV_VAR=SC_READ LD_LIBRARY_PATH=$(PWD) LD_PRELOAD=./libHijack.so ./main

clean:
	- rm -rf main *.so
