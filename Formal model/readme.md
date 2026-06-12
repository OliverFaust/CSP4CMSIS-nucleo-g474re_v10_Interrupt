# Interrupt-Driven Counter Pipeline: Formal Model (CSP-M)

This directory contains the formal **Communicating Sequential Processes (CSP)** model for bridging asynchronous hardware events (interrupts) with a synchronous process network. The specification is written in machine-readable CSP-M and is configured for animation and verification using the [ProB](https://prob.hhu.de/) model checker.

While the physical platform handles edge-triggered hardware pins via non-blocking Interrupt Service Routines (ISRs) and the `CSP4CMSIS` library's `putFromISR` wrapper, this formal abstraction evaluates the logical integrity, concurrency safety, and deterministic liveness of the pipeline.

## Network Architecture

The system models the interaction between the asynchronous physical environment and a multi-stage execution pipeline via synchronized rendezvous channels:

1. **`ENV` (ISR Environment):** Models the physical button toggles as an external entity. It non-deterministically yields an unbounded sequence of `pressed` and `released` states via internal choice (`|~|`) to evaluate the pipeline against arbitrary physical input timing.
2. **`ButtonProcess`:** Acts as the software input driver. It blocks on `buttonChan`. Upon receiving a `pressed` event, it propagates an unvalued signal across `g_trigger_chan` to wake the downstream processing core; `released` events are safely consumed and dropped without downstream consequences.
3. **`Sender(n)`:** A parameterized processing stage initialized at `Sender(0)`. It blocks on the empty trigger event, outputs its internal state `n` over `counterChan`, and then loops with an incremented counter wrapped via modulo arithmetic.
4. **`Receiver`:** Represents the terminal output stage. It blocks until a state message arrives on `counterChan`, captures the data locally (`counterChan?x`), and maps it directly to an observable `print_!x` event.

## System Composition

The structural composition of the system is modeled as a linear processing chain divided into distinct synchronization zones using the `SYSTEM` process:

```csp
SYSTEM =
    (ENV [| {|buttonChan|} |] ButtonProcess)
        [| {|g_trigger_chan|} |]
    (Sender(0) [| {|counterChan|} |] Receiver)
