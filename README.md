### SigilVM

    Application developement framework and a runtime.

    Sigil abstracts an application as a group of interacting modules. 
    Modules are kept in an environment vector, and managed by 0 indexed runtime module. 

    Sigil provides premade modules:
        - "runtime": must be loaded first, essential for SigilVM functionality
        - "station": network and connectivity manager
        - "hid": driver for low level human interface, like hardware buttons, or 4bit mode LCD, or window events
        - "workthread": takes in work item and attaches it to a thread
        - "workqueue": extended workthread, has additional field -> queue of work items
        - "ntt": entity component system implementation for general usage
        - "visor": Graphics engine and renderer for Sigil

### Usage:

    mkdir build && cd ./build
    cmake ../
    make -j

### TODO:

    Asset system still not complete, most of deinit process does not exist