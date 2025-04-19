## Discord Bot for SP6PWT

#### What it does?
This bot allows to use commands on Discord server to switch the antennas in the club without a need for VPN connection.

#### How it works?
Antenna switch controller provides a web interface and communicates using Websockets, so this bot essentially replaces the fronted of the switch controlles. On the other end it uses Discord library for Python to expose some control commands on our internal Discord server.

Script itself runs on our Linux server located in the club so it has direct access to the switch controller. It's started and monitored with systemd.

#### Can I use it?
Code is customized to the environment we have, especially the antenna switch controller WS commands, so it may turn out it's easier to use this code as a reference and create own solution from scratch.

#### Other info
##### Discord App required scopes
- applications.commands
- bot
##### Available commands
- /jakie-anteny
- /antena [1-4]
- /off
