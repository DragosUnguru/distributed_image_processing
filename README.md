# Brief intro

    A distributed program implemented using MPI
to manipulate PNM / PGM formatted images.

    This program is able to apply different filters
on these types of images. (smooth, blur, mean, emboss, sharpen)

# Implementation details

    The designated master process is responsible for some additional
tasks such as:
        -> Reads the input image and pads it accordingly
        -> Sends needing data and metadata from image to the rest
        -> Receives the computed data from peer processes
        -> Writes output image
    And for performance concerns, it also takes part in processing
it's chunk of the image.

    Every other thread receives it's chunk of the image to be processed
along with needed additional information (line width, image max value, type, etc...).
    They then proceed to apply the given filter on their chunk of the image making use
of neighbouring upper and/or bottom lines needed to compute the chunk-bordering data.
    After the filter is applied on their part of the image, they send the result back to
the master process in order to resend the updated info of bordering lines needed by every
other thread.
    This process is repeated for every filter demanded.

    The "apply_filter" function is where the magic is happening. This function takes
the thread's chunk of the image and borders it accordingly with the needed upper and/or
lower missing lines that were designated to other threads.
    The only other thing remaining from this point is to properly apply the filter using
the convolution between every other's pixel neighbours and the kernel proper to the filter.

    Through the whole program, the image data was preserved in a liniarized form.

# SCALABILITY

    The scalability of this implementation is measured using
the "time" utilitary from Linux and taking into account the "real" 
measurement. So we'll have a measurement in seconds.
    The result shown is obtained by computing a mean of the total
of 5 times ran for every number of processes from 1 up to 8 and
applying every filter for a total of 5 times on the "baby-yoda"
colored image.

    1 PROCESS:      5.31
    2 PROCESSES:    3.09
    3 PROCESSES:    2.27
    4 PROCESSES:    1.91
    5 PROCESSES:    2.11
    6 PROCESSES:    1.93
    7 PROCESSES:    1.89
    8 PROCESSES:    1.71

    For better observing the performance converge, these are the times
measured for applying all filters for 25 times each (for a total of 100 
applied filters) on the high-resolution "landscape" image:

    1 PROCESS:      102.30
    2 PROCESSES:    58.79
    3 PROCESSES:    45.09
    4 PROCESSES:    40.51
    5 PROCESSES:    42.68
    6 PROCESSES:    38.43
    7 PROCESSES:    34.55
    8 PROCESSES:    31.71

    The times shown above are the "best" times obtained from 3 to 4 runs each.

    Take these results with a pinch of salt as many factors come into play. The times
may converge to a lower value at some point due to cache-memory management. Also,
a disturbing factor is that at some unknown point during testing the processor might
have gone into boost mode, or worse, thermal throttling (as it is a ultra-low-powered
"U" series processor and it really started to burn and make noise :) ).

    These values are given running on a 4-core ultra-low-powered processor
with Hyperthreading. Talking about an Intel Core I7-8550U 1.8GHz.
