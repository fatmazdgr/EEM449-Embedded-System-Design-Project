# EEM449-Embedded-System-Design-Project

In this project, a mobile robot platform was controlled over the internet. Project is worked on Tiva C Series EK-TM4C1294XL and an integrated development environment (IDE) is Code Composer Studio which version is 10.0. The mobile robot has two DC motors and these motors are controlled by a L298N Voltage Regulator Dual Motor Driver. On launchpad, it is created TCP/IP server that waits for incoming connections over the port 5030. The server waits for commands from the Internet Server. Client can send HELLO, GETTIME, LEFT X(Left motor running time in seconds), RIGHT Y(Right motor running time in seconds), EXEC and QUIT. Once program receives “LEFT 2” command, it will understand that LEFT motor will be active for 2 seconds. Similarly, “RIGHT 3” means that RGHT motor will be active for 3 seconds. When program receives “EXEC” commands, it will start executing the previous commands. All synchronization is provided using events and mailboxes. Also, this program receives time data from the NTP server once and updates the received time with the timer in the system. When the client sends “GETTIME” to server, server sends data that includes year, month, day, hour, minutes and seconds to client.

Video link:https://drive.google.com/file/d/1p1rykRCEhr6T5F7s5LpNEVmeoINEtl12/view?usp=sharing
