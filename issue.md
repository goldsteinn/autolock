The issue is that when a task skips its timeslice because its
"auto-lock" memory is locked, it will continue accumulating desire to
run, so when it does finally get scheduled (and for the next few
times), it will get extremely long time-slices.

For ticket/queue locks this leads to a bad priority-inversion (hence
the regressions we see). For spinlocks it works great because we end
up with less context switches.
