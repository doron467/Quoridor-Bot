from pathlib import Path
import subprocess

HERE = Path(__file__).resolve().parent

def start_bot(path):
    return subprocess.Popen(
        [str(HERE / path)],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        text=True,
        bufsize=1
    )


def send_board(bot, board):
    bot.stdin.write(board + "\n")
    bot.stdin.flush()

    return bot.stdout.readline().strip()


def play_game(bot1, bot2, referee,board):

    #board = "76 4 0 0 10 10 1"
    moves = 0

    while True:

        moves += 1
        if (moves > 300):
            #TOO LONG
            return referee,0

        turn = int(board.split(" ")[6])
        bot = bot1 if turn == 1 else bot2

        bot.stdin.write(board + "\n")
        bot.stdin.flush()

        move = bot.stdout.readline().strip()
        print(f"Player {turn}: {move}")

        referee.stdin.write(board + "\n")
        referee.stdin.write(move + "\n")
        referee.stdin.flush()

        result = referee.stdout.readline().strip()

        if result == "ILLEGAL":
            # current bot loses
            return (bot2 if bot == bot1 else bot1),0

        if result.startswith("WIN"):
            # game finished
            winner = int(result.split()[1])
            difference = abs(int(result.split()[3]))
            return (bot1 if winner == 1 else bot2),difference
        
        # "OK ..." becomes the new board
        board = result[3:].strip()


bot_v1 = start_bot("Bot_v1")
bot_v2 = start_bot("Bot_v2")
referee = start_bot("Referee")

boards = ["49 31 0 0 10 10 1","49 31 8830452760576 0 9 9 1","49 31 8796093546496 17179869184 8 9 2",
          "59 22 2260595906707456 0 9 9 2","57 23 0 137506062336 9 9 1"]

v1Wins,v2Wins = 0,0
bfsDifference = 0
terminations = 0
for board in boards:
    winner1,diff1 = play_game(bot_v1,bot_v2,referee,board)
    winner2,diff2 = play_game(bot_v2,bot_v1,referee,board)
    if (winner1 == bot_v1):
        v1Wins += 1
        bfsDifference += diff1
    elif winner1 == bot_v2:
        v2Wins += 1
        bfsDifference -= diff1
    else:
        terminations += 1

    if (winner2 == bot_v1):
        v1Wins += 1
        bfsDifference += diff1
    elif winner2 == bot_v2:
        v2Wins += 1
        bfsDifference -= diff1
    else:
        terminations += 1


print("v1 wins: " + str(v1Wins))
print("v2 wins: " + str(v2Wins))
print("bfs difference: " + str(bfsDifference))
print("terminations: " + str(terminations))
