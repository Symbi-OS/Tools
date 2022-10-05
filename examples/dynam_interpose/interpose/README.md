Ok, so main intends to dynamically link with libExpected.so

We ld_preload libHijack stealing calls to libExpected. 

Also to complete the analog, libHijack depends on libHelper.
