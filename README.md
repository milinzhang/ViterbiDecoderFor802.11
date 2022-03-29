# ViterbiDecoderFor802.11
This is a part of our cuda sdr project, written in pure c++ without any other dependencies, which can be easily changed to cuda implementation. 
This decoder can not be used as a generic viterbi decoder. 
It is based on 802.11 standard. 
But hopefully, we will present some basic views of the viterbi algorithm in the following sections, which may give you an idea to implement your own decoder.

***Notes: since we only tested this code using a limited bunch of input, there might be unforeseen bugs in some cases.***


## 802.11 std Encoder
In 802.11, the convolutional encoder use industry-standard generator polynomials 133(octal) and 171(octal). In this case, polynomials are MSB first. In our actual implementation, we use a LSB first view, which does not change anything but can provide some convinient properties.  ![Alt pic](https://github.com/milinzhang/ViterbiDecoderFor802.11/blob/main/fig/ConvEnc.png)

## Butterflies
In the trilles graph, branches between current states and previous states have a butterfly-like pattern. This can let us easily represent multiple states using one butterfly. In the LSB view, when the bit at rightmost is 0, every input will either double the number (when input is 0) or double the number and plus 1 (when input is 1). Similarly, when the bit at rightmost is 1, every input will shift registers and cause same result as the state who has same other bits but the rightmost is 0 (which is 32 less than the previous one) does. ![Alt pic]()
