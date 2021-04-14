# mmotd&nbsp;[![MIT License](https://img.shields.io/badge/license-mit-blue.svg?style=flat-square&logo=data:image/svg+xml;base64,PD94bWwgdmVyc2lvbj0iMS4wIj8+PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIxOTYiIGhlaWdodD0iMTk2Ij48Y2lyY2xlIGN4PSI5OCIgY3k9Ijk4IiByPSI5OCIgZmlsbD0iIzgwODA4MCIvPjxjaXJjbGUgY3g9Ijk4IiBjeT0iOTgiIHI9Ijc4IiBmaWxsPSIjZmZmIi8+PGNpcmNsZSBjeD0iOTgiIGN5PSI5OCIgcj0iNTUiIGZpbGw9IiM4MDgwODAiLz48Y2lyY2xlIGN4PSI5OCIgY3k9Ijk4IiByPSIzMCIgZmlsbD0iI2ZmZiIvPjxyZWN0IHdpZHRoPSIzMSIgaGVpZ2h0PSIyNSIgeD0iMTIzIiB5PSI4NSIgZmlsbD0iI2ZmZiIvPjxyZWN0IHdpZHRoPSIyMC41IiBoZWlnaHQ9IjEwMCIgdHJhbnNmb3JtPSJtYXRyaXgoMSwxLC0xLDEsMTM3Ljc1LDM3Ljc1KSIgZmlsbD0iI2ZmZiIvPjxyZWN0IHdpZHRoPSIxNy41IiBoZWlnaHQ9IjEyMCIgdHJhbnNmb3JtPSJtYXRyaXgoMSwxLC0xLDEsMTQ5LjI1LDI5LjI1KSIgZmlsbD0iIzgwODA4MCIvPjwvc3ZnPg==)](https://opensource.org/licenses/MIT)

## Modified Message of the Day

[![build results](https://img.shields.io/github/workflow/status/jasonivey/mmotd/Build%20Mmotd?label=Build%20%26%20Test&logo=github-actions&logoColor=white&labelColor=grey&style=flat-square)](https://github.com/jasonivey/mmotd/actions/workflows/push.yml) ![current tag](https://img.shields.io/github/v/tag/jasonivey/mmotd?color=pink&include_prereleases&label=current%20tag&logo=git&sort=semver&style=flat-square) ![current release](https://img.shields.io/github/v/release/jasonivey/mmotd?color=pink&include_prereleases&label=current%20release&logo=git&sort=semver&style=flat-square)
![supported platforms](https://img.shields.io/badge/platforms%20supported-grey?style=flat-square)![ubuntu](https://img.shields.io/badge/Ubuntu-E95420?style=flat-square&logo=ubuntu&logoColor=white&labelColor=D74516&color=310926)![macOS](https://img.shields.io/badge/Apple-000000?style=flat-square&logo=apple&logoColor=5AB73B&labelColor=EDEDED&color=4F9BCD) ![build system](https://img.shields.io/badge/build%20system-CMake-blue?style=flat-square&logo=cmake&logoColor=green)
[![commit activity](https://img.shields.io/badge/yes-brightgreen?style=flat-square&label=maintained%3F)](https://github.com/jasonivey/mmotd/graphs/commit-activity) [![maintainer](https://img.shields.io/badge/jasonivey-blue?style=flat-square&label=maintainer)](https://github.com/jasonivey) [![commits](https://img.shields.io/github/commit-activity/m/badges/shields?logo=github&style=flat-square)](https://github.com/jasonivey/mmotd/commits/master)
![code size](https://img.shields.io/github/languages/code-size/jasonivey/mmotd?color=red&logo=c%2B%2B&logoColor=red&style=flat-square)

![demo](https://github.com/jasonivey/mmotd/raw/master/.assets/demo.gif)

### Introduction

The `/etc/motd` is a file on `Unix`-like systems that contains a "**message of the day**", used to send a common message to all users in a more efficient manner than sending them all an e-mail message. Other systems might also have an motd feature, such as the motd info segment on `MULTICS`.

The idea that a set of information could be printed everytime I open a new terminal enticed me. But what was even more enticing is the customization of that message.
1. I wanted the ability to turn on/off features of the `motd`.
1. I also wanted the ability to either pre-process and/or post-process the information gathered by any or all of the `motd` segments so I could colorize them, strip them of certain information, etc.
1. Most importantly, I wanted the ability to describe the information which the client would want to print by specifying both a `configuration` and a `template` file.

I did a majority of this work already inside a `Python` script.  Unfortunately, it wasn't until I finished that I noticed that my performance was noticeable.  [**No, I mean really noticeable!  Each new open terminal would take quite a few seconds to start. This was in contrast to terminals which would instantaneously open.**]

I ran profilers on the code and found a number of bottle necks which I could and should eliminate.  I thought about caching values on disk and only refreshing that data periodically by running a  `cron` (or some other scheduler) task periodically.

After deliberating my options for a few seconds I hastily decided that it would be a good exercise to start from scratch.

Hence, `modified message of the day` or `mmotd`
