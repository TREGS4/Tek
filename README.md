# Tek by TREG

Tek is a student project.

A blockchain based on a peer-to-peer network.


## Try it

A permanent demo website is available on [here](http://tek.ollopa.fr:7000/).

****BE CAREFUL****: **The demo web site source code and the database password is accessible by everybody. And the passwords are not secured, so put not real passwords**

You can **create an account** and you will **receive a generous donation** from us !
You can play with the website and send transactions requests to the blockchain.

For budget concerns, the blockchain networks is composed of few nodes, 
but nothing prevents you to join the network for fun. You can do this by following the [installation section](#Installation)

Of course all transactions inside the website has no real value and you cannot buy anything with it.

**Website source code**: [go to](https://github.com/TREGS4/Tek_webdemo)

## Installation

You can **download a binary** from the demo website. [here](http://tek.ollopa.fr:7000/). (Linux Amd64)

**Or**

You can **manually compile the source code** by following the next steps.

You need to go in the `Projet/` directory
```sh
cd Projet/ #From the root of the source code
```
Then, you need to compile the projet with `Makefile` by running
```sh
make
```

>You you have not Makefile installed, run this following command.
>```sh
>sudo apt install make
>```
>You you have not gcc installed, run this following command.
>```sh
>sudo apt install gcc
>```
>You you have not OpenSSL installed, run this following command.
>```sh
>sudo apt install libssl-dev
>```

Binary builded: **tek**

## How to use

### Syntax 
```sh
./tek [OPTIONS] -ip YOUR IP OR HOSTNAME [OPTIONS]
```
### Options
-  **-p**		Port of the node, set to DEFAULT_PORT by default
-  **-ip2** 		IP or hostname of another node in the network.
-  **-p2** 		Port of the other node, set to DEFAULT_PORT by default.
-  **-a** 		Active the API part of the node. Disabled by default.
-  **-pa**      Port of the api server. Set to DEFAULT_API_PORT by default.
-  **-m** 		Active the mining part of the node. Disabled by default.
-  **-d** 		The difficulty for the mining, set to the DEFAULT_DIFFICULTY by default.

This parameter is for **tests only**. Normaly this information is given by the network.
-  **-l**		Active the loading of the binary file of the blockchain. path: 'bcsave.data'.
-  **-nbthr**	The number of thread the mining thread can use, set to the number of cores - 1 by default.




## Contributors
|   Name          |        mail              |
|-----------------|--------------------------|
| Adrien Pingard  | adrien.pingard@epita.fr  |
| Thimot Veyre    | thimot.veyre@gmail.com   |
| Margaux Cavalie | margaux.cavalie@epita.fr |
| Pierre Litoux   | pierre.litoux@epita.fr   |
