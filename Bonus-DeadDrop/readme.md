DOCUMENTATION: Lab1.2

Additionally, 
I had missed out on adding 2 important point in the paper due to my system crash

1. A bit threshold has been introduced in which a bit has be received atleast BIT_THERSHOLD number of times to be recognized as a received bit. Else, the receiver is in busy waiting

2. The eviction set been mapped non-consecutivley in such a way that, set 0 of the virtual address is actually mapped to set (3*0 +2 =2 ), set 2 of the physical address. This is to ensure that we have minimal interference from other modules

Running modes:
To run the program in 1 bit mode, please modify the util.h's L2_SETS_DEFINE to 1.
To run the program in 256 bits mode, modify the above parameter to 255. 