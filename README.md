# Overview

This repository contains the Symbiote library and a bunch of example applications. The Symbiote libray is built to the standard application programming model (as opposed to e.g. Linux kernel code). It currently builds as a static library which can be linked into an application.

The Symbiote library is intended to be useful to programmers working on Chrono-kernels. Applications can use thse tools to extend and rework their interaction with the kernel. We provide multiple examples where shortcutting is used to improve application performance such as throughput and latency.

# Definitions
Chrono-kernel: A kernel which allows application threads to toggle in and out of the supervisor mode of execution (e.g. ring 0, exception level 1). Applications can use this ability to extend the operating system they run on in order to improve against some criteria (e.g. performance, security, real time etc). We added the kElevate mechanism to Linux, turning it into a Chrono-kernel.

kElevate mechanism: We added a system call to Linux which allows application threads to return in the supervisor mode of execution (see Linux fork repo).
