# readers-writers
[![My Skills](https://skillicons.dev/icons?i=c&perline=)](https://skillicons.dev)

## Problem definition
We have a buffer of size N that can store N data items. On this buffer, several producers and several consumers operate, the producers writing about the buffer and the consumers reading from it. Without the proper timing, the following errors can occur:
- Two producers write in the same position.
- A consumer reads from the buffer while the producer is writing.

The objective of this work is to develop synchronization mechanisms between processes to allow several readers and several writes to synchronize in order to operate concurrently on this buffer. In this work it is allowed to have several writers and give higher priority to the writings than to the readers in accessing the buffer.

The synchronization mechanisms studied have been:
Mutex. They are used to avoid concurrent uses of a critical section. The thread that locks the mutex is responsible for unlocking it.
Traffic lights. They are used as signals to notify other threads of the availability of a resource. There are no restrictions as in the case of mutexes.

In timing problems, the biggest challenge is preventing deadlocks. When this happens, it can be due to a misapplication of synchronization mechanisms where one thread waits for a resource to be released while it has a resource that the other is waiting for. This causes program execution to stop indefinitely. In this problem, special care must be taken since there are many readers and writers.

## Solution
The following diagram shows the design of the synchronization mechanisms that have been used to solve the problem. There are two semaphores AR and AW, and two counters aw and rr managed by a critical section.

<img width="631" alt="Captura de pantalla 2021-05-15 a las 16 38 49" src="https://user-images.githubusercontent.com/62698658/118727830-51c68000-b82b-11eb-8c44-851289ff48fc.png">

### Definitions
- Active Reader. It is a reader that is in queue to be able to read the buffer.
- Running Reader. It is a reader that is already reading the buffer. There can be many reading the buffer simultaneously.
- Active Writer. It is a writer that is queued to be able to write to the buffer.
- Running Writer. It is a writer who is already writing to the buffer. There can only be one at a time.

### Synchronization mechanisms
- Active Writers (aw) counter. Contains the number of Active Writers queued. This counter is updated by a writer each time it is turned on or off. Therefore, a critical section must be included with a mutex to ensure that it updates correctly.
- Running Readers counter (rr). Contains the number of Running Readers. This counter is updated by a reader every time it is executed or terminated. Therefore, a critical section must be included with a mutex to ensure that it updates correctly.
- Traffic light for Active Readers (AR). It closes when aw> 0 otherwise it is open.
- Traffic light for Active Writers (AW). It closes when rr> 0 otherwise it is open.

### Cases
Case 1: There are no Active Writers
As long as there are no Active Writers, the AR Semaphore, controlled by the aw counter, will be enabled, allowing all activated readers to become Running Readers.

Case 2: An Active Writer
As there is an Active Writer, aw> 0, therefore the AR semaphore will prevent the next Active Readers from starting to read the buffer. With this, the writer is given priority to run while readers will have to wait for the writer to finish its process.

Case 3: Multiple Running Readers and a writer is activated
As long as there are Running Readers operating, that is rr> 0, the AW semaphore will prevent Active Writers from making modifications to the buffer. However, at that time the AR semaphore will also be blocked since aw> 0, thus preventing new Running Readers from appearing. As soon as the Running Readers finish, rr = 0, the AW semaphore will be unlocked and the Active Writer will start working.

(c) 2021 José Juan Hernández Gálvez
