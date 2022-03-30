# ViterbiDecoderFor802.11
This is a part of our cuda sdr project, written in pure c++ without any other dependencies, which can be easily changed to cuda implementation. 
This decoder can not be used as a generic viterbi decoder. 
It is based on 802.11 standard. 
But hopefully, we will present some basic views of the viterbi algorithm in the following sections, which may give you an idea to implement your own decoder.

***Notes: since we only tested this code using a limited bunch of input, there might be unforeseen bugs in some cases.***


## 802.11 std Encoder
In 802.11, the convolutional encoder use industry-standard generator polynomials 133(octal) and 171(octal). In this case, polynomials are MSB first. In our actual implementation, we use a LSB first view, which does not change anything but can provide some convinient properties.  ![Alt pic](https://github.com/milinzhang/ViterbiDecoderFor802.11/blob/main/fig/ConvEnc.png)

## Butterflies
In the trilles graph, branches between current states and previous states have a butterfly-like pattern. This can let us easily represent multiple states using one butterfly. In the LSB view, when the bit at rightmost is 0, every input will either double the number (when input is 0) or double the number and plus 1 (when input is 1). Similarly, when the bit at rightmost is 1, every input will shift registers and cause same result as the state who has same other bits but the rightmost is 0 (which is 32 less than the previous one) does. Therefore, we can use 32 butterflies to represent 64 previous states and 64 current states.

Another convinient property of these butterflies is that the output bits A and B of outter branchs are just the inverse of the inner branch. (This can not be applied in a generic case. But in 802.11 or other situations where generator polynomials have both 1s at rightmost and leftmost, it is indeed the truth.) So in our implementation, we only use a 2X32 lookup table to store all the outputs of branches.
![Alt pic](https://github.com/milinzhang/ViterbiDecoderFor802.11/blob/main/fig/Butterfly.png)

## Forward phase
### compute branch metrics
The Viterbi algorithm can be seperated into two phase: we first compute path metrics in a forward view, then we pick up a path who have the maximum score (equivalently having the minimum hamming distance) to trace back and decode. In the forward phase, basically two things should be done. First compute the hamming distance between all branchs and the received bits. ![Alt pic](https://github.com/milinzhang/ViterbiDecoderFor802.11/blob/main/fig/BranchCompute.png)
### add compare select
After doing this, we should compute the cumulative path metrics and prune those branches which are less likely to achieve, which is so-called Add-Compare-Select (ACS). This is the most crucial feature of Viterbi algorithm. By ultilizing the properties of butterflies we have mentioned above, we can easily achieve this algorithm as shown in figures. ![Alt pic](https://github.com/milinzhang/ViterbiDecoderFor802.11/blob/main/fig/ACS.png)
### initial and tailing bits
Another thing to which readers should pay attention is that the decoder should always start from the all-zero state. Our way to do this is to give a initial score to state 0 which is large enough to ensure that other branchs can be pruned.
>path_metric[initial_state] = 2*6+1
>
>path_metric[other_state] = 0

Also, in 802.11, every received sequence has at least six 0s at its tail in order to refresh the register to the all-zero state. Hence we should only take those branchs corresponding to input 0 into consideration at the last 6 steps. However, we didn't do this actually in the code since usually sequences are long enough (which means the culmulated hamming distance of the correct path is large enough) to avoid decoding errors.
## Backward phase
In the backward phase, we use a circular table to store indices of all previous survivor path. One of the convinient property of LSB representation is that when the input bit is 0, the corresponding state will always be even(state 2f). On the contrary, when the input bit is 1, the corresponding state is always odd (state 2f+1). this can make the decoding phase really easy
> output_bit = survivor_state % 2 ? 1:0;

## Depuncturing
In the 802.11 standard, a higher coding rate is achieved by removing some encoded bits in a certain way. To decode these data correctly, we should insert some dummy bits and these dummy bits should not be used to calculate branch metrics. This part of code will be updated in the future. ![Alt pic](https://github.com/milinzhang/ViterbiDecoderFor802.11/blob/main/fig/Depuncturing.png)
