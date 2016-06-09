
Wires
======

Rules
-----

Signal flow is to the right and down.
Signals leave nodes only from the right.
Signals enter nodes only from the left.

Syntax
-------

// means a commented line
Spaces have no meaning except where they enforce vertical alignment of syntatical elements.
A string bracketed with brackets is a node.
If a bracketed node string contains a colon, the string to the left of the colon is the node type, the string to the right is the node name
Node names are made unique internally by appending a number even if they are not unique in the diagram.
Nodes whose names preceded by a * are not unique. This is for creating loops or back flows.
A simple string is a node.
A string with a colon is an attribute.
The string to the left of a colon is an attribute name.
If there are two colons in an attribute, the second colon brackets the type
If there is no type specified, the type is deduced.
The string to the right of a colon name is a value.
The default type is string.
If there is no string to the right of a colon, then the value is the default value for the type
If a value is quoted, it is a string.
If a value is numeric, it is a float.
If a value is followed by one or more comma value pairs, it is a float array of a certian number of components.
Types can be one of s, f, f2, f3, f4, f22, f23, f32, f33, f42, f43, f44, f24, f34
- means signal flows right
| means signal flows down
+ mean signal flows right and/or down
> means signal stops at the thing to the right.
) means signal flows down and if signal flows from the right it goes under the down signal (in other words a bridge)
An attribute belongs to the node its colon is under.
If an attribute's colon is under an attribute, the attribute belongs to the node the upper attribute belongs to, recursively.
An attribute under no node is a free standing attribute.
signal wires not connected at both ends are ignored

Samples
-----------

Here are some sample Wires diagrams, in the form of C encoded strings


    #define TEST1 "\
    trivial ---> trivial2 \n\
    foo ------->bar"

    #define TEST2 "\
    trivial ---+                  \n\
               |                  \n\
               )                  \n\
               +---->   trivial2"

    #define TEST3 "          \n\
    foo -------+             \n\
    trivial1 --)-->trivial2  \n\
               +->     bar   "

    #define TEST4 "\
    sample0 ---------+                 \n\
    sample1 -----+   |                 \n\
    sample2 -----)---)----->output2    \n\
                 +---)----->output1    \n\
                     |                 \n\
                     +--->output0     "

    #define TEST5 "\
    sample --+                                                                                           \n\
             +-> bassFilter ---> bassGain --------+                                                      \n\
             +-> midFilter ----> midGain ---------+                                                      \n\
             +-> trebleFilter ----> trebleGain ---+---> gain -+---> recorder ------+                     \n\
                                                              +---> monitor  ------+---> oscilloscope    \n\
                                                              +---> analyser                             \n\
                                                              +---> audiocontext                         "


    #define TEST6 "\
    [buffer:sample] ----------+                                                                                           \n\
      file:'human-voice.mp4'  |                                                                                             \n\
                              +-> [filter:bassFilter] ---> [gain:bassGain] --------+                                        \n\
                              |        type:'lowpass'          gain:2.0            |                                        \n\
                              |   frequency:160                                    |                                        \n\
                              |           q:1.2                                    |                                        \n\
                              +-> [filter:midFilter] ----> [gain:midGain] ---------+                                        \n\
                              |        type:'bandpass'          gain:4.0           |                                        \n\
                              |   frequency:500                                    |                                        \n\
                              |           q:1.2                                    |                                        \n\
                              +-> [filter:trebleFilter] ----> [gain:trebleGain] ---+---> gain -----+---> recorder           \n\
                                    type:'highpass'            gain:3.0                  gain:1.0  +---> monitor            \n\
                               frequency:2000                                                      +---> oscilloscope       \n\
                                       q:1.2                                                       +---> analyser           \n\
                                                                                                   +---> audiocontext         "

