// application.cpp (or wherever your CSP code resides)

#include "application.h"
#include "csp/csp4cmsis.h"
#include <cstdio>

using namespace csp;

// --- Button event type ---
struct ButtonEvent {
    bool pressed;   // true = pressed, false = released
};

// --- Channel for button events (non‑blocking, keep newest) ---
static Channel<ButtonEvent> buttonChan;

struct trigger_t {};
static Channel<trigger_t> g_trigger_chan;

using MessageType = unsigned int;
static Channel<MessageType> counterChan;

// --- C‑callable function for the ISR ---
extern "C" void csp_send_button_event(bool pressed) {
    // putFromISR never blocks; with KeepNewest it always succeeds.
    buttonChan.writer().putFromISR(ButtonEvent{pressed});
}

class ButtonProcess : public CSProcess {
    Chanin<ButtonEvent> in;
    Chanout<trigger_t> out;
public:
    ButtonProcess(Chanin<ButtonEvent> r, Chanout<trigger_t> w) : in(r), out(w) {}
    void run() override {
        ButtonEvent ev;
        trigger_t t;
        while (true) {
            in >> ev;   // blocks until a button event arrives
            if (ev.pressed){
            	printf("Blue button released\r\n");
            	out << t;
            }
            else
            	printf("Blue button pressed\r\n");
        }
    }
};

class Sender : public CSProcess {
	Chanin<trigger_t> in;
    Chanout<MessageType> out;
public:
    Sender(Chanin<trigger_t> r, Chanout<MessageType> w) : in(r), out(w) {}
    void run() override {
        unsigned int counter = 0;
        trigger_t t;
        while (true) {
        	in >> t;
            out << counter;
            counter++;
        }
    }
};

class Receiver : public CSProcess {
    Chanin<MessageType> in;
public:
    Receiver(Chanin<MessageType> r) : in(r) {}
    void run() override {
        MessageType received;
        while (true) {
            in >> received;
            printf("Send: %u Received: %u\r\n", received, received);
        }
    }
};

// --- Main application task (unchanged except adding button process) ---
void MainApp_Task(void* params) {
    vTaskDelay(pdMS_TO_TICKS(10));
    printf("\r\n--- Single Sender & Receiver + Button ISR ---\r\n");

    static ButtonProcess buttonProc(buttonChan.reader(), g_trigger_chan.writer());
    static Sender sender(g_trigger_chan.reader(), counterChan.writer());
    static Receiver receiver(counterChan.reader());

    Run(
        InParallel(sender, receiver, buttonProc),
        ExecutionMode::StaticNetwork
    );
}

void csp_app_main_init(void) {
    BaseType_t status = xTaskCreate(MainApp_Task, "MainApp", 2048, NULL, tskIDLE_PRIORITY + 3, NULL);
    if (status != pdPASS) {
        printf("ERROR: MainApp_Task creation failed!\r\n");
    }
}
