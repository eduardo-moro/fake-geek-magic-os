#pragma once

// Claude Desktop Buddy App
// Button A (pin 32, capacitive touch) = approve permission prompt
// Button B (pin 0, physical push) = deny permission prompt

void buddy_app_start();
void buddy_app_loop();
void buddy_app_quit();
void buddy_app_send_approve();  // Called when button A is pressed
void buddy_app_send_deny();     // Called when button B is pressed
