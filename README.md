The all project is in the Projet directory

Command to compile and clean the project:
make : compile the project
make clean : remove all temporary files execept the executable
maek clean all : call make clean but remove also the executable


Executable : blockchain
Syntax: blockchain [OPTIONS] -ip YOUR IP OR HOSTNAME [OPTIONS]

  -p		Port of the node, set to DEFAULT_PORT by default
  -ip2 		IP or hostname of another node in the network.
  -p2 		Port of the other node, set to DEFAULT_PORT by default.
  -a 		Active the API part of the node. Disabled by default.
  -m 		Active the mining part of the node. Disabled by default.
  -d 		The difficulty for the mining, set to the DEFAULT_DIFFICULTY by default.
		    This parameter is for tests only. Normaly this information is given by the network.
  -l		Active the loading of the binary file of the blockchain. path: 'bcsave.data'.
  -nbthr	The number of thread the mining thread can use, set to the number of cores - 1 by default.
