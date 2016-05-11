# TODO

Grade is out of 200 Points (because it is a program).

## Baseline Effort (70 points)
~~Reasonable effort... LFS code that compiles and does some kind of file system type stuff. (low bar here)~~

## LFS Structure (25 points)

Demonstrate each of the following:

  - ~~[5 points] Show contents of checkpoint region (imap locations and byte array for clean/dirty segments)~~
  - [10 points] Show contents of segment summary blocks (make it non-trivial... different file types... some live blocks, some not, etc.... how can you tell an imap block from an inode from a data block?)
  - ~~[5 points] Show contents of imap -- Create a handful of files, then delete a couple, then create more and show that the imap knows that the "old" inode number is gone (that is, not mapped to a block number any more).~~
  - ~~[5 points] Proper DRIVE/SEGMENT files - 32 files inside drive, each 1 MB in size - should be these sizes on startup and after file system ages a little bit.~~
  - ~~[5 points] No extra files in DRIVE directory (or anywhere else), and DRIVE is initialized to 0's in segment files. (A "file map" that maps file names to inode numbers is OK of course)~~

## Basic Commands (55 points)
Demonstrate the following simple operations:
  - ~~[5 points] import file with size < 1 block~~
  - ~~[5 points] import file with size > 1 block~~
  - ~~[5 points] cat file with size  < 1 block~~
  - ~~[5 points] cat file with size > 1 block~~
  - ~~[5 points] remove file with size < block (show it worked - no credit if you only do it without proving it)~~
  - ~~[5 points] remove file with size > block (show it worked - same as above)~~
  - ~~[5 points] display command~~
  - ~~[5 points] display command with different bytes and/or file~~
  - [5 points] overwrite without needing another data block (and show it worked - again no credit if its done without being proven it worked)
  - [5 points] overwrite so another data block must be allocated (show it worked)
  - ~~[5 points] list (at least once, this will probably need to be done multiple times to show commands are working)~~

## Exit/Restart and Cleaning (50 pts)

  - [20 pts] clean (without exiting) - show that inodes and blocks are still accessible, and actually moved as a result of cleaning; and show the checkpoint region info to see that the number of clean segments increases.
  - ~~[10 pts] exit -> restart -> show restart loaded data properly -- everything should be as it was before exiting.~~
  - [10 pts] exit -> restart -> clean -> show restart loaded data and cleaning worked
  - [10 pts] clean -> exit -> restart -> show data as above
These can be done in any order, and for any commands where possible, should be shown to have worked correctly. Any commands that are not proven to have worked will be 0 points.

## Bonus (Up to 10 points)
  - Can you max out the number of inodes?!
  - Can you fill the entire file system and then recognize that?!
  - Any additional commands that we did not ask for?  (gotta be substantial file system type stuff...)
  - Bonus points will be hard to come by; if you get some, I will be impressed and I've asked Chelsea to tell me about it!
