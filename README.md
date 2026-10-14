# Secrets of Ravenwood

A room-by-room haunted mansion game. C++ owns the question sets, randomized selection, answer checks, score, lives, powers, room progression, and final reward. The browser uses HTML forms and CSS; there is no JavaScript game logic.

## Project files

- `index.html` — bungalow start screen.
- `style.css` — room scenes, responsive layout, effects, and completion panel.
- `src/main.cpp` — HTTP server, OOP challenge types, game state, randomized questions, and backend validation.
- `assets/hallway-background.png` — the supplied full-screen Hall background image.
- `assets/bedroom-background.png` — the supplied full-screen Bedroom background image.
- `assets/kitchen-background.png` — the supplied full-screen Kitchen background image.
- `assets/library-background.png` — the supplied full-screen Library background image.
- `assets/victory-background.png` — the supplied escape-screen background image.
- `assets/start-background.png` — the supplied Get Started screen background image.
- `Makefile` — build and run targets.

## Build and run on macOS or Linux

From this folder:

```sh
make
./build/ravenwood
```

The Makefile automatically selects `/Library/Developer/CommandLineTools/SDKs/MacOSX26.5.sdk` when it is present, avoiding the incompatible MacOSX27 SDK on the affected toolchain. Open `http://localhost:8080` and keep the terminal running. To use a different port, run `./build/ravenwood 9090`.

## Windows with VS Code

The HTTP server uses POSIX sockets. The simplest Windows setup is VS Code with the **WSL** extension and an Ubuntu WSL distribution:

1. Open this project folder in VS Code through WSL.
2. In the WSL terminal, install a compiler if needed: `sudo apt update && sudo apt install build-essential`.
3. Run `make && ./build/ravenwood`.
4. Open `http://localhost:8080` in a Windows browser.

Windows forwards WSL localhost ports. A native Windows build would need a Winsock implementation of the server socket layer.

## Game flow

Get Started → three Hall riddles → Continue → Bedroom scrambled-word puzzles (forest, candle, and mirror, shuffled each game) → Continue → Kitchen dish puzzle → Continue → three Library questions → Continue → collect the key. Correct answers score 100 points; incorrect answers cost one of five lives. The escape key grants 500 points and one life (up to five). Each puzzle room refreshes two Magic Potion uses and two Ghost Whisper uses on entry. Three Answer Keys are available for the whole game; each reveals the current answer without submitting it. Potions reveal a clue for the current question; whispers reveal its first letter, then its second, without submitting an answer. Kitchen dishes are randomly selected from pizza, burger, noodles, cake, ice cream, French fries, sandwich, and donut. Play Again resets game state and chooses a new set of questions.

Question sets are randomized from each room's C++ pool. Answer checks, powers, stage completion, and transitions are validated by the backend; posting a locked action cannot skip a room. State persists through browser refreshes while the server stays running.

## C++ OOP concepts

- `Challenge` is an abstract base class with virtual methods; `RiddleChallenge` and `MealChallenge` override them (runtime polymorphism).
- Challenge/question objects encapsulate room data; `Game` stores each player's state.
- Constructors initialize the challenge sets; virtual destruction is provided by `Challenge`.
- Vectors, maps, and an unordered map organize pools, form data, and per-browser sessions.

## Publish online

GitHub stores source files but does not run this C++ server. Public hosting requires a host that can compile and run a persistent C++ web process, plus a public port or reverse proxy and suitable session persistence. This starter server keeps sessions in memory, so restarting it resets active games.
