# Overview

This repository contains a bunch of symbiote tools and example applications. All programs are built to the standard application programming model (as opposed to e.g. Linux kernel code). We are moving to dynamically linked executables. We are in the process of depricating static builds.

These tools and examples are intended to use the Symbiote library which can be found in the symlib repo of this organization. Applications can use thse tools to extend and rework their interaction with the kernel. We provide multiple examples where shortcutting is used to improve application performance such as throughput and latency.

# Definitions
Chrono-kernel: A kernel which allows application threads to toggle in and out of the supervisor mode of execution (e.g. ring 0, exception level 1). Applications can use this ability to extend the operating system they run on in order to improve against some criteria (e.g. performance, security, real time etc). We added the kElevate mechanism to Linux, turning it into a Chrono-kernel.

kElevate mechanism: We added a system call to Linux which allows application threads to return in the supervisor mode of execution (see Linux fork repo).

# Building
By default the executables will get linked with the dynamic version of the symbiote library, but you can also link against a static libsym.a library by passing in ```TYPE=static``` into the make call.

Example: ```make TYPE=static```
