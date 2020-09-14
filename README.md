# mmotd
## Modified Message of the Day

### Introduction

The `/etc/motd` is a file on `Unix`-like systems that contains a "**message of the day**", used to send a common message to all users in a more efficient manner than sending them all an e-mail message. Other systems might also have an motd feature, such as the motd info segment on MULTICS.

The idea that a set of information could be printed everytime I open a new terminal enticed me. But what was even more enticing is the customization of that message.
1. I wanted the ability to turn on/off features of the `motd`.
1. I also wanted the ability to either pre-process and/or post-process the information gathered by any or all of the `motd` segments so I could colorize them, strip them of certain information, etc.
1. Most importantly, I wanted the ability to describe the information which the client would want to print by specifying both a `configuration` and a `template` file.

I did a majority of this work already inside a Python script.  Unfortunately, it wasn't until all of the information gathering code was finished that I noticed that my performance was noticable.  [**No, I mean really noticable.  Each new open terminal would take numerous seconds to start instead of the instaneous start-ups we are all used to.**]

I ran profilers on the code and found a number of bottle necks which I could eliminate by caching values on disk and only refreshing that data periodically by `cron` (or some other scheduler) periodically.

After deliberating my options for only a few seconds I hastily decided that I should use all of the powers of `C++17` and do this project the way it deserves to be done.

Hence, `modified message of the day` or `mmotd`

### Build

`mmotd` is using `CMake` for building and testing. Following these few steps will get you up and running. Since this project is still in its infancy I'm going to forego all the detailed environment setup steps (i.e. Install the latest XCode along with Command Line Tools, Install brew, run `brew install cmake, llvm, git`, etc.  See, I said I wasn't going to do it and I already did it.)

1. Clone the repository
	1. Choose a location for the `mmotd` source repository. It is personal preference but some will choose `$HOME/projects`, others choose `$HOME/dev` and many more choose `$HOME/code`. The point is that you find a place that works for you and you keep your repo's all within that directory. So the first step is to find an adequate location for all of your dev work. Once that location is found change the directory into it.
	1. `git clone https://github.com/jasonivey/mmotd.git`
	1. Change directory into the repo, `cd mmotd`
1. This step is actually a combination of a few commands glued together
	`rm -rf build && mkdir build && cd build`
	1. To make sense of this, first understand that the build directory is not part of repository.
	1. The build directory is just for our build artifacts and left over scrapts (object files, and libraries.)
1. Building the source
	1. `cmake ..`
	1. `make`
1. Assuming all of the above commands succeeded then the mmotd binary should be ready to execute from the root of the build directory.

### Testing

This needs to be fleshed out more.  I need not only `unit` testing but I also need some type of **real world** platform testing where the code can exercise on real/virtualized hardware.

