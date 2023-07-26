# nent

Calculates the normalized Shannon entropy of a file.

Normalized Shannon entropy is defined as the Shannon entropy divided by the bit-width and calculated for all bit-widths smaller than and equal to the chosen bit-width (referred to as the "confidence level"), with the minimum value taken.

The difference between normalized Shannon entropy and standard Shannon entropy can be illustrated with the `test-consecutive` example. This file is a 1MB file of consecutive 8-bit integers, where every time the integer hits 255 it rolls back over to 0. See below the differences between how the `ent` command and `nent` report on this file.

```
ent: Entropy = 8.000000 bits per byte.
nent: Normalized Shannon Entropy = 70.50% at a confidence level of 14.
```

Consecutive distributions score worse the greater the confidence level is from the bit-widths of consecutive integers. Below we can see that confidence level 8 returns a result similar to the `ent` command with a perfect score, but as the level is gradually increased, the score declines.

```
nent: Normalized Shannon Entropy = 100.00% at a confidence level of 8.
nent: Normalized Shannon Entropy = 96.70% at a confidence level of 9.
nent: Normalized Shannon Entropy = 88.34% at a confidence level of 10.
nent: Normalized Shannon Entropy = 87.50% at a confidence level of 11.
nent: Normalized Shannon Entropy = 74.48% at a confidence level of 12.
nent: Normalized Shannon Entropy = 74.48% at a confidence level of 13.
nent: Normalized Shannon Entropy = 70.50% at a confidence level of 14.
```

As an interesting fact, it appears that the normalized Shannon entropy of N, where the minimum is taken from all bit-widths from 1 to N, approaches the same value as simply taking the Shannon entropy of N alone as the number of sample sizes increase. Here, with a 1MB file of consecutive numbers, the Shannon entropy of 8 and the normalized Shannon entropy of 8 both imply, misleadingly, that the distribution is uniformly random. However, much smaller file sizes of consecutive numbers will report entropy <100% at a confidence level of 8.

The result of `nent` and `ent` will both often appear the same when a confidence level of 8 is automatically chosen, however, because `nent` will, unless explicitly specified otherwise with the `-L` flag, choose the confidence level based on the number of samples. This means that `nent` will only choose a confidence level of 8 for testing if there is sufficient samples for a reliable test of 8, which with that many samples will cause the two values mentioned in the last paragraph to converge.

If this is being used to test for the quality of randomness in a file, I recommend inputting a 2 MB file, which will automatically choose the highest confidence level currently supported, of 15. A score >= 99% would be a pretty good distribution.

