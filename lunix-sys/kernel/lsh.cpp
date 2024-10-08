// lsh.cpp
// SPDX-FileCopyrightText: 2024 <copyright holder> <email>
// SPDX-License-Identifier: GPL-3.0-or-later



#include "lsh.h"

#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <cstdlib>


#include "disk/disk.h"
#include "kernel/kernel.h"
#include "kernel/error_handler.h"
#include "color.h"
#include "security/userman.h"
#include "net/lisp/server/server.h"
#include "net/lisp/client/client.h"

using namespace ANSIColors;

namespace fs = std::filesystem;
extern disk Disk;
extern kernel Kernel;
extern error_handler ErrHandler;
UserManager userManager;

Server server;
Client client;

std::string python_bin;

lsh::lsh() {}

void lsh::printHelp() {
    const std::vector<std::pair<std::string, std::string>> commands = {
        {"cat", "Display the contents of a file"},
        {"cd", "Change the current working directory"},
        {"chmod", "Change the permissions of a file or directory"},
        {"editor", "Open a simple text editor (use 'nano' for more advanced features)"},
        {"exit", "Exit the shell"},
        {"help", "Display this help information"},
        {"ls", "List files and directories in the current directory"},
        {"mkdir", "Create a new directory in the current working folder"},
        {"nano", "Run the Nano text editor"},
        {"passwd", "Change the password for a user (root only)"},
        {"pwd", "Print the current working directory"},
        {"mod", "Run a module"},
        {"rl", "Display the current system runlevel"},
        {"rm", "Remove a file or empty directory\n"
             "  Use -R to delete a directory and its contents recursively"},
        {"shutdown", "Shut down the system and exit the shell"},
        {"ver", "Display the OS and shell version information"}
    };

    std::cout << "\n" << "Available commands:" << "\n\n";
    std::cout << "built-in: {";
    for (size_t i = 0; i < commands.size(); ++i) {
        std::cout << commands[i].first;
        if (i < commands.size() - 1) {
            std::cout << ", ";
        }
        if ((i + 1) % 10 == 0) {
            std::cout << "\n          ";
        }
    }
    std::cout << "}\n";
}

void lsh::man(const std::string& command) {
    const std::vector<std::pair<std::string, std::string>> commands = {
        {"cat <file>", "Display the contents of a file"},
        {"cd <directory>", "Change the current working directory"},
        {"mod <module-name>", "Run a python module that adds functionality to Lunix. (e.g. a module that scans the directory for a file). Modules can be used as commands. Startup modules have a startup function that runs on Lunix boot."},
        {"chmod <args>", "Change the permissions of a file or directory"},
        {"editor <file>", "Open a simple text editor (use 'nano' for more advanced features)"},
        {"exit", "Exit the shell"},
        {"help", "Display this help information"},
        {"ls", "List files and directories in the current directory"},
        {"mkdir <directory>", "Create a new directory in the current working folder"},
        {"nano", "Run the Nano text editor"},
        {"passwd <username> <new_password>", "Change the password for a user (root only)"},
        {"pwd", "Print the current working directory"},
        {"rl", "Display the current system runlevel"},
        {"rm [-R] <file/directory>", "Remove a file or empty directory\n"
             "  Use -R to delete a directory and its contents recursively"},
        {"shutdown", "Shut down the system and exit the shell"},
        {"ver", "Display the OS and shell version information"}
    };

    for (const auto& [cmd, desc] : commands) {
        if (cmd.find(command) != std::string::npos) {
            std::cout << cmd << "\n" << desc << "\n";
            return;
        }
    }
    std::cout << "No manual entry for " << command << "\n";
}

void lsh::changeDirectory(const std::string& path) {
    Disk.fchdir(path);
}

void lsh::printWorkingDirectory() {
    std::cout << fs::current_path() << std::endl;
}

