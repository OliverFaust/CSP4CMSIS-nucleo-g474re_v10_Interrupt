# CSP4CMSIS Interrupt Demo for NUCLEO-G474RE

A real‑time embedded demonstration of the **CSP (Communicating Sequential Processes)** library using CMSIS‑RTOS v2 on an STM32G474RE microcontroller. This project shows how to safely connect an external interrupt (blue user button) to CSP channels, triggering a chain of process communications.

## Features

- **FreeRTOS** with CMSIS‑RTOS v2 API  
- **CSP4CMSIS** library for channel‑based, deterministic concurrency  
- **External interrupt** (blue button on PC13) configured for both rising/falling edges  
- **Zero‑heap** static memory allocation – all channels and processes reside in `.data`/`.bss`  
- **ISR to task communication** using `putFromISR` on a non‑blocking channel  
- **Trigger chain**: Button press → ButtonProcess → Sender → Receiver  
- **Roll‑over counter** sent from Sender to Receiver (unsigned integer)  
- **Serial console output** via USART1 (115200 baud)

## Hardware Requirements

- STM32 Nucleo‑G474RE board  
- USB cable for power, programming, and serial communication  
- The blue user button (PC13) – no external components needed

## Software Requirements

- STM32CubeIDE (or any ARM GCC toolchain)  
- CSP4CMSIS library (included as a git submodule or directly in `lib/`)

## Serial Configuration

| Parameter   | Value          |
|-------------|----------------|
| Baud Rate   | 115200         |
| Data Bits   | 8              |
| Stop Bits   | 1              |
| Parity      | None           |
| Flow Control| None           |

## Building with STM32CubeIDE

1. **Clone this repository** (do not place it inside your STM32CubeIDE workspace directory).  
2. Open STM32CubeIDE.  
3. Go to `File → Import → Existing Projects into Workspace`.  
4. Select the cloned directory.  
5. Build the project (default configuration `Debug` or `Release`).  
6. Flash the binary to your Nucleo board.

## Project Structure
```text
├── Core/ # Main application code (main.c, application.cpp)
├── Drivers/ # STM32 HAL drivers
├── lib/CSP4CMSIS/ # CSP library (static, zero‑heap)
├── Middlewares/ # FreeRTOS + CMSIS‑RTOS v2
├── .gitignore # Excludes build artefacts
└── README.md
```

## How It Works

1. **Hardware interrupt**: The blue button (PC13) is configured in `main.c` to trigger on **both rising and falling edges**. The ISR calls `csp_send_button_event(pressed)`.

2. **CSP channel**:  
   `static Channel<ButtonEvent> buttonChan;` – a synchronous rendezvous channel (blocking). The `putFromISR` call from the interrupt is safe and non‑blocking.

3. **ButtonProcess**:  
   Waits on `buttonChan`. When a button event arrives, it prints the state (`"Blue button pressed"` or `"Blue button released"`) **and** if the button is *pressed*, it sends a `trigger_t` message to the Sender via `g_trigger_chan`.

4. **Sender**:  
   Waits for a trigger on `g_trigger_chan`. Each trigger causes the Sender to send an ever‑incrementing `unsigned int` (which rolls over from `UINT_MAX` to `0`) to the Receiver.

5. **Receiver**:  
   Waits on `counterChan`, reads the number, and prints:  
   `Send: X Received: X` – ensuring that every sent number is correctly received.

All processes run in parallel using CSP’s `InParallel` operator, managed by `ExecutionMode::StaticNetwork`.

## Example Console Output

```text
Welcome to STM32 world !

=== STM32 FreeRTOS + CSP4CMSIS bootstrap ===

--- Single Sender & Receiver + Button ISR ---
Blue button pressed
Blue button released
Send: 0 Received: 0
Blue button pressed
Blue button released
Send: 1 Received: 1
Blue button pressed
Blue button released
Send: 2 Received: 2
...
```
Each press of the blue button prints a "Blue button pressed" message; each release prints a "Blue button released" message and it triggers a counter increment. The counter continues indefinitely, rolling over automatically.

## Key CSP4CMSIS Concepts Demonstrated

- **Channel** – synchronous rendezvous between processes.
- **`putFromISR`** – safe interrupt‑to‑task communication.
- **Static network** – no dynamic memory allocation after startup.
- **Process composition** – `InParallel` combines independent processes.
- **External choice** (not used here, but supported via `Alternative`).

## Troubleshooting

- **No output on serial**: Verify the baud rate and that the correct COM port is used.
- **Button not detected**: Ensure the second `BSP_PB_Init` call is removed (see `main.c` comments). The interrupt must be configured **after** `MX_GPIO_Init()`.
- **Heap usage warning**: CSP4CMSIS uses zero heap – all memory is static. If you see heap allocations, check that you are not using `new`/`malloc` elsewhere.

## License

MIT License – see `LICENSE` file (if included) or refer to the CSP4CMSIS library license.

## Acknowledgments

- STMicroelectronics for the STM32 HAL and CMSIS‑RTOS v2
- The FreeRTOS team
- [CSP4CMSIS](https://oliverfaust.github.io/CSP4CMSIS/) library by Oliver Faust
