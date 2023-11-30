Embedded Systems 💻
======================================

Project Description 📎
------------------
1. Simulate an algorithm that needs 7 ms for its execution, and needs to work at 100 Hz.
2. Read characters from UART and display the characters received on the first row of the LCD.
3. When the end of the row has been reached, clear the first row and start writing again from the first row first column.
4. Whenever a CR ’\r’ or LF ’\n’ character is received, clear the first row.
5. On the second row, write ”Char Recv: XXX”, where XXX is the number of characters received from the UART2. (use sprintf(buffer, “%d”, value) to convert an integer to a string to be displayed.
6. Whenever button S5 is pressed, send the current number of chars received to UART2.
7. Whenever button S6 is pressed, clear the first row and reset the characters received counter.

Tools Needed 🧰
-----------------

▶ MPLAB X IDE:
http://www.microchip.com/mplab/mplab-x-ide

▶ XC16 Compiler:
http://www.microchip.com/mplab/compilers

Team Members 👨‍🔬
-------------

|    |Name |Surname |
|----|---|---|
| 1 | Manuel | Delucchi |
| 2 | Matteo | Cappellini |
