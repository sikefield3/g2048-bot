g2048-bot
=========

just some bot playing 2048

2014-07-18 16:36:43
This is an attempt at a robot playing the game "2048" by  gabrielecirulli (https://github.com/gabrielecirulli/2048).
On startup, a random start configuration is created (depending on the random seed value).
The bot than starts playing like in the original game by evulating all possible moves using the number of empty tiles 
as evaluation function. The program was tested with a lookahead value up to 3, after that the running time was getting too long.

I have done only superficial debugging and testing and so far the bot hasn't won any games.
For improvement, one might consider another evaluation function or pruning or some other tricks.
