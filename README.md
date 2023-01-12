# JavaServerforproject

Program for simulating several scenarios when a "fire" has broken out and creating a emergency route algorithm. 
"Project.ino" is for the microcontrollers, Wemos D1 mini, that were used in this project. Max3485 were used for full-duplex communication
MCP23008 were used for positioning of the microcontrollers and id allocating for the nodes.

Server was written with multiple threads, 8 microcontrollers sent information about their status to the server, which displays it in a GUI.
