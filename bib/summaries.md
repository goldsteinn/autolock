## High Level

### Benchmarks

- **memcached**
    - people tend to amp up the number of threads > number of cores
- **PARSEC**
- **SPLASH-2**
- **micro**
    - custom written is fine
- **hashtable**
    - custom written is fine


### Lock Design

- Most research on NUMA aware

- [Ticket Lock](https://en.wikipedia.org/wiki/Ticket_lock)
    - Good fairness, minimal overhead
- [Queue Lock](https://en.wikipedia.org/wiki/Array_Based_Queuing_Locks)
    - Good fairness, minimal network traffic (note doesn't need to be an array)
    - People REALLY like queue locks
- Hierarchical
    - Applied to generally a queue lock type. Essentially makes the
      lock explicitly NUMA aware (i.e instead of just spinning on
      previous Queue node (any socket) it organizes multiply queues by
      socket/core/etc...)

- [Other](https://www.usenix.org/sites/default/files/conference/protected-files/atc16_slides_guiroux.pdf#page=15)

### OS

- Most research on NUMA aware

- Scheduling for better cache/socket locality between user-level processes
- Optimizing data structures in the kernel for better cache/socket locality
- Assigning VM -> PM based on socket locality


## Our Contribution

- Scheduling aware (as opposed to NUMA aware)

## Seth To Read

- [Multicore Locks: The Case Is Not Closed Yet](https://www.usenix.org/system/files/conference/atc16/atc16_paper-guiroux.pdf)
    - Think using [their benchmarks](https://github.com/multicore-locks/litl) makes sense.
    - Otherwise we can just use `LD_PRELOAD`

- [Everything you always wanted to know about synchronization but were afraid to ask](https://dl.acm.org/doi/pdf/10.1145/2517349.2522714)
    - Good general summary
    - **Note: They incorrectly describe the MESI (Modified) state**
    - Has [libslock](git@github.com:tudordavid/libslock.git) which has lock benchmarks.
    - Unable to find `ssht`.
    - Their application benchmark is memcached.
    - Ticket lock is good. Look into `prefetchw`

## Relevant

### Benchmarks

- [Multicore Locks: The Case Is Not Closed Yet](https://www.usenix.org/system/files/conference/atc16/atc16_paper-guiroux.pdf)
    - Think using [their benchmarks](https://github.com/multicore-locks/litl) makes sense.
    - Otherwise we can just use `LD_PRELOAD`

- [Brief Announcement: Benchmarking Recoverable Mutex Locks](https://dl.acm.org/doi/pdf/10.1145/3350755.3400269)
    - Explains a bit into benchmarking mutexes.
    - Idea is to vary level of contention by randomly selecting among N different mutexes and varying the value of N.
    - Emailed author about getting the source code.


- [TBBench: A Micro-Benchmark Suite for Intel Threading Building Blocks](https://www.koreascience.or.kr/article/JAKO201222340312051.pdf)
    - For [intel TBB](https://github.com/oneapi-src/oneTBB). Includes mutex benchmarks.
    - Little to no info about the exact benchmarks and couldn't find source. Emailed author.
    - [cite](https://data.doi.or.kr/cite/10.3745/JIPS.2012.8.2.331)
    - Author cant find the code. Pointer me
      [here](https://github.com/LangdalP/EPCC-OpenMP-micro-benchmarks)
      for an alternative.


- [Characterizing and Improving the Performance of Intel Threading Building Blocks](https://parsec.cs.princeton.edu/publications/contreras08tbb.pdf)
    - Exploration of overheads in [intel TBB](https://github.com/oneapi-src/oneTBB)
    - Includes real benchmarks: [PARSEC](https://parsec.cs.princeton.edu/) which is essentially SPEC for parallel applications.
    - [parse manual](https://arco.e.ac.upc.edu/wiki/images/8/8a/Seminar_Parsec3.pdf)


- [SPLASH-2](https://collaborate.princeton.edu/en/publications/splash-2-programs-characterization-and-methodological-considerati-2)
    - [source](http://liuyix.github.io/splash2_benchmark/)


- [PARSEC](https://dl.acm.org/doi/abs/10.1145/1454115.1454128)
    - [source](https://parsec.cs.princeton.edu/)

- [PARSEC vs. SPLASH-2: A Quantitative Comparison of Two Multithreaded Benchmark Suites on Chip-Multiprocessors](https://parsec.cs.princeton.edu/publications/bienia08comparison.pdf)
    - PARSEC is better (no shit it was written by the PARSEC authors)
    - SPLASH-2 has smaller memory footprint.

- [Scalable NUMA-aware Blocking Synchronization Primitives](https://www.usenix.org/system/files/conference/atc17/atc17-kashyap.pdf)
    - Really about locking in the kernel being NUMA-aware
    - [Public benchmark suite](https://github.com/sslab-gatech/cst-locks/tree/master/libslock)
    - Two benchmarks:
        1. Atomic counter (easy enough)
        2. Hash table with 1-access lock (implemented in the kernel but we can steal the idea)

- [Contention-Aware Lock Scheduling for Transactional Databases](https://www.vldb.org/pvldb/vol11/p648-tian.pdf)
    - Really about when to schedule critical sections. Not about messing with general process scheduler.
    



### OS
- [Fast Mutual Exclusion for Uniprocessors](https://dl.acm.org/doi/pdf/10.1145/143371.143523)
    - Fundemental idea that lead to linux `rseq`
    - Relevant in using the kernel to manage concurrency state
    - Fundementally about scheduling

- [Tornado: Maximizing Locality and Concurrency in a Shared Memory  Multiprocessor Operating System](https://www.usenix.org/legacy/events/osdi99/full_papers/gamsa/gamsa.pdf)
    - About designing OS for socket locality

- [Corey: An Operating System for Many Cores](https://www.usenix.org/legacy/event/osdi08/tech/full_papers/boyd-wickizer/boyd_wickizer.pdf)
    - About designing OS datastructures around core/socket locality

- [An Analysis of Linux Scalability to Many Cores](https://pdos.csail.mit.edu/papers/linux:osdi10.pdf)
    - Again appear to be concerned with memory locality/sharing in the kernel (overheads in kernel-space)

- [Scheduling support for concurrency and parallelism in the Mach operating system](https://ieeexplore.ieee.org/document/53353)
    - Abstract only, seems to be about just scheduling for better locality for the user-land processes

- [The implications of cache affinity on processor scheduling for multiprogrammed, shared memory multiprocessors](https://dl.acm.org/doi/pdf/10.1145/121132.121140)
    - Again, only related to scheduling based on userland processor
      cache locality (wonder how they feel about SPECTRE/Meltdown
      protections!)

- [Empirical Studies of Competitive Spinning for A Shared-Memory Multiprocessor](https://homes.cs.washington.edu/~karlin/papers/spinlock.ps)
    - NB: User `ps2pdf` to turn into PDF
    - Really just about when to spin vs enter `futex`

- [Scheduler Activations: Effective Kernel Support for the User-Level Management of Parallelism](https://dl.acm.org/doi/pdf/10.1145/146941.146944)
    - Thought had missed the mark by 30 years
    - They are essentially arguing for how `kernel threads` / `user threads` should be designed (they didn't win).
    - Their solution if `user thread` gets schedule interupted during a critical section that it be able to continue until CS completion.

    - **idea**: Maybe we could add a mechanism where if autolock is armed +
      current thread has the lock it will ignore the first schedule
      interrupt (we can't propose something that would deadlock the
      kernel) but this would be deadlock free (although we would lose
      any realtime gurantee). Will look into scheduler a bit. Maybe
      can find if there is some easy flag to check if a high priority
      task is ready, and skip de-schedule with short timeslice iff
      nothing important is waiting.

- [Design and Implementation of a Locking-Aware Scheduler for Multiprocessor Environments](https://link.springer.com/chapter/10.1007/978-3-642-24082-9_47)
    - See `papers/design-and-implementation-of-a-locking-aware-scheduler-for-multiprocessor-environments.pdf`
    - Somewhat similiar but not quite. It proposed a shared memory
      interface between VMs and host machine whereby the host machine
      can write scheduling info and the VM can make decisions about
      whether to take locks or not.


### Lock Design

- [Solution of a Problem in Concurrent Programming Control ](http://cs-exhibitions.uni-klu.ac.at/fileadmin/template/documents/text/p569-dijkstra.pdf)
    - Original Paper on mutexes

- **Seth See This** [Remote Core Locking: Migrating Critical-Section Execution to Improve the Performance of Multithreaded Applications](https://www.usenix.org/system/files/conference/atc12/atc12-final237.pdf)
    - Instead of locking, move the critical section to a core.
    - Note super relevant for us, but honestly related as hell to what your doing with ULI.

- [A Hierarchical CLH Queue Lock](https://www.cs.tau.ac.il/~shanir/nir-pubs-web/Papers/CLH.pdf)
    - Queue lock that promotes re-accessing the lock with another
      thread from the same socket (seems like they coulda gone
      ticket).
    - NB: `CLH` is the term for a lock built with principals from:
        - [Queue locks on cache coherent multiprocessors](https://ieeexplore.ieee.org/document/288305)
        - [The performance of spin lock alternatives for shared-memory multiprocessors](https://www.cc.gatech.edu/classes/AY2009/cs4210_fall/papers/anderson-spinlock.pdf)
        - [Building FIFO and Priority-Queuing Spin Locks from Atomic Swap](http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.38.7889&rep=rep1&type=pdf)


- [Optimal Strategies for Spinning and Blocking](https://www.sciencedirect.com/science/article/pii/S0743731584710562)
    - Just about when the spin vs enter `futex`

- [GLocks: Efficient Support for Highly-Contended Locks in Many-Core CMPs](http://www.ditec.um.es/caps/pdfs/abellan_ipdps11.pdf)
    - A hardware proposal for MPI support for locking.

- [Synchronization Without Contention](https://www.cs.rice.edu/~johnmc/papers/asplos91.pdf)
    - [Algorithms for Scalable Synchronization on Shared Memory Multiprocessor](http://web.mit.edu/6.173/www/currentsemester/readings/R06-scalable-synchronization-1991.pdf)
        - Same paper as above AFAICT
    - Queue lock on list, implemented as FIFO (for fairness)

- [The performance of spin lock alternatives for shared-memory multiprocessors](https://www.cc.gatech.edu/classes/AY2009/cs4210_fall/papers/anderson-spinlock.pdf)
    - Original Queue lock paper, non-FIFO

- [Queue locks on cache coherent multiprocessors](https://ieeexplore.ieee.org/document/288305)
    - Abstract only
    - Seems to another Queue lock with better cache/socket behavior


- [Scalable Queue-Based Spin Locks with Timeout](https://www.cs.rochester.edu/u/scott/papers/2001_PPoPP_Timeout.pdf)
    - Just what the title says.

- [Building FIFO and Priority-Queuing Spin Locks from Atomic Swap](http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.38.7889&rep=rep1&type=pdf)
    - Just lock-free queue
    - Related heavily to first Queue lock (known as CLH) from [Algorithms for Scalable Synchronization on Shared Memory Multiprocessor](http://web.mit.edu/6.173/www/currentsemester/readings/R06-scalable-synchronization-1991.pdf)




## Irrelivant
- [Automatic Mutual Exclusion](https://www.usenix.org/legacy/event/hotos07/tech/full_papers/isard/isard_html/)
    - Just about implementing transactions in programming languages
- [Adaptive and efficient abortable mutual exclusion](https://dl.acm.org/doi/pdf/10.1145/872035.872079)
    - Just about implementing abortable mutexes

- [Lock Cohorting: A General Technique for Designing NUMA Locks](https://groups.csail.mit.edu/mag/a13-dice.pdf)
    - Not really as relevant as the rest. Mostly about how to convert non-NUMA lock to NUMA lock.


- [Evaluating MapReduce for Multi-core and Multiprocessor Systems](https://web.stanford.edu/~kozyraki/publications/2007.cmp_mapreduce.hpca.pdf)
    - Just evaluation of map reduce (from NUMA perspective mostly)
    - Map reduce doesn't have enough contention for it to matter to us.

- [Performance Studies of Commercial Workloads on a Multi-core System](https://ieeexplore.ieee.org/document/4362181)
    - Only read abstract but appears to be a general measure of
      applications / how they scale, now of the synchronization
      primitives (locks)

- [Factored Operating Systems (fos): The Case for a Scalable Operating System for Multicores](https://www.princeton.edu/~wentzlaf/documents/Wentzlaff.2009.OSR.fos.pdf)
    - Message passing OS
