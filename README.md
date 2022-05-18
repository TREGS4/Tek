# Tek by TREG

Tek is a cryptocurrency that uses a blockchain, based on a peer-to-peer network.
This is a 2nd year student project.

## About Tek

A permanent demo website is available [here](http://tek.ollopa.fr:7000/).

****BE CAREFUL****: The demo website's source code and the database passwords are accessible by everyone and the passwords are not secured, so do not choose real passwords.

You can **create an account** and you will **receive a generous donation** from us!
You can play with the website and send transactions requests to the blockchain.

For budget concerns, the blockchain network is only composed of a few nodes, but nothing prevents you from joining the network for fun. You can do this by following the [installation section](#Installation).

Of course, these transactions have no real value, and you cannot buy anything with it.

For the **website source code** go [here](https://github.com/TREGS4/Tek_webdemo).

## Installation

You can **download a binary** from the demo website [here](http://tek.ollopa.fr:7000/) (Linux Amd64).

**OR**

You can **manually compile the source code** by following the next steps:

In order to do so, you need to go to the `Projet/` directory:
```sh
cd Projet/ #From the root of the source code
```
Then, you need to compile the projet with `Makefile` by running:
```sh
make
```

>If make is not already installed, run the following command:
>```sh
>sudo apt install make
>```
>If gcc is not already installed, run the following command:
>```sh
>sudo apt install gcc
>```
>If OpenSSL is not already installed, run the following command.
>```sh
>sudo apt install libssl-dev
>```


The builded binary should be named **tek**.

## How to use it?

### Syntax 
```sh
./tek [OPTIONS] -ip YOUR_IP_OR_HOSTNAME [OPTIONS]
```
### Options
-  **-p**		→ Port of the node (set to DEFAULT_PORT by default).
-  **-ip2** 		→ IP or hostname of another node in the network.
-  **-p2** 		→ Port of the other node (set to DEFAULT_PORT by default).
-  **-a** 		→ Active the API part of the node (disabled by default).
-  **-pa**      → Port of the api server (set to DEFAULT_API_PORT by default).
-  **-m** 		→ Active the mining part of the node (disabled by default).
-  **-d** 		→ The difficulty for the mining (set to DEFAULT_DIFFICULTY by default).

The following options are for **testing purposes only**. Normally these pieces of information are given by the network.
-  **-l**		→ Activate the loading of the binary file of the blockchain. path: 'bcsave.data'.
-  **-nbthr**	→ The number of threads the mining thread can use (set to the number of cores - 1 by default).




## Contributors
|   Name          |        mail              |
|-----------------|--------------------------|
| Adrien Pingard  | adrien.pingard@epita.fr  |
| Thimot Veyre    | thimot.veyre@gmail.com   |
| Margaux Cavalie | margaux.cavalie@epita.fr |
| Pierre Litoux   | pierre.litoux@epita.fr   |
