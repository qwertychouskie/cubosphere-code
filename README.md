# Cubosphere [reborn] :basketball: :video_game:
[![CircleCI (all branches)](https://img.shields.io/circleci/project/github/cubosphere/cubosphere-code/master.svg)](https://circleci.com/gh/cubosphere/cubosphere-code) [![#cubosphere on the freenode IRC network](https://img.shields.io/badge/FreeNode-%23cubosphere-brightgreen.svg)](https://webchat.freenode.net/?channels=cubosphere) [![linux downloads](https://img.shields.io/badge/downloads-linux-brightgreen.svg)](http://cubosphere.vallua.ru/artifacts)
[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2Fqwertychouskie%2Fcubosphere-code.svg?type=shield)](https://app.fossa.io/projects/git%2Bgithub.com%2Fqwertychouskie%2Fcubosphere-code?ref=badge_shield)
## A little story behind it
One upon a time on planet called Earth one man have found very very cool but anadobed in 2011-12 years game called "cubosphere". He enjoyed it and dreamt that some day it will become great.

And after a while he decided to pick it up and at least make look like a modern project, so others will be able to contribute easily.

This is why this organisation exists now.

# What is this game about?
Goal of this project is a freeware game similar to the PSX game "Kula World" / "Roll Away". It is designed platform independent, written in C++ and using the following libraries: OpenGL, SDL2 (Simple DirectMedia Layer), Lua, GLSL-Shader and POCO libraries set.

## Features
* Game similar to Kula World
* Over 450 levels in 34 different designs!
* Starting from beta 0.3 — a lot of new stuff like magnets, gravity changers and so on
* Multiball feature and two-player-mode
* Internal level editor!
* 3d-Engine via OpenGL
* GLSL-Shaders
* Joystick/Gamepad support

# What have I already done to improve the game?
* **Moved to SDL2 from SDL1.2**
* Migrated to CMake
* Separated data and code repos with auto ZIP
* Fixed few warnings
* Removed `using namespace std;`
* Made code C++17 compatible (no either warnings or errors)
* Configured Cricle CI with auto .deb build for Ubuntu LTS + last release (read next section), cross-linux .tar.gz
* Created icon and .desktop file
* Moved editor manual to GitHub wiki which can be opened from editor menu
* Added `.png` support and converted all textures to it, previews are saved in it too
* Replaced self-patched minizip with POCO Zip

… and a lot of technical or smaller internal changes!

# How to download last git build?
1. Go to http://cubosphere.vallua.ru/artifacts (warning: only last build artifacts are kept, some of them may be missing if build is uploading now)
2. Download apporative package for you (naming is Cubosphere-timestamp+git-hash-…)
3. Install as normal (if you don't know how, just open file and click "Install" button or someting like it)

## Ubuntu packages
You can be confused by ubuntu packaging system, but it isn't hard in fact.

1. You need both `BIN` and `DATA` packages, they are `cubosphere` and `cubosphere-data` respectively.
2. It is ok that their versions differ, data is stored in another repo and updates with it
3. When updating, download only `BIN` package: download `DATA` only if deps can't be resolved 
    1. You'll probably be reqested to remove `cubosphere` when updating `cubosphere-data`, do so and install new version later
    2. Use `dpkg -i` from cmd to avoid this issues (google a bit)

# What will/should be done?
Check out [GitHub™ projects](https://github.com/cubosphere/cubosphere-code/projects)!

# How can I take part in fun and make cubosphere great again?
You can:

* Fork repo and make pull request or
* If you have made a lot of contributions and I trust you then you can ask for direct write access
* Test game, report issues, tell friends about it
* If you have experience with CMake on Windows™ or MacOs, any help with it will be appreciated!
* Join IRC (check badge) and talk to me, other players and (if they are here!) team members
* Support development on [![Beerpay](https://beerpay.io/cubosphere/cubosphere-code/badge.svg?style=flat)](https://beerpay.io/cubosphere/cubosphere-code), motivate using [![Beerpay](https://beerpay.io/cubosphere/cubosphere-code/make-wish.svg?style=flat)](https://beerpay.io/cubosphere/cubosphere-code)

# Ok, some credits
You can find original homepage, codebase and developers at https://sourceforge.net/projects/cubosphere.


## License
[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2Fqwertychouskie%2Fcubosphere-code.svg?type=large)](https://app.fossa.io/projects/git%2Bgithub.com%2Fqwertychouskie%2Fcubosphere-code?ref=badge_large)