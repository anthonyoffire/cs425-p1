# Project 1

- Name: Anthony Baird
- Email: anthonybaird@u.boisestate.edu
- Class: CS525-001

## Known Bugs or Issues

None are known.

## Experience

VPN trouble again - I tried setting up the recommended VPN on my Windows desktop. It did not work; it lets me log in and 2FA, then just gets stuck. Spent 2 hours trying to get that to work, then gave up and went back to the laptop using openconnect for the onyx portion.

This assignment was huge, complicated, and difficult. In my experience, this is the type of thing you usually have more towards a month to complete - especially since there are many aspects to learn about during the process. I haven't used C in 2-3 years, so it's all a little rusty. I'd never done sockets in C before. Unfamiliar with this argument library, so that took a couple of hours. It builds up! For everything you're unfamiliar with, that thing also uses structs and datatypes you're not familiar with, so you have to go research those too. I've never had a class that required full test coverage, and hadn't done testing in C at all before this course. After getting it all correct (I think?), I have spent a total of 22 hours working on it; much more than the estimated "8 hours". If I'd have had homework in all three of my classes this week, I would not have had enough time to get anywhere near completing this. It almost dug into my time for the capstone project. I'd appreciate the projects being a LITTLE smaller, or having a bit longer to complete them, in the future please! If they're all like this, I'm sure there will be one or two that I have to just decide not to do.

As far as research sources, I used man pages, GeeksForGeeks, and StackOverflow to solve a few problems. I enabled (probably ai) auto-suggestions in vscode, but mainly just used it to type things that I was already planning on typing. It did dynamically highlight some bugs for me while I was working, so that was helpful. It also screwed me occasionally by adding extra things I didn't notice before hitting tab, and didn't notice until tracking down why it wouldn't run. I did use it for the header file function documentation comments, double-checking that they looked right.

I developed on codespaces this time. When I shipped it to onyx, make leak-test showed a segfault that was not in codespaces. I had to redo 1 test and add another to re-hit a null body case using tmpfile instead of fmemopen. I had a heck of a time keeping all of the functions that should be static static, because in the current environment if you include a .c file in the testing file, you gain access to them but the compiler starts complaining about double-definition of the non-static methods in that .c file. My solution was to use a keyword that evaluates to static in production, and nothing during TEST, and define prototypes in the header file if TEST is defined. I don't know if that's the right way, but it does work!

## Design
Layers:

Layer 1 holds only helper functions with no side-effects and no system/library calls, which is very easy to test. Layer 2 holds all protocol logic, with the system/library calls abstracted. This makes it far easier to test the protocol logic. Layer 3 holds only the system/library calling functionality, and thus doesn't need to be tested. I did split arguments into a different file, so hopefully that's okay! I looked at the makefile and it appears to pick everything up, as well as the submission report.