void lsh::catFile(const std::string& filename) {
    try {
        std::ifstream file(filename);
        if (file) {
            if (fs::is_directory(filename)) {
                throw std::runtime_error("Cannot cat a directory");
            }
            std::cout << file.rdbuf();
        } else {
            std::cerr << "No such file: " << filename << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void lsh::simpleEditor(const std::string& filename) {
    std::string content;
    std::cout << "Simple Editor\n";
    std::cout << "Editing " << filename << ". Type ':wq!' to save and exit.\n";

    std::ofstream outFile(filename, std::ios::app);
    while (true) {
        std::getline(std::cin, content);
        if (content == ":wq!") break;
        outFile << content << std::endl;
    }
    outFile.close();
}

void lsh::listFiles(const std::string& path) {
    try {
        fs::path dirPath = fs::absolute(path);
        for (const auto& entry : fs::directory_iterator(dirPath)) {
            std::string name = entry.path().filename().string();
            if (fs::is_directory(entry.status())) {
                std::cout << BOLD_BLUE << name << "/" << RESET << std::endl;
            } else {
                std::cout << name << std::endl;
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

/**
 * Starts the Lunix shell.
 *
 * This function initializes user management and handles the login process.
 * It displays a welcome message and a prompt for user input.
 * The function continuously reads user commands and executes them until the user enters "shutdown" or "exit".
 * Supported commands include changing directories, listing files, creating directories, viewing file contents,
 * editing files, changing permissions, deleting files and directories, and more.
 *
 * @return 0 upon successful completion.
 */


int lsh::lshStart() {
    userManager.initialize();  // Initialize user management and handle login

    Kernel.crlrq(4);
    std::string command;
    fs::path rootfsPath = fs::current_path();  // Get the current path after rootfs() has been called

    std::cout << "\nWelcome to Lunix, a small simulation OS built on C++" << std::endl;
    std::cout << "lsh shell 0.2.0; type 'help' for commands\n\n";
    std::cout << "Current working directory: " << rootfsPath << std::endl;

    while (true) {
        fs::path currentPath = fs::current_path();
        std::string promptPath;

        if (currentPath == rootfsPath) {
            promptPath = "~";
        } else if (currentPath.string().find(rootfsPath.string()) == 0) {
            promptPath = "~" + currentPath.string().substr(rootfsPath.string().length());
        } else {
            promptPath = currentPath.string();
        }

        std::cout << (userManager.isRoot() ? BOLD_RED + "root" + RESET : BOLD_GREEN + userManager.getUsername() + RESET)
                  << "@" << BOLD_CYAN << "lunix " << RESET << promptPath << (userManager.isRoot() ? " # " : " % ");

        if (!std::getline(std::cin, command)) {
            std::cout << std::endl;  // Print a newline to move to the next line after the prompt
            break;
        }

        if (command == "shutdown" || command == "exit") {
            break;
        } else if (command == "") {

        } else if (command == "help") {
            printHelp();
        } else if (command.substr(0, 4) == "man ") {
            lsh::man(command.substr(4));
        } else if (command.substr(0, 3) == "cd ") {
            if (Disk.fchdir(command.substr(3)) != 0) {
                std::cout << "Directory " << command.substr(3) << " not found.\n";
            }
        } else if (command == "pwd") {
            std::cout << Disk.fcwd() << std::endl;
        } else if (command.substr(0, 2) == "ls") {
            std::string path = ".";
            if (command.length() > 3) {
                path = command.substr(3);
            }
            listFiles(path);
        } else if (command.substr(0, 6) == "mkdir ") {
            Disk.fmkdir(command.substr(6));
        } else if (command.substr(0, 4) == "cat ") {
            catFile(command.substr(4));
        } else if (command.substr(0, 7) == "editor ") {
            simpleEditor(command.substr(7));
        } else if (command.substr(0, 4) == "nano") {
            system(command.c_str());
        } else if (command.substr(0, 5) == "chmod") {
            system(command.c_str());
        } else if (command.substr(0, 2) == "rl") {
            std::cout << Kernel.runlevel << std::endl;
        } else if (command.substr(0, 3) == "rm ") {
            std::string args = command.substr(3);
            bool recursive = false;

            if (args.substr(0, 3) == "-R ") {
                recursive = true;
                args = args.substr(3);
            }

            if (recursive) {
                if (Disk.frmdir_r(args) == 0) {
                    std::cout << "Deleted directory " << args << " recursively.\n";
                } else {
                    std::cout << "Failed to delete directory " << args << ".\n";
                }
            } else {
                if (Disk.funlink(args) == 0) {
                    std::cout << "Deleted " << args << ".\n";
                } else {
                    std::cout << "Failed to unlink " << args << ". Error code " << Disk.funlink(args) << std::endl;
                }
            }
        } else if (command == "clear") {
            system("clear");
        } else if (command == "ver") {
            std::cout << Kernel.ver << std::endl << "Codename " << Kernel.codename << std::endl;
            std::ifstream buildDateFile("../.builddate");
            if (buildDateFile) {
                std::string buildDate;
                std::getline(buildDateFile, buildDate);
                std::cout << "Built on: " << buildDate << std::endl;
            } else {
                std::cerr << "Failed to open .builddate file." << std::endl;
            }
        } else if (command.substr(0, 2) == "./") {
            std::string executable = command.substr(0);
            if (Disk.fopenbin(executable) != 0) {
                std::cout << "Failed to execute '" << executable << "'." << std::endl;
            }
        } else if (command == "panic") {
            if (userManager.isRoot()) {
                ErrHandler.panic("User initiated panic using 'panic' command");
            } else {
                cout << "Failed to run command: Permission denied\n";
            }
        } else if (command.substr(0, 6) == "passwd") {
            if (userManager.isRoot()) {
                std::string user, newPassword;
                std::istringstream iss(command.substr(7));
                iss >> user >> newPassword;
                if (user.empty() || newPassword.empty()) {
                    std::cout << "Usage: passwd <username> <new_password>\n";
                } else {
                    userManager.setPassword(user, newPassword);
                }
            } else {
                std::cerr << "Only root can change other users' passwords.\n";
            }
        } else if (command == "server start") {
            server.start();
        } else if (command == "server stop") {
            server.stop();
        } else if (command == "client ping") {
            std::string ipAddr;
            std::cout << "IP address of the server: ";
            std::cin >> ipAddr;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            client.pingServer(ipAddr.c_str(), 6942);
        } else if (command == "client connect") {
            std::string ipAddr;
            std::cout << "IP address of the server: ";
            std::cin >> ipAddr;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            client.connectToServer(ipAddr.c_str(), 6942);
        } else if (command.substr(0, 3) == "mod") {
            // Check if the command length is sufficient to avoid out_of_range exception
            if (command.length() < 4) {
                std::cout << "Usage: mod <module-name>\n";
            } else {
                std::string moduleName = command.substr(4);
                std::cout << "Loading module " << moduleName << "\n";
                if (Disk.loadMod(moduleName) != 0) {
                    std::cout << "An error occurred loading the module\n";
                }
            }
        }
        else {
            std::cout << "Command not found: " << command << std::endl;
        }
    }

    return 0;
}