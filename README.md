# echo-server-riot
This is a project to create a simple UDP echo server in [RIOT](http://www.riot-os.org/). Following requirements from RAPstore.

## Project Build

  * Use ```./generate.sh -p``` to prepare an enviroment, require sudo.
  * Use ```./generate.sh -g``` to clone [RIOT](http://www.riot-os.org/] repository.
  * Use ```./generate.sh -i``` to create 'tap' interface (requires sudo).
  * Use ```./generate.sh -b``` to build a project, binary will be on ./src/bin/native/echo_server.elf.
  * Use ```./generate.sh``` to show help options.

## Project Run

  * Use ```./generate.sh -r``` to run a RIOT OS.
    * Use ```server_start <port>``` to start a UDP echo server on specific <port>.
    * Use ```server_stop``` to stop a UDP echo server.
    * Use ```ifconfig``` to show network interfaces informations.
    * Use ```help``` to show acceptables commands.
    * Use ```CTRL^C``` to stop RIOT OS.

## Project Clean
  * Use ```./generate.sh -c``` to clean a build files.
  * Use ```./generate.sh -d``` to clean all temporary files and remove ethernet interface (requires sudo)
