// Wires syntax 2

char* test = "\
.-----------------------.
| buffer                |-+                                                                                             \n\
| file:'human-voice.mp4'| |   .------------------.                                                                      \n\
'-----------------------' +-> |filter:bassFilter |---> [gain:bassGain] --------+                                        \n\
                          |   |   type:'lowpass' |         gain:2.0            |                                        \n\
                          |   | frequency:160    |                             |                                        \n\
                          |   |       q:1.2      |                             |                                        \n\
                          |   '------------------'                             |                                        \n\
                          |   .------------------.                             |                                        \n\
                          +-> |filter:midFilter  |---> [gain:midGain] ---------+                                        \n\
                          |   |  type:'bandpass' |          gain:4.0           |                                        \n\
                          |   | frequency:500    |                             |                                        \n\
                          |   |    q:1.2         |                             |                                        \n\
                          |   '------------------'                             |                                        \n\
                          |   .-------------------.                            |                                        \n\
                          +-> |filter:trebleFilter| ----> [gain:trebleGain] ---+---> gain -----+---> recorder           \n\
                              | type:'highpass'   |        gain:3.0                 gain:1.0   +---> monitor            \n\
                              | frequency:2000    |                                            +---> oscilloscope       \n\
                              |    q:1.2          |                                            +---> analyser           \n\
                              '-------------------'                                            +---> audiocontext         "
