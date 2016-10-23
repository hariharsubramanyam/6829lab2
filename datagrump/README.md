# Introduction
This is my submission for MIT's 6.829 (Computer Networks) second problem set: Congestion-Control Contest.

# Report
You can see my detailed report in [`Report.pdf`](https://github.com/hariharsubramanyam/6829lab2/raw/master/datagrump/Report.pdf). The report also links to my plots (on Tableau Public) and my raw data (on Dropbox).

# Code
I have separate code files for Exercises A, B, C, and D (and an additional set of files for my extension to Exercise D). Each exercise (and the extension to Exercise D) has a corresponding `sender_*.cc`, `controller_*.hh`, and `controller_*.cc`. To make the files for a chosen exercise replace the existing `sender.cc`, `controller.cc`, and `controller.cc` files, I have written a shell script called [`part.sh`](https://github.com/hariharsubramanyam/6829lab2/blob/master/datagrump/part.sh).

## Warmup Exercise A
My Warmup Exercise A code is in
* [`sender_fixed_window.cc`](https://github.com/hariharsubramanyam/6829lab2/blob/master/datagrump/sender_fixed_window.cc)
* [`controller_fixed_window.cc`](https://github.com/hariharsubramanyam/6829lab2/blob/master/datagrump/controller_fixed_window.cc)
* [`controller_fixed_window.hh`](https://github.com/hariharsubramanyam/6829lab2/blob/master/datagrump/controller_fixed_window.hh)

You can make the above files replace the existing `sender.cc`, `controller.cc`, and `controller.cc` by running `./part.sh a` (make sure that `./part.sh` is executable).

## Warmup Exercise B
My Warmup Exercise B code is in
* [`sender_aimd.cc`](https://github.com/hariharsubramanyam/6829lab2/blob/master/datagrump/sender_aimd.cc)
* [`controller_aimd.cc`](https://github.com/hariharsubramanyam/6829lab2/blob/master/datagrump/controller_aimd.cc)
* [`controller_aimd.hh`](https://github.com/hariharsubramanyam/6829lab2/blob/master/datagrump/controller_aimd.hh)

Run `./part.sh b`

## Warmup Exercise C
My Warmup Exercise C code is in
* [`sender_delay.cc`](https://github.com/hariharsubramanyam/6829lab2/blob/master/datagrump/sender_delay.cc)
* [`controller_delay.cc`](https://github.com/hariharsubramanyam/6829lab2/blob/master/datagrump/controller_delay.cc)
* [`controller_delay.hh`](https://github.com/hariharsubramanyam/6829lab2/blob/master/datagrump/controller_delay.hh)

Run `./part.sh c`

## Exercise D
My Exercise D code is in
* [`sender_custom.cc`](https://github.com/hariharsubramanyam/6829lab2/blob/master/datagrump/sender_custom.cc)
* [`controller_custom.cc`](https://github.com/hariharsubramanyam/6829lab2/blob/master/datagrump/controller_custom.cc)
* [`controller_custom.hh`](https://github.com/hariharsubramanyam/6829lab2/blob/master/datagrump/controller_custom.hh)

Run `./part.sh d`

## Exercise D': Reinforcement Learning (did not work, but included for reference)
I tried another approach to solve Exercise D (I call my extension Exercise D', or "D prime"). While it did not perform as well as my solution to Exercise D, I have included it in this code because the reader may find it interesting. Its code is in
* [`sender_sarsa.cc`](https://github.com/hariharsubramanyam/6829lab2/blob/master/datagrump/sender_sarsa.cc)
* [`controller_sarsa.cc`](https://github.com/hariharsubramanyam/6829lab2/blob/master/datagrump/controller_sarsa.cc)
* [`controller_sarsa.hh`](https://github.com/hariharsubramanyam/6829lab2/blob/master/datagrump/controller_sarsa.hh)

Run `./part.sh dprime`
