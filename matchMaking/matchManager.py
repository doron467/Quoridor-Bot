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


def play_game(bot1, bot2, referee):

    board = "76 4 0 0 10 10 1"

    while True:

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
            return bot2 if bot == bot1 else bot1

        if result.startswith("WIN"):
            # game finished
            winner = int(result.split()[1])
            return bot1 if winner == 1 else bot2
        
        # "OK ..." becomes the new board
        board = result[3:].strip()


bot_v1 = start_bot("Bot")
bot_v1_2 = start_bot("Bot")
referee = start_bot("Referee")

winner = play_game(bot_v1,bot_v1_2,referee)
if winner == bot_v1:
    print("winner: bot1")
else:
    print("winner: bot2")