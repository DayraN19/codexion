*This project has been created as part of the 42 curriculum by bgranier.*

# Codexion

**Master the race for resources before the deadline masters you**

## Description

Codexion is a concurrent programming project focused on resource sharing, scheduling policies, and thread synchronization using POSIX threads (`pthreads`).

The project simulates a team of coders competing for a limited number of shared resources called **dongles**. In order to compile successfully, a coder must acquire two dongles. Each coder repeatedly goes through the following cycle:

1. Compile
2. Debug
3. Refactor

If a coder does not start compiling again before its burnout deadline expires, it burns out and the simulation ends.

The main goal of the project is to design a robust synchronization system capable of:

* Managing concurrent access to shared resources
* Preventing deadlocks
* Reducing starvation risks
* Detecting burnout precisely
* Coordinating multiple threads safely
* Supporting different scheduling strategies

Two scheduling policies are implemented:

* **FIFO (First In First Out)**: coders are served according to request order.
* **EDF (Earliest Deadline First)**: coders with the most urgent burnout deadline receive priority.

---

## Features

* Multi-threaded simulation using POSIX threads
* FIFO and EDF scheduling algorithms
* Priority queues implemented with binary heaps
* Shared resource management through mutexes
* Condition variables for efficient waiting
* Dedicated monitor thread for burnout detection
* Serialized logging output
* Configurable cooldown period after resource release
* Configurable compilation quota

---

## Instructions

### Compilation

Compile the project using:

```bash
make
```

Remove generated files:

```bash
make clean
```

Remove all generated files including the executable:

```bash
make fclean
```

Rebuild the project:

```bash
make re
```

---

### Execution

```bash
./codexion <nb_coders> <t_burnout> <t_compile> <t_debug> <t_refactor> [nb_compiles] <cooldown> <scheduler>
```

### Parameters

| Parameter   | Description                           |
| ----------- | ------------------------------------- |
| nb_coders   | Number of coders                      |
| t_burnout   | Maximum time before a coder burns out |
| t_compile   | Time spent compiling                  |
| t_debug     | Time spent debugging                  |
| t_refactor  | Time spent refactoring                |
| nb_compiles | Optional compilation quota            |
| cooldown    | Resource cooldown after release       |
| scheduler   | `fifo` or `edf`                       |

### Example

```bash
./codexion 5 800 200 200 200 5 10 fifo
```

This launches:

* 5 coders
* burnout time: 800 ms
* compile time: 200 ms
* debug time: 200 ms
* refactor time: 200 ms
* compilation quota: 5
* cooldown: 10 ms
* FIFO scheduling

---

## Blocking Cases Handled

### Deadlock Prevention

The implementation prevents circular waiting by enforcing a deterministic acquisition order for shared resources.

Each coder always requests dongles according to a consistent ordering rule, eliminating the possibility of circular dependency between threads.

This directly breaks one of Coffman's four necessary conditions for deadlock:

* Mutual exclusion ✔
* Hold and wait ✔
* No preemption ✔
* Circular wait ✘

Since circular wait is prevented, deadlocks cannot occur.

---

### Starvation Prevention

The scheduler maintains waiting coders inside priority queues.

Under FIFO scheduling, coders are served according to arrival order.

Under EDF scheduling, coders approaching burnout receive higher priority.

These policies significantly reduce starvation risks and ensure fair access to resources.

---

### Cooldown Handling

After a coder releases its dongles, they enter a cooldown period.

During this period:

* the dongles cannot be immediately reused,
* waiting threads remain blocked,
* the scheduler reevaluates access once the cooldown expires.

This avoids unrealistic monopolization of resources.

---

### Precise Burnout Detection

A dedicated monitor thread continuously checks every coder.

Burnout is detected when:

```text
current_time - last_compile_start > t_burnout
```

The monitor immediately:

1. marks the simulation as stopped,
2. prints the burnout event,
3. terminates the simulation safely.

This guarantees accurate timing and deterministic behavior.

---

### Log Serialization

Multiple threads may attempt to write to standard output simultaneously.

To prevent corrupted or interleaved logs, every print operation is protected by a dedicated logging mutex.

This guarantees:

* complete log messages,
* chronological readability,
* thread-safe console output.

---

## Thread Synchronization Mechanisms

### pthread_mutex_t

Mutexes are used to protect all shared data structures.

Examples include:

* dongles
* monitor state
* global stop flag
* logging system
* scheduler data

Example:

```c
pthread_mutex_lock(&dongle->mutex);
/* protected access */
pthread_mutex_unlock(&dongle->mutex);
```

This prevents simultaneous modification of shared resources.

---

### pthread_cond_t

Condition variables allow threads to wait efficiently without busy waiting.

A coder that cannot immediately acquire a dongle sleeps on a condition variable until the resource becomes available.

Example:

```c
pthread_cond_wait(&dongle->cond, &dongle->mutex);
```

When a dongle is released:

```c
pthread_cond_broadcast(&dongle->cond);
```

waiting coders are awakened and reevaluate scheduling conditions.

This minimizes CPU usage while maintaining responsiveness.

---

### Monitor Thread

A dedicated monitor thread supervises all coders.

Responsibilities include:

* burnout detection,
* simulation termination,
* completion detection.

The monitor communicates through protected shared state rather than active polling between worker threads.

---

### Race Condition Prevention

Shared variables are never accessed concurrently without synchronization.

Examples:

* simulation stop flag
* last compilation timestamp
* compilation counters
* scheduler queues

Mutexes ensure that only one thread modifies protected data at a time.

---

### Thread-Safe Communication

Communication between coders and the monitor is achieved through:

* shared synchronized state,
* condition variable signaling,
* protected timestamps,
* protected completion counters.

This design avoids undefined behavior and ensures deterministic coordination.

---

## Project Architecture

```text
Monitor Thread
        │
        ▼
   Shared State
        │
 ┌──────┼──────┐
 ▼      ▼      ▼
Coder1 Coder2 CoderN
   │      │      │
   └── Dongles ──┘
```

Main components:

* Coder threads
* Monitor thread
* Dongles
* Scheduler (FIFO / EDF)
* Heap-based priority queues

---

## Resources

### POSIX Threads Documentation

* https://man7.org/linux/man-pages/man7/pthreads.7.html
* https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_create.html

### Synchronization Concepts

* https://en.wikipedia.org/wiki/Mutex
* https://en.wikipedia.org/wiki/Condition_variable
* https://en.wikipedia.org/wiki/Deadlock
* https://en.wikipedia.org/wiki/Coffman_conditions

### Scheduling Algorithms

* https://en.wikipedia.org/wiki/First_in_first_out_(computing)
* https://en.wikipedia.org/wiki/Earliest_deadline_first_scheduling

### Binary Heaps

* https://en.wikipedia.org/wiki/Binary_heap

### AI Usage

Artificial intelligence tools were used as development assistants during the project.

AI assistance was limited to:

* reviewing synchronization logic,
* discussing deadlock prevention strategies,
* debugging concurrency issues,
* identifying race conditions,
* improving documentation quality,
* generating README structure and explanations.

All implementation decisions, testing, integration, validation, and final code modifications were performed manually by the project author.
