#ifndef KERNEL_H
#define KERNEL_H

#include <iostream>
using namespace std;

class kernel
{
public:
    kernel();
    int runlevel = 0;
    string ver = "Kernel version 0.1.0; Lunix shell 0.1.0";

    // Start the kernel
    void start();

    // Shutdown the system
    void shutdown();

    // Process control
    int fork();
    int exec();
    int wait();

    void haltrq(string reason); // External request to halt
    void crlrq(int rl); // External request to change runlevel

    void panic(string reason);
    void oops(string reason);


private:
    /*
     * Runlevels
     * 0 = System halted, can be powered off safely
     * 1 = Kernel starting and initiating devices such as Network
     * 2 = Kernel finished starting, additional actions and startup scripts are done at this stage
     * 3 = Startup finished. Waiting for interactive shell.
     * 4 = Single, interactive terminal started
     */
    void crl(int rl);

    /*
     * Simplified version of kernel panic.
     * Halts the kernel, exits, does not hang the system like panic.
     */
    void halt(string reason);

    void print_stack_trace();
    void setup_signal_handlers();
    static void handle_signal(int signal);

    void log_to_file(const string& log_message);
    void dump_registers(stringstream& panic_info);

    // Check if there are root privileges at startup
    void check_sudo();

    bool process_is_alive(pid_t pid);
};

#endif // KERNEL_H
