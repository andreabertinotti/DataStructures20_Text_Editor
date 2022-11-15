# DataStructures20_Text_Editor

The project consists in implementing a simple text editor. 
The editor considers a document as a sequence of lines, of arbitrary size, numbered starting from one. 
The editor interface consists of text commands, terminated by a "head" character. 
The commands can be followed by a portion of text, consisting of one or more lines, terminated by a character "." (period), appearing as the only character on the next line. 
Commands consist of a single letter, optionally preceded by one or two integers. 
The editor interface is freely inspired by that of the traditional editor Ed. 
In some commands, the integers appearing in them represent address specifiers. 
More precisely, an address specifier is a number n, expressed in decimal, which indicates the address of the n-th row; the first line of the text has address 1. 
The supported commands are the following, with the convention that ind1,ind2 indicate two address specifiers such that ind1 ≤ ind2 and the parentheses are introduced for ease of reading this text, but not included in the command:

- (ind1,ind2)c 
Changes the text on lines between ind1 and ind2 (which have to be considered included). 
The text following the command must consist of a number of lines equal to ind2-ind1+1. 
ind1 must either be an address actually present in the text, or the first address after the last line present in the text (or 1 if the text is still empty).

- (add1,add2)d
Delete the lines between ind1 and ind2 (inclusive), moving the lines following the one with address ind2 upwards (if there are any). 
Deleting a line that doesn't exist in the text has no effect.

- (add1,add2)p
Print the lines between ind1 and ind2, (ind1 and ind2 have to be included). 
Where there is no line in the text in the position to be printed, a line containing only the character '.' followed by a "new line" is printed.

- (number)u 
Undo a number of commands (c or d) equal to the number specified in parentheses (where number is an integer strictly greater than zero). 
A sequence of consecutive undo commands undoes a number of steps equal to the sum of the steps specified in each one. 
If the number of commands to undo is greater than the number of commands executed, all steps are undone. 
Executing a text modification command (c, d) after an undo cancels the effects of the commands permanently undone. 
Note that the commands that have no effect are also counted in the number of commands to be canceled (for example the deletion of a block of lines that do not exist).

- (number)r
Cancels the undo effect for a number of commands starting from the current version (redo function). 
Note that number must be an integer strictly greater than zero. 
We therefore have a sequence of commands of the type 10u 5r is effectively equivalent to the 5u command only.

- q 
Ends execution of the editor

A line of text supplied as input to the editor can contain a maximum of 1024 characters. 

Assume that only correct commands are given to the editor (it is therefore not necessary to verify their correctness).